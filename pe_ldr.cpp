// pe_ldr.cpp : Defines the entry point for the console application.
//

/*
	Program Procedure
	The PE loader module loads the relevant PE image and calls the CPU functions to map the relevant data into its address space


*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

#include "headers.h"
#include "../sim386.h"

LOADED_IMAGE* loaded_images;
WINDOW_CLASS* window_classes;

uint32_t escape_addr;
i386* global_cpu;

uint32_t lookup_classname(char* class_name){ //returns a pointer to the WndProc
	WINDOW_CLASS* temp = window_classes;

	while (temp){
		if (strcmp(class_name, temp->class_name) == 0){ //match found
			return temp->WndProc;
		}
		temp = temp->next;
	}
	return 0;
}

void register_window_class(char* class_name, uint32_t WndProc){
	WINDOW_CLASS* temp;

	if (window_classes == 0){
		window_classes = (WINDOW_CLASS*)malloc(sizeof(WINDOW_CLASS));
		window_classes->next = 0;
		window_classes->WndProc = WndProc;
		strcpy(window_classes->class_name, class_name);
	}
	else{
		temp = window_classes;
		window_classes = (WINDOW_CLASS*)malloc(sizeof(WINDOW_CLASS));
		window_classes->next = temp;
		window_classes->WndProc = WndProc;
		strcpy(window_classes->class_name, class_name);
	}
}

LOADED_PE_IMAGE* find_image(char* name){
	LOADED_IMAGE* cur = loaded_images;

	while (cur){
		if (strcmp(name, cur->name) == 0){
			return &(cur->image);
		}

		cur = cur->next;
	}

	return 0;
}

LRESULT CALLBACK dummy_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	//return DefWindowProcA(hWnd, msg, wParam, lParam);

	HRESULT return_value;
	uint32_t wndproc_addr;
	LPSTR name_buffer = (LPSTR)malloc(100);
	GetClassNameA(hWnd, name_buffer, 100);
	wndproc_addr = lookup_classname((char*)name_buffer);
	free(name_buffer);

	printf("\nThunking WndProc(%p, %p, %p, %p)", hWnd, msg, wParam, lParam);

	if (wndproc_addr){
		//push the arguments onto the stack
		cpu_push32(global_cpu, &(global_cpu->eip));
		cpu_push32(global_cpu, (uint32_t*)&lParam);
		cpu_push32(global_cpu, (uint32_t*)&wParam);
		cpu_push32(global_cpu, (uint32_t*)&msg);
		cpu_push32(global_cpu, (uint32_t*)&hWnd);

		//call the function
		return_value = (LRESULT)cpu_reversethunk(global_cpu, wndproc_addr, escape_addr); //it should return into the unthunker

		//pop the arguments off of the stack
		//global_cpu->esp += 16;

		printf(" Finished WndProc, EIP=%p!", global_cpu->eip);

		return return_value;
	}
	else{
		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}
}

uint32_t thunk_MessageBoxA(i386* cpu){
	HWND hwnd;
	uint32_t lpText;
	uint32_t lpCaption;
	uint32_t uType;

	hwnd = (HWND)*(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);//NULL;
	lpText = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);//0x401020;
	lpCaption = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);//0x401020;
	uType = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);//0;

	//printf("\nCalling MessageBoxA(%08x, %p, %p, %08x)", hwnd, lpText, lpCaption, uType);
	printf("\nCalling MessageBoxA(%08x, \"%s\", \"%s\", %08x)", hwnd, virtual_to_physical_addr(cpu, lpText), virtual_to_physical_addr(cpu, lpCaption), uType);

	return MessageBoxA(hwnd, (LPCSTR)virtual_to_physical_addr(cpu, lpText), (LPCSTR)virtual_to_physical_addr(cpu, lpCaption), uType);
}

uint32_t thunk_ExitProcess(i386* cpu){
	uint32_t uExitCode = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);

	printf("\nCalling ExitProcess(%08x)", uExitCode);

	return 0;
}

uint32_t thunk_SetBkMode(i386* cpu){
	HDC hdc;
	int mode;

	hdc = (HDC)*(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	mode = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);

	printf("\nCalling SetBkMode(%p, %08x)", hdc, mode);

	return SetBkMode(hdc, mode);
}

uint32_t thunk_GetModuleHandleA(i386* cpu){
	uint32_t lpModuleName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	LPCSTR moduleName;

	printf("\nCalling GetModuleHandleA(%p)", lpModuleName);

	if (lpModuleName == 0){
		moduleName = 0;
	}
	else{
		moduleName = (LPCSTR)virtual_to_physical_addr(cpu, lpModuleName);
	}

	return (uint32_t)GetModuleHandleA(moduleName);
}

uint32_t thunk_LoadCursorA(i386* cpu){
	uint32_t hInstance = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpCursorName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPCSTR cursorName;

	printf("\nCalling LoadCursorA(%p, %p)", hInstance, lpCursorName);

	if (hInstance){ //handle ordinal
		cursorName = (LPCSTR)virtual_to_physical_addr(cpu, lpCursorName);
	}
	else{
		cursorName = (LPCSTR)lpCursorName;
	}

	return (uint32_t)LoadCursorA((HINSTANCE)hInstance, cursorName);
}

uint32_t thunk_LoadIconA(i386* cpu){
	uint32_t hInstance = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpCursorName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPCSTR cursorName;

	printf("\nCalling LoadIconA(%p, %p)", hInstance, lpCursorName);

	if (hInstance){ //handle ordinal
		cursorName = (LPCSTR)virtual_to_physical_addr(cpu, lpCursorName);
	}
	else{
		cursorName = (LPCSTR)lpCursorName;
	}

	return (uint32_t)LoadIconA((HINSTANCE)hInstance, cursorName);
}

uint32_t thunk_RegisterClassA(i386* cpu){
	uint32_t lpWndClass = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	WNDCLASSA* wc = (WNDCLASSA*)virtual_to_physical_addr(cpu, lpWndClass);

	if (wc->lpszClassName){
		wc->lpszClassName = (LPCSTR)virtual_to_physical_addr(cpu, (uint32_t)wc->lpszClassName);
		register_window_class((char*)wc->lpszClassName, (uint32_t)wc->lpfnWndProc);
	}

	if (wc->lpszMenuName){
		wc->lpszMenuName = (LPCSTR)virtual_to_physical_addr(cpu, (uint32_t)wc->lpszMenuName);
	}

	wc->lpfnWndProc = dummy_WndProc;

	printf("\nCalling RegisterClassA(%p)", lpWndClass);

	return RegisterClassA(wc);
}

uint32_t global;

uint32_t thunk_CreateWindowExA(i386* cpu){
	uint32_t dwExStyle = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpClassName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t lpWindowName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t dwStyle = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t X = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	uint32_t Y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 24);
	uint32_t nWidth = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 28);
	uint32_t nHeight = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 32);
	uint32_t hWndParent = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 36);
	uint32_t hMenu = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 40);
	uint32_t hInstance = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 44);
	uint32_t lpParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 48);

	printf("\nCalling CreateWindowExA(%p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p)", dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	LPCSTR className = 0;
	LPCSTR windowName = 0;
	LPVOID param = (LPVOID)lpParam;

	if (lpWindowName){
		windowName = (LPCSTR)virtual_to_physical_addr(cpu, lpWindowName);
	}

	if (lpClassName){
		className = (LPCSTR)virtual_to_physical_addr(cpu, lpClassName);
	}

	uint32_t retval = (uint32_t)CreateWindowExA(dwExStyle, className, windowName, dwStyle, X, Y, nWidth, nHeight, (HWND)hWndParent, (HMENU)hMenu, (HINSTANCE)hInstance, param);
	global = retval;
	//printf("Window Handle: %d\n", retval);

	return retval;
}

uint32_t thunk_ShowWindow(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t nCmdShow = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);

	printf("\nCalling ShowWindow(%p, %p)", hWnd, nCmdShow);

	return ShowWindow((HWND)hWnd, nCmdShow);
}

uint32_t thunk_UpdateWindow(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);

	printf("\nCalling UpdateWindow(%p)", hWnd);
	return UpdateWindow((HWND)hWnd);
}

uint32_t thunk_DefWindowProcA(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t Msg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t wParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);

	printf("\nCalling DefWindowProcA(%p, %p, %p, %p)", hWnd, Msg, wParam, lParam);

	return (uint32_t)DefWindowProcA((HWND)hWnd, Msg, wParam, lParam);
}

uint32_t thunk_PeekMessageA(i386* cpu){
	uint32_t lpMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t wMsgFilterMin = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t wMsgFilterMax = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	uint32_t wRemoveMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 24);

	printf("\nCalling PeekMessageA(%p, %p, %p, %p, %p)", lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);

	LPMSG pMsg = (LPMSG)virtual_to_physical_addr(cpu, lpMsg);

	return PeekMessageA(pMsg, (HWND)hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
}

uint32_t thunk_TranslateMessage(i386* cpu){
	uint32_t lpMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	CONST MSG * _lpMsg = (CONST MSG *)virtual_to_physical_addr(cpu, lpMsg);
	printf("\nCalling TranslateMessage(%p)", lpMsg);
	return (uint32_t)TranslateMessage((CONST MSG *)_lpMsg);
}

uint32_t thunk_DispatchMessageA(i386* cpu){
	uint32_t lpMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	CONST MSG * _lpMsg = (CONST MSG *)virtual_to_physical_addr(cpu, lpMsg);
	printf("\nCalling DispatchMessageA(%p)", lpMsg);
	return (uint32_t)DispatchMessageA((CONST MSG *)_lpMsg);
}

uint32_t thunk_GetCommandLineA(i386* cpu){
	printf("\nCalling GetCommandLineA()");

	return 0;
}

uint32_t thunk_GetStartupInfoA(i386* cpu){
	uint32_t lpStartupInfo = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	LPSTARTUPINFOA startupInfo = (LPSTARTUPINFOA)virtual_to_physical_addr(cpu, lpStartupInfo);
	printf("\nCalling GetStartupInfoA(%p)", lpStartupInfo);

	GetStartupInfoA(startupInfo);

	return 0;
}

uint32_t thunk_RegisterClassExA(i386* cpu){
	uint32_t lpWndClass = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	WNDCLASSEXA* wc = (WNDCLASSEXA*)virtual_to_physical_addr(cpu, lpWndClass);

	if (wc->lpszClassName){
		wc->lpszClassName = (LPCSTR)virtual_to_physical_addr(cpu, (uint32_t)wc->lpszClassName);
		register_window_class((char*)wc->lpszClassName, (uint32_t)wc->lpfnWndProc);
	}

	if (wc->lpszMenuName){
		wc->lpszMenuName = (LPCSTR)virtual_to_physical_addr(cpu, (uint32_t)wc->lpszMenuName);
	}

	wc->lpfnWndProc = dummy_WndProc;

	printf("\nCalling RegisterClassExA(%p)", lpWndClass);

	return RegisterClassExA(wc);
}

uint32_t thunk_GetMessageA(i386* cpu){
	uint32_t lpMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t hWnd = global;//*(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t wMsgFilterMin = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t wMsgFilterMax = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);

	printf("\nCalling GetMessageA(%p, %p, %p, %p)", lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);

	LPMSG pMsg = (LPMSG)virtual_to_physical_addr(cpu, lpMsg);

	return GetMessageA(pMsg, (HWND)hWnd, wMsgFilterMin, wMsgFilterMax);
}

uint32_t thunk_GetStockObject(i386* cpu){
	uint32_t i = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetStockObject(%p)", i);
	return (uint32_t)GetStockObject((int)i);
}

uint32_t thunk_PostQuitMessage(i386* cpu){
	uint32_t nExitCode = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling PostQuitMessage(%p)", nExitCode);
	//PostQuitMessage((int)nExitCode);
	return 0;
}

uint32_t thunk_EndPaint(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpPaint = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	CONST PAINTSTRUCT * _lpPaint = (CONST PAINTSTRUCT *)virtual_to_physical_addr(cpu, lpPaint);
	printf("\nCalling EndPaint(%p, %p)", hWnd, lpPaint);
	return (uint32_t)EndPaint((HWND)hWnd, (CONST PAINTSTRUCT *)_lpPaint);
}

uint32_t thunk_DrawTextA(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpchText = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t cchText = (int32_t)(int8_t)*(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lprc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t format = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	LPCSTR _lpchText = (LPCSTR)virtual_to_physical_addr(cpu, lpchText);
	LPRECT _lprc = (LPRECT)virtual_to_physical_addr(cpu, lprc);
	printf("\nCalling DrawTextA(%p, %p, %p, %p, %p)", hdc, lpchText, cchText, lprc, format);
	return (uint32_t)DrawTextA((HDC)hdc, (LPCSTR)_lpchText, (int)cchText, (LPRECT)_lprc, (UINT)format);
}

uint32_t thunk_GetClientRect(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpRect = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPRECT _lpRect = (LPRECT)virtual_to_physical_addr(cpu, lpRect);
	printf("\nCalling GetClientRect(%p, %p)", hWnd, lpRect);
	return (uint32_t)GetClientRect((HWND)hWnd, (LPRECT)_lpRect);
}

uint32_t thunk_BeginPaint(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpPaint = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPPAINTSTRUCT _lpPaint = (LPPAINTSTRUCT)virtual_to_physical_addr(cpu, lpPaint);
	printf("\nCalling BeginPaint(%p, %p)", hWnd, lpPaint);
	return (uint32_t)BeginPaint((HWND)hWnd, (LPPAINTSTRUCT)_lpPaint);
}

uint32_t thunk_GetSystemMetrics(i386* cpu){
	uint32_t nIndex = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetSystemMetrics(%p)", nIndex);
	return (uint32_t)GetSystemMetrics((int)nIndex);
}

uint32_t thunk_TranslateAcceleratorA(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t hAccTable = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t lpMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	LPMSG _lpMsg = (LPMSG)virtual_to_physical_addr(cpu, lpMsg);
	printf("\nCalling TranslateAcceleratorA(%p, %p, %p)", hWnd, hAccTable, lpMsg);
	return (uint32_t)TranslateAcceleratorA((HWND)hWnd, (HACCEL)hAccTable, (LPMSG)_lpMsg);
}

uint32_t thunk_GetDC(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetDC(%p)", hWnd);
	return (uint32_t)GetDC((HWND)hWnd);
}

uint32_t thunk_ReleaseDC(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t hDC = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling ReleaseDC(%p, %p)", hWnd, hDC);
	return (uint32_t)ReleaseDC((HWND)hWnd, (HDC)hDC);
}

uint32_t thunk_SetPixel(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t color = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	printf("\nCalling SetPixel(%p, %p, %p, %p)", hdc, x, y, color);
	return (uint32_t)SetPixel((HDC)hdc, (int)x, (int)y, (COLORREF)color);
}

uint32_t thunk_InvalidateRect(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpRect = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t bErase = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	CONST RECT * _lpRect = 0;
	
	if (lpRect){
		_lpRect = (CONST RECT *)virtual_to_physical_addr(cpu, lpRect);
	}

	printf("\nCalling InvalidateRect(%p, %p, %p)", hWnd, lpRect, bErase);
	return (uint32_t)InvalidateRect((HWND)hWnd, (CONST RECT *)_lpRect, (BOOL)bErase);
}

uint32_t(*thunk_table[256])(i386*) = { thunk_MessageBoxA, thunk_ExitProcess, thunk_SetBkMode, thunk_GetModuleHandleA, thunk_LoadCursorA, thunk_LoadIconA, thunk_RegisterClassA, thunk_CreateWindowExA,
									   thunk_ShowWindow, thunk_UpdateWindow, thunk_DefWindowProcA, thunk_PeekMessageA, thunk_TranslateMessage, thunk_DispatchMessageA, thunk_GetCommandLineA, thunk_GetStartupInfoA,
									   thunk_RegisterClassExA, thunk_GetMessageA, thunk_GetStockObject, thunk_PostQuitMessage, thunk_EndPaint, thunk_DrawTextA, thunk_GetClientRect, thunk_BeginPaint,
									   thunk_GetSystemMetrics, thunk_TranslateAcceleratorA, thunk_GetDC, thunk_ReleaseDC, thunk_SetPixel, thunk_InvalidateRect};

void handle_syscall(i386* cpu){
	int function_id = cpu->eax;

	if (thunk_table[function_id] == 0){
		printf("\nUnimplemented thunk %x!", function_id);
		cpu->running = 0;
	}
	else{
		cpu->eax = thunk_table[function_id](cpu);
		printf(" and returning %08x (EIP=%p)", cpu->eax, cpu->eip);
	}
}

TAKEN_BASE* taken_bases = 0;

int is_base_taken(uint32_t base){
	TAKEN_BASE* temp = taken_bases;

	while (temp){
		if (temp->base == base) return 1;

		temp = temp->next;
	}

	return 0;
}

uint32_t find_free_base(uint32_t base){
	while (is_base_taken(base)){
		base += 0x400000;
	}

	return base;
}

LOADED_PE_IMAGE load_pe_file(const char* filename){
	LOADED_PE_IMAGE image;
	size_t sz;
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	image.data = (BYTE*)malloc(sz);
	fseek(fp, 0, SEEK_SET);
	fread(image.data, 1, sz, fp);
	fclose(fp);

	printf("Loading PE image %s\n", filename);
	image.import_table = 0;
	image.export_table = 0;

	return image;
}

/*
Returns failure if the MZ or PE headers cannot be found.
*/

