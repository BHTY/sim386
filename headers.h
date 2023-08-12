#pragma once

#include <stdint.h>

#if defined (WIN32)
#include <Windows.h>
#include <winnt.h>
#else

#error "Only works on Windows"

#endif

#include "../sim386.h"

typedef struct TAKEN_BASE{
	uint32_t base;
	struct TAKEN_BASE* next;
} TAKEN_BASE;

typedef struct IMPORTED_FUNCTION{
	uint32_t address;
	int ordinal_or_name; //set to 0 for ordinal, 1 for name
	union{
		char* name;
		uint16_t ordinal;
	};
	struct IMPORTED_FUNCTION* next;
} IMPORTED_FUNCTION;

typedef struct IMPORT_TABLE{
	BYTE* image_name;
	IMPORTED_FUNCTION* fn_table;
	struct IMPORT_TABLE* next;
} IMPORT_TABLE;

typedef struct EXPORTED_FUNCTION{
	uint32_t address;
	uint32_t name;
	uint16_t ordinal;
	struct EXPORTED_FUNCTION* next;
} EXPORTED_FUNCTION;

typedef struct{
	BYTE* data;
	uint32_t image_base;
	uint32_t entry_point;
	IMAGE_NT_HEADERS* nt_headers;
	IMAGE_SECTION_HEADER* section_headers;
	IMPORT_TABLE* import_table;
	EXPORTED_FUNCTION *export_table;
	uint32_t resource_offset;

	uint32_t stack_commit;
	uint32_t stack_reserve;
	uint32_t heap_commit;
	uint32_t heap_reserve;
} LOADED_PE_IMAGE;

typedef struct LOADED_IMAGE{
	LOADED_PE_IMAGE image;
	char* name;
	struct LOADED_IMAGE* next;
} LOADED_IMAGE;

//this is getting deprecated!
typedef struct WINDOW_CLASS{
	uint32_t WndProc;
	uint32_t TimerProc;
	char class_name[100];
	struct WINDOW_CLASS* next;
} WINDOW_CLASS;

typedef struct EMU_HINSTANCE{
	uint32_t image_base; //contains the actual HINSTANCE (GetModuleHandle(NULL) always returns the HINSTANCE of the root - DLL entry points are passed their HINSTANCE)
	uint32_t root_rsdir;
	struct EMU_HINSTANCE* next;
} EMU_HINSTANCE;

typedef struct REGISTERED_WINDOW_CLASS { //this one is stored in a flat array, not a linked list
	int used;
	EMU_HINSTANCE* hInstance;
	HMENU hMenu;
	uint32_t wndproc;
	LPWSTR class_name;
	int global;
} REGISTERED_WINDOW_CLASS;

typedef struct ACTIVE_WINDOW {
	uint32_t hwnd;
	uint16_t wndclass;
	struct ACTIVE_WINDOW* next;
	struct WINDOW_TIMER* timers;
} ACTIVE_WINDOW;

typedef struct WINDOW_TIMER {
	uint16_t timerID;
	uint32_t TimerProc;
	struct WINDOW_TIMER* next;
} WINDOW_TIMER;

uint32_t find_resource(i386* cpu, uint32_t pRootDirectory, LPWSTR resourceType, LPWSTR resourceName, int type_id, int name_id);
uint32_t find_string(i386* cpu, uint32_t string_base, uint32_t string_id);