int load_pe_header(LOADED_PE_IMAGE* image){
	IMAGE_DOS_HEADER* mz_header = (IMAGE_DOS_HEADER*)image->data;
	image->nt_headers = (IMAGE_NT_HEADERS*)(image->data + mz_header->e_lfanew);

	return 0;
}

void parse_optional_header(LOADED_PE_IMAGE* image, i386* cpu){
	_IMAGE_OPTIONAL_HEADER* optional_header = &(image->nt_headers->OptionalHeader);	

	if (taken_bases == 0){
		image->image_base = optional_header->ImageBase;
		taken_bases = (TAKEN_BASE*)malloc(sizeof(TAKEN_BASE));
		taken_bases->base = image->image_base;
		taken_bases->next = 0;
	}
	else{
		image->image_base = find_free_base(optional_header->ImageBase);
		TAKEN_BASE* temp = taken_bases;
		taken_bases = (TAKEN_BASE*)malloc(sizeof(TAKEN_BASE));
		taken_bases->base = image->image_base;
		taken_bases->next = temp;
	}

	image->entry_point = optional_header->AddressOfEntryPoint + image->image_base;
	
	printf("  Image Base: %p\n", image->image_base);
	printf("  Entry Point: %p\n", image->entry_point);
	printf("  Stack Size: %d bytes committed (%d reserved)\n", optional_header->SizeOfStackCommit, optional_header->SizeOfStackReserve);

	//allocate a stack if none exists
	uint32_t stack_pde = GET_PDE(STACK_BASE - 0x1000);
	uint32_t stack_pte = GET_PTE(STACK_BASE - 0x1000);

	if (cpu->page_dir.entries[stack_pde] == 0){
		cpu->page_dir.entries[stack_pde] = (i386_PT*)malloc(sizeof(i386_PT));
		memset(cpu->page_dir.entries[stack_pde], 0, sizeof(i386_PT));
	}

	if (cpu->page_dir.entries[stack_pde]->entries[stack_pte] == 0){
		virtual_mmap(cpu, STACK_BASE - 0x1000, (uint8_t*)malloc(0x1000));
		virtual_mmap(cpu, STACK_BASE, (uint8_t*)malloc(0x1000));
	}
}

void parse_data_directories(LOADED_PE_IMAGE* image, i386* cpu){
	IMAGE_DATA_DIRECTORY* data_dir;
	IMAGE_OPTIONAL_HEADER *optional_header = &(image->nt_headers->OptionalHeader);

	//parse export directory -- this one's important
	DWORD* exportAddressTable;
	WORD* nameOrdinalsPointer;
	DWORD* exportNamePointerTable;
	IMAGE_EXPORT_DIRECTORY* imageExportDirectory;
	EXPORTED_FUNCTION* temp;
	data_dir = &(optional_header->DataDirectory[0]);

	if (data_dir->Size != 0){
		imageExportDirectory = (IMAGE_EXPORT_DIRECTORY*)(virtual_to_physical_addr(cpu, data_dir->VirtualAddress + image->image_base));

		exportAddressTable = (DWORD*)virtual_to_physical_addr(cpu, image->image_base + imageExportDirectory->AddressOfFunctions);
		nameOrdinalsPointer = (WORD*)virtual_to_physical_addr(cpu, image->image_base + imageExportDirectory->AddressOfNameOrdinals);
		exportNamePointerTable = (DWORD*)virtual_to_physical_addr(cpu, image->image_base + imageExportDirectory->AddressOfNames);

		printf("  Export Table Size: %d bytes (%d entries)\n", data_dir->Size, imageExportDirectory->NumberOfNames);

		for (int nameIndex = 0; nameIndex < imageExportDirectory->NumberOfNames; nameIndex++){
			temp = image->export_table;
			image->export_table = (EXPORTED_FUNCTION*)malloc(sizeof(EXPORTED_FUNCTION));
			memset(image->export_table, 0, sizeof(image->export_table));

			image->export_table->ordinal = nameOrdinalsPointer[nameIndex];
			image->export_table->next = temp;
			image->export_table->address = image->image_base + exportAddressTable[image->export_table->ordinal];
			image->export_table->name = image->image_base + exportNamePointerTable[nameIndex];

			printf("    %p: %s (ordinal %d)\n", image->export_table->address, virtual_to_physical_addr(cpu, image->export_table->name), image->export_table->ordinal);
		}
	}

	//parse import directory -- this one's important
	//parse resource directory
	//parse exception directory
	//parse security directory
	//parse base relocation table
	//parse debug directory
	//parse arch specific data
	//parse rva of gp
	//parse tls directory
	//parse load config dir
	//parse bound import dir in headers
	//parse import address table -- seems pretty important to me
	//parse delay load import descriptors
	//parse com runtime descriptor
}