/* 
The (mostly linked) lists I'll need
- Currently loaded images
	Each entry will contain the HINSTANCE (image base) and a pointer to the root resource directory
- Registered window classes (most of the variables aren't important but a few are)
    Each entry will contain a pointer to an EMU_HINSTANCE structure, menu handle, window procedure, class name, and a flag indicating whether it's global or not
- Active windows
	Each entry will contain the actual Win32 HWND of the window, pointer to a REGISTERED_WINDOW_CLASS struct, and a pointer to a linked list of TIMER structs
- Timers
	Each entry contains a timer ID and a pointer to a TimerProc

On process startup, the root of the WINDOW list will have an HWND of 0 (i.e. the desktop window) for purpose of SetTimer but more windows can be added

I can usually assume that an application will call RegisterClass(Ex)A + CreateWindow(Ex)A or RegisterClass(Ex)W + CreateWindow(Ex)W so I should not
need to deal with Unicode<->ANSI conversion for the most part.

Order of business
1.) Fix the way that HINSTANCEs work
2.) Fix the way that windows work (including timers and all that nonsense)
3.) Implement resources -> get Reversi working @ 100%
	Reversi requires LoadString, LoadCursor, LoadIcon, LoadAccelerators, and has a menu in the window class
4.) Implement import directory parsing -> get FreeCell working
5.) Rearchitect the directory structure of both this PC and the repo to both make sense and mirror each other

FindResourceA/W will be implemented using a thunk, but the LoadString/LoadBitmap/LoadXXX functions will be implemented in C code inside of KERNEL32.dll

WINAPI HBITMAP LoadBitmapA(HINSTANCE hInstance, LPCSTR lpBitmapName){
	HRSRC hRsrc = FindResourceA(hInstance, lpBitmapName, RT_BITMAP);

	if(hRsrc == 0) return (HBITMAP)0;

	rsrc = LoadResource(hInstance, hRsrc);
	pBmp = LockResource(rsrc);
	return CreateBitmap(pBmp->bmWidth, pBmp->bmHeight, pBmp->bmPlanes, pBmp->bmBitsPixel, pBmp->bmBits);
}

Looking at this function, the only two functions that are actually thunked are FindResourceA and CreateBitmap
FindResourceA in this implementation returns a pointer to the resource data entry corresponding to the requested resource
	LoadResource just returns ((IMAGE_RESOURCE_DATA_ENTRY*)hRsrc)->OffsetToData    (i.e. the pointer to the actual bits)
	LockResource is essentially a no-op
CreateBitmap then takes the data passed and creates a HBITMAP in the context of the host and passes it back
	The only "conversion" CreateBitmap has to do is converting lpBits to a host-memory pointer

There's a find_resource() function on the host-side, called by the FindResourceA thunk. find_resource() takes the following parameters
- Pointer to the root resource data directory
	This is just the sum of the EMU_HINSTANCE fields image_base and root_rsdir
- The resource type name or ID
- The resource name or ID
- A flag indicating whether the search is by name or by ID
- A flag indicating whether the resource type is by name or by ID

The names are expected to be (if they are names at all) Unicode strings in the host's memory space
	It is up to the caller to ensure that this is the case
find_resource() returns a pointer (in the virtual memory space of the simulated CPU) to the directory entry that points to the resource



Creating a window
Step 1.) Call RegisterClass
	Most of the elements of the WNDCLASS structure are left unmodified but a few are "tweaked" before the thunker calls RegisterClass on the host
		The HINSTANCE is substituted for GetModuleHandle(NULL), the window procedure is replaced with dummy_WndProc, the menu name / ID is zeroed out
	The class name is converted into the host's memory space before RegisterClass is called by the thunker
	A few variables are cached in the window class table
		A pointer to the EMU_HINSTANCE for the instance handle, a copy of the class name, the global flag, and window procedure pointer
	If the menu name is non-NULL, a menu will be created on the host using CreateMenu(), reading the menu resource from the desired HINSTANCE
		The HMENU generated by this is stored into the window class table entry
	If CS_GLOBALCLASS is set, any further attempts to create a class with that name will fail
	The return value of RegisterClass is both returned to the program as an atom and used as the index into the window class table to store the entry
Step 2.) Call CreateWindow
	Most of the arguments passed to CreateWindow are left unmodified, but a few parameters are tweaked
		lpClassName - If non-NULL and an atom, it's an index into the window class table. If it's a string, it's a class name that, combined with the hInstance,
			refers to a window class. This will be used to find the REGISTERED_WINDOW_CLASS pointer which is stored in the entry for this window into the active
			window table. Additionally, the atom stored in the REGISTERED_WINDOW_CLASS is passed to lpClassName. It's still passed to CreateWindow as is.
		lpWindowName - Converted into the host's memory space if non-NULL
		hMenu - If NULL, it is substituted for the menu handle stored in the REGISTERED_WINDOW_CLASS
		hInstance - Substituted for the HINSTANCE of the host, but used to find the REGISTERED_WINDOW_CLASS
		lpParam - NULL (not handled for now)
	The linked list of TIMER structs is initialized to zero and the Win32 HWND returned by CreateWindow is stored in the active window table


How does dummy_WndProc know what to call?
Find the ACTIVE_WINDOW corresponding to the hwnd, then go into the window class table using ACTIVE_WINDOW.wndclass and call the function.

SetTimer()
None of the arguments are mutated except for SetTimer(). If it's zero, it's passed as such, but otherwise, it's replaced with dummy_TimerProc. The 
actual TimerProc passed is then passed as a parameter to register_window_timer alongside the ACTIVE_WINDOW and timer ID.
When dummy_TimerProc is called, it first summons the ACTIVE_WINDOW corresponding to the hwnd. It then reverse thunks into the TimerProc whose ID
corresponds to wParam.
*/
//4092c8