/*
	This is the function that loads each DLL and puts the address of any imported functions into the relevant area
*/

void parse_ilt(LOADED_PE_IMAGE* image, uint32_t* ilt, uint32_t rva_ilt, BYTE* addr_base, i386* cpu){
	IMPORTED_FUNCTION* temp;
	IMAGE_IMPORT_BY_NAME *name_table;

	while (1){
		if (*ilt == 0){
			break;
		}

		temp = image->import_table->fn_table;
		image->import_table->fn_table = (IMPORTED_FUNCTION*)malloc(sizeof(IMPORTED_FUNCTION));
		memset(image->import_table->fn_table, 0, sizeof(IMPORTED_FUNCTION));
		image->import_table->fn_table->next = temp;
		image->import_table->fn_table->address = rva_ilt;

		if ((*ilt) & 0x80000000){ //imported by ordinal
			printf("      (%x) Function imported by ordinal %d.\n", rva_ilt, (*ilt) & 0xFFFF);
			image->import_table->fn_table->ordinal_or_name = 0;
			image->import_table->fn_table->ordinal = (*ilt) & 0xFFFF;
		}else{ //imported by name
			name_table = (IMAGE_IMPORT_BY_NAME*)(addr_base + ((*ilt) & 0x7FFFFFFF));
			printf("      (%x) Function imported by name %s.\n", rva_ilt, name_table->Name);
			image->import_table->fn_table->ordinal_or_name = 1;
			image->import_table->fn_table->name = (char*)malloc(strlen(name_table->Name) + 1);
			strcpy(image->import_table->fn_table->name, name_table->Name);
		}

		ilt++;
		rva_ilt += 4;
	}
}

void parse_idt(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	IMPORT_TABLE* temp;
	IMAGE_IMPORT_DESCRIPTOR* import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(image->data + sec->PointerToRawData);
	IMAGE_IMPORT_DESCRIPTOR null_desc;
	BYTE* address_base = image->data + sec->PointerToRawData - sec->VirtualAddress;
	memset(&null_desc, 0, sizeof(null_desc));

	while (1){
		//check if it's zeroed out - if it is, quit the loop
		if (memcmp(import_desc, &null_desc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) == 0){
			//printf("    Done parsing IDT\n");
			break;
		}

		printf("    IAT RVA = %x, ILT RVA = %x ", import_desc->OriginalFirstThunk, import_desc->FirstThunk);
		printf("Name: %s\n", address_base + import_desc->Name);

		temp = image->import_table;
		image->import_table = (IMPORT_TABLE*)malloc(sizeof(IMPORT_TABLE));
		memset(image->import_table, 0, sizeof(IMPORT_TABLE));
		image->import_table->image_name = address_base + import_desc->Name;
		image->import_table->next = temp;

		//iterate over IAT & load binaries
		parse_ilt(image, (uint32_t*)(address_base + import_desc->OriginalFirstThunk), import_desc->FirstThunk, address_base, cpu);

		import_desc++;
	}
}

void parse_rt(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	IMAGE_BASE_RELOCATION* reloc = (IMAGE_BASE_RELOCATION*)(image->data + sec->PointerToRawData);
	IMAGE_BASE_RELOCATION null_reloc;
	uint32_t num_entries;
	WORD *reloc_entry;
	uint32_t virtual_addr;
	uint32_t base_delta = image->image_base - image->nt_headers->OptionalHeader.ImageBase;
	uint16_t offset, type;
	int p = 0;

	memset(&null_reloc, 0, sizeof(IMAGE_BASE_RELOCATION));

	while (1){
		if (memcmp(reloc, &null_reloc, sizeof(IMAGE_BASE_RELOCATION)) == 0){
			//printf("    Done parsing relocations table\n");
			break;
		}

		num_entries = (reloc->SizeOfBlock - 8) / 2;
		virtual_addr = reloc->VirtualAddress;

		//printf("    Relocation Block %d | Page RVA: %p (%d entries)\n", p, virtual_addr, num_entries);

		reloc_entry = (WORD*)((BYTE*)reloc + sizeof(IMAGE_BASE_RELOCATION));

		for (int i = 0; i < num_entries; i++){
			type = *reloc_entry >> 12;
			offset = *reloc_entry & 0xFFF;

			//printf("      Relocation Type: %01x | Offset: %03x\n", type, reloc_entry);

			switch (type){
				case 3:
					*(uint32_t*)virtual_to_physical_addr(cpu, image->image_base + virtual_addr + offset) += base_delta;
					break;
				default:
					break;
			}

			reloc_entry++;
		}

		reloc = (IMAGE_BASE_RELOCATION*)((BYTE*)reloc + reloc->SizeOfBlock);
		p++;
	}
}

void parse_text(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	//virtual_mmap(cpu, sec->VirtualAddress + image->image_base, image->data + sec->PointerToRawData); //temporary hack!

	if (cpu->eip == 0){
		cpu->eip = sec->VirtualAddress + image->image_base;
	}
}

void parse_section_headers(LOADED_PE_IMAGE* image, i386* cpu){
	IMAGE_SECTION_HEADER current_section; //reloc_pointer & num_relocs don't matter for EXEs
	image->section_headers = (IMAGE_SECTION_HEADER*)((BYTE*)&(image->nt_headers->OptionalHeader) + image->nt_headers->FileHeader.SizeOfOptionalHeader);

	for (int i = 0; i < image->nt_headers->FileHeader.NumberOfSections; i++){
		current_section = image->section_headers[i];
		printf("  Section %s\n", current_section.Name);
		printf("    Physical Address: %p Virtual Address %p Size: %d\n", current_section.PointerToRawData, current_section.VirtualAddress, current_section.SizeOfRawData);

		if (strcmp((const char*)current_section.Name, ".text") == 0){
			parse_text(image, &current_section, cpu);
		} else if(strcmp((const char*)current_section.Name, ".idata") == 0){
			parse_idt(image, &current_section, cpu);
		} else if (strcmp((const char*)current_section.Name, ".reloc") == 0){
			parse_rt(image, &current_section, cpu);
		} else if (strcmp((const char*)current_section.Name, ".data") == 0){
			map_section(cpu, current_section.VirtualAddress + image->image_base, image->data + current_section.PointerToRawData, 8192);
		}

		map_section(cpu, current_section.VirtualAddress + image->image_base, image->data + current_section.PointerToRawData, current_section.SizeOfRawData);
	}
}

void resolve_imports(LOADED_PE_IMAGE*, i386*);		

void parse_headers(LOADED_PE_IMAGE* image, i386* cpu){
	load_pe_header(image);
	if (image->nt_headers->FileHeader.SizeOfOptionalHeader != 0){
		parse_optional_header(image, cpu);
	}
	parse_section_headers(image, cpu);
	parse_data_directories(image, cpu);
	resolve_imports(image, cpu);
}

void print_export_table(LOADED_PE_IMAGE* image, i386* cpu){
	printf("EXPORT TABLE\n");
	EXPORTED_FUNCTION* table = image->export_table;

	while (table){
		printf("  %p: %s (ordinal %d)\n", table->address, virtual_to_physical_addr(cpu, table->name), table->ordinal);
		table = table->next;
	}
}

uint32_t scan_export_table_for_name(EXPORTED_FUNCTION* export_table, i386* cpu, char* name){
	while (export_table){
		if (strcmp(name, (const char*)virtual_to_physical_addr(cpu, export_table->name)) == 0){
			return export_table->address;
		}

		export_table = export_table->next;
	}
}

uint32_t scan_export_table_for_ordinal(EXPORTED_FUNCTION* export_table, WORD ordinal){
	return 0;
}

void resolve_imports(LOADED_PE_IMAGE* image, i386* cpu){
	//do a list of all imports
	IMPORT_TABLE *table = image->import_table;
	IMPORTED_FUNCTION *function;
	LOADED_PE_IMAGE new_image;
	LOADED_PE_IMAGE *cur_image;
	LOADED_IMAGE* temp;
	uint32_t addr;
	uint32_t* paddr;

	while (table){
		printf("IMPORTS FROM %s\n", table->image_name);

		if (loaded_images == 0){
			loaded_images = (LOADED_IMAGE*)malloc(sizeof(LOADED_IMAGE));
			loaded_images->next = 0;
			loaded_images->name = (char*)table->image_name;
			new_image = load_pe_file((const char*)(table->image_name));
			loaded_images->image = new_image;
			cur_image = &(loaded_images->image);
			parse_headers(cur_image, cpu);
		}
		else{
			cur_image = find_image((char*)table->image_name);
			if (cur_image == 0){
				temp = loaded_images;
				loaded_images = (LOADED_IMAGE*)malloc(sizeof(LOADED_IMAGE));
				loaded_images->next = temp;
				loaded_images->name = (char*)table->image_name;
				new_image = load_pe_file((const char*)(table->image_name));
				loaded_images->image = new_image;
				cur_image = &(loaded_images->image);
				parse_headers(cur_image, cpu);
			}
		}
		//new_image = load_pe_file((const char*)(table->image_name));
		//parse_headers(cur_image, cpu);

		function = table->fn_table;

		//load the relevant PE image and then link your import table with their export table

		while (function){
			addr = 0x0;
			printf("  (%p) ", image->image_base + function->address);

			if (function->ordinal_or_name){
				printf("%s", function->name);
				addr = scan_export_table_for_name(cur_image->export_table, cpu, function->name);
			}
			else{
				printf("%d", function->ordinal);
				addr = scan_export_table_for_ordinal(cur_image->export_table, function->ordinal);
			}

			printf(" (resolved to address %p)\n", addr);

			*(uint32_t*)(virtual_to_physical_addr(cpu, image->image_base + function->address)) = addr;

			function = function->next;
		}

		table = table->next;
	}
}

uint32_t debug_step(i386* cpu){
	FILE* fp;
	BREAKPOINT* temp;
	BREAKPOINT* temp2;
	BREAKPOINT* temp3;
	LOADED_IMAGE* cur;
	LOADED_PE_IMAGE* cur_image;
	uint32_t bp_addr;

	char option[10];
	char reg[25];
	uint32_t value, value_2;
	char buf[100];
	option[0] = 'r';
	option[1] = 0;

	if (cpu->single_step){
		value = 0;
		value_2 = 0;

		printf("(%p) ", cpu->eip);

#ifndef HEADLESS
		gets(buf);
		sscanf(buf, "%s %x %x", option, &value, &value_2);
#endif

		if (strlen(buf) == 0) return 0;

		if (strcmp(option, "dr") == 0){ //dump registers
			cpu_dump(cpu);
		}
		else if (strcmp(option, "dm") == 0){ //dump memory
			for (int p = 0; p < value_2; p++){
				printf("%p: ", value);

				for (int i = value; i < value + 16; i++){
					printf("%02x ", *virtual_to_physical_addr(cpu, i));
				}
				printf(" | ");
				for (int i = value; i < value + 16; i++){
					printf("%c", *virtual_to_physical_addr(cpu, i));
				}
				printf("\n");
				value += 16;
			}
		}
		else if (strcmp(option, "dfm") == 0){
			sscanf(buf, "%s %x %x %s", option, &value, &value_2, reg);
			fp = fopen(reg, "wb");
			value_2 = fwrite(virtual_to_physical_addr(cpu, value), 1, value_2, fp);
			fclose(fp);
			printf("Wrote %d bytes to %s\n", value_2, reg);
		}
		else if (strcmp(option, "pr") == 0){ //poke register (i.e. pr ah 0x0)
			sscanf(buf, "%s %s %x", option, reg, &value);
			set_reg(cpu, reg, value);
		}
		else if (strcmp(option, "p8") == 0){ //poke memory 8-bit (p8 40104d 3)
			*virtual_to_physical_addr(cpu, value) = value_2;
		}
		else if (strcmp(option, "p16") == 0){ //poke memory 16-bit
			*(uint16_t*)virtual_to_physical_addr(cpu, value) = value_2;
		}
		else if (strcmp(option, "p32") == 0){ //poke memory 32-bit
			*(uint32_t*)virtual_to_physical_addr(cpu, value) = value_2;
		}
		else if (strcmp(option, "s") == 0){
			cpu->running = 1;
			cpu->single_step = 1;
		}
		else if (strcmp(option, "r") == 0){
			cpu->running = 1;
			cpu->single_step = 0;
		}
		else if (strcmp(option, "b") == 0){ //list breakpoints
			temp = cpu->breakpoint;
			while (temp){
				printf("  %p\n", temp->addr);
				temp = temp->next;
			}
		}
		else if (strcmp(option, "bs") == 0){ //set breakpoint
			temp = cpu->breakpoint;
			cpu->breakpoint = (BREAKPOINT*)malloc(sizeof(BREAKPOINT));
			cpu->breakpoint->addr = value;
			cpu->breakpoint->value = *virtual_to_physical_addr(cpu, value);
			cpu->breakpoint->next = temp;
			printf("Breakpoint set at %p\n", value);
			*virtual_to_physical_addr(cpu, value) = 0xCC;
		}
		else if (strcmp(option, "br") == 0){ //remove breakpoint
			printf("Removed breakpoint at %p (but not really)\n", value);
		}
		else if (strcmp(option, "g") == 0){ //goto
			cpu->eip = value;
		}
		else if (strcmp(option, "e") == 0){ //exit
			exit(0);
		}
		else if (strcmp(option, "d") == 0){
			printf("Not yet implemented!\n");
		}
		else if (strcmp(option, "ex") == 0){
			sscanf(buf, "%s %s", option, reg);
			cur_image = find_image(reg);

			if (cur_image == 0){
				printf("No image %s present in memory.\n", reg);
			}
			else{
				print_export_table(cur_image, cpu);
			}
		}
		else if (strcmp(option, "li") == 0){
			cur = loaded_images;

			while (cur){
				printf("%s\n", cur->name);
				cur = cur->next;
			}
		}
		else if (strcmp(option, "st") == 0){
			cpu_trace(cpu);
		}
		else if (strcmp(option, "help") == 0){
			printf("sim80386 machine language monitor / debugger by Will Klees\n");
			printf("COMMAND SET\n");
			printf("d hex:addr hex:bytes - Disassembles bytes bytes from addr\n");
			printf("dr - Dump registers\n");
			printf("dm hex:addr hex:pages - Dumps 16-byte memory pages in hex and ascii starting from addr\n");
			printf("dfm hex:addr hex:bytes str:file - Dumps the specified number of bytes starting at addr to the file\n");
			printf("pr str:reg hex:val - Puts val into the desired register specified in reg\n");
			printf("p8 hex:addr hex:val - Puts the byte specified in val into addr\n");
			printf("p16 hex:addr hex:val - Puts the word specified in val into addr\n");
			printf("p32 hex:addr hex:val - Puts the dword specified in val into addr\n");
			printf("s - Single steps execution\n");
			printf("r - Begins normal execution\n");
			printf("b - Lists breakpoints\n");
			printf("bs hex:addr - Sets breakpoint at addr\n");
			printf("br hex:addr - Removes breakpoint set at addr\n");
			printf("g hex:addr - Sets EIP to addr\n");
			printf("ex str:name - Prints out all of the exports of a given PE image\n");
			printf("li - Lists images loaded into memory\n");
			printf("st - Stack trace (only works if the program sets up stack frames via EBP)\n");
			printf("e - Exits\n");
		}
		else{
			printf("Unknown command %s. Press help for more options.\n", option);
		}
	}

	if (cpu->breakpoint_hit){ //fixup the breakpoint
		cpu->fixing_breakpoint = 1;

		//find the relevant breakpoint
		temp = cpu->breakpoint;

		while (temp){
			if (temp->addr == cpu->eip){
				break;
			}
			temp = temp->next;
		}

		*virtual_to_physical_addr(cpu, cpu->eip) = temp->value;
	}

	if (cpu->running){
		cpu_step(cpu);
	}

	if (cpu->running && cpu->single_step == 0 & kbhit()){
		getch();
		printf("Break.\n");
		cpu->running = 0;
		cpu->single_step = 1;
	}

	if (cpu->fixing_breakpoint){ //return 0xCC to the breakpoint
		*virtual_to_physical_addr(cpu, temp->addr) = 0xCC;
		cpu->fixing_breakpoint = 0;
	}

	if (cpu->running == 0){
		if (cpu->escaping){
			cpu->escaping = 0;
			cpu->running = 1;
			return 1;
		}

		cpu->single_step = 1;
	}

	if (cpu->single_step){
		cpu->running = 0;
	}

	return 0;
}

uint8_t escape_routine[3] = {0xcd, 0xff, 0xc3};

int main(int argc, char* argv[])
{
	FILE* fp;

	int single_step = 1;
	char option[10];
	char reg[25];
	uint32_t value, value_2;

	char buf[100];

	i386 CPU;
	cpu_init(&CPU);
	//LOADED_PE_IMAGE hello = load_pe_file("C:\\Users\\Will\\peldr\\hellowin.exe");
	LOADED_PE_IMAGE hello = load_pe_file(argv[1]);
	parse_headers(&hello, &CPU);
	CPU.running = 0;
	CPU.single_step = 1;

	//map the escape routine into RAM somewhere
	escape_addr = 0xFFFF0000;
	global_cpu = &CPU;
	virtual_mmap(&CPU, escape_addr, escape_routine);

	while (1){
		debug_step(&CPU);
	}
}