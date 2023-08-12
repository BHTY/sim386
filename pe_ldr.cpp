// pe_ldr.cpp : Defines the entry point for the console application.
//

/*
	Program Procedure
	The PE loader module loads the relevant PE image and calls the CPU functions to map the relevant data into its address space


*/



#undef UNICODE

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#include "headers.h"
#include "../sim386.h"
#include "heap.h"

time_t start_time;

LOADED_IMAGE* loaded_images;
WINDOW_CLASS* window_classes;

uint16_t temp_window_class = 0;
REGISTERED_WINDOW_CLASS registered_window_classes[65536];
ACTIVE_WINDOW* window_list;

EMU_HINSTANCE* root_instance = 0;

uint32_t escape_addr;
i386* global_cpu;

uint32_t Address_EnvironmentStringsA = 0;
uint32_t Address_EnvironmentStringsW = 0;
uint32_t Address_CommandLine = 0;

void add_window(uint32_t hwnd, uint16_t wndclass) {
	ACTIVE_WINDOW* window = window_list;
	ACTIVE_WINDOW* prevWindow = window_list;
	ACTIVE_WINDOW* newWindow = (ACTIVE_WINDOW*)malloc(sizeof(ACTIVE_WINDOW));
	newWindow->hwnd = hwnd;
	newWindow->wndclass = wndclass;
	newWindow->timers = 0;
	newWindow->next = 0;

	if (window_list == 0) {
		window_list = newWindow;
		return;
	}

	while (window) {
		prevWindow = window;
		window = window->next;
	}

	prevWindow->next = newWindow;
	return;
}

ACTIVE_WINDOW* find_window(uint32_t hwnd) {
	ACTIVE_WINDOW* window = window_list;

	while (window) {
		if (window->hwnd == hwnd) return window;
		window = window->next;
	}

	return 0;
}

uint16_t find_window_class(uint32_t hInstance, LPWSTR name) {
	for (int i = 0; i < 65536; i++) {
		if (registered_window_classes[i].used && registered_window_classes[i].hInstance->image_base == hInstance) {
			if (wcscmp(name, registered_window_classes[i].class_name) == 0) {
				return i;
			}
		}
	}

	return 0;
}

uint16_t create_dialog_box_window_class(uint32_t wndproc, EMU_HINSTANCE* hInstance) {
	for (int i = 1; i < 65536; i++) {
		if (registered_window_classes[i].used == 0) {
			registered_window_classes[i].hMenu = 0;
			registered_window_classes[i].hInstance = hInstance;
			registered_window_classes[i].class_name = L"#32770";
			registered_window_classes[i].global = 0;
			registered_window_classes[i].used = 1;
			registered_window_classes[i].wndproc = wndproc;
			return i;
		}
	}
}

EMU_HINSTANCE* find_instance(uint32_t hInstance) {
	EMU_HINSTANCE* instance = root_instance;

	while (instance) {
		if (instance->image_base == hInstance) return instance;
		instance = instance->next;
	}

	return 0;
}

void add_instance(uint32_t image_base, uint32_t root_rsdir) {
	EMU_HINSTANCE* instance = root_instance;
	EMU_HINSTANCE* prevInstance = root_instance;
	EMU_HINSTANCE* newInstance = (EMU_HINSTANCE*)malloc(sizeof(EMU_HINSTANCE));
	newInstance->image_base = image_base;
	newInstance->root_rsdir = root_rsdir;
	newInstance->next = 0;

	if (root_instance == 0) {
		root_instance = newInstance;
		return;
	}

	while (instance) {
		prevInstance = instance;
		instance = instance->next;
	}

	prevInstance->next = newInstance;
	return;
}

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

uint32_t lookup_classname_timerproc(char* class_name){ //returns a pointer to the TimerProc
	WINDOW_CLASS* temp = window_classes;

	while (temp){
		if (strcmp(class_name, temp->class_name) == 0){ //match found
			return temp->TimerProc;
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

void register_window_timer(HWND hwnd, uint32_t timerproc, int id){
	char name_buffer[100];
	GetClassNameA(hwnd, name_buffer, 99);

	WINDOW_CLASS* temp = window_classes;

	while (temp){
		if (strcmp(name_buffer, temp->class_name) == 0){ //match found
			temp->TimerProc = timerproc;
		}

		temp = temp->next;
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

LRESULT CALLBACK dummy_TimerProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	uint32_t wndproc_addr;
	HRESULT return_value;
	LPSTR name_buffer = (LPSTR)malloc(100);
	GetClassNameA(hWnd, name_buffer, 100);
	wndproc_addr = lookup_classname_timerproc((char*)name_buffer);
	free(name_buffer);

	printf("\nThunking TimerProc(%p, %p, %p, %p)", hWnd, msg, wParam, lParam);

	if (wndproc_addr){
		cpu_push32(global_cpu, &(global_cpu->eip));
		cpu_push32(global_cpu, (uint32_t*)&lParam);
		cpu_push32(global_cpu, (uint32_t*)&wParam);
		cpu_push32(global_cpu, (uint32_t*)&msg);
		cpu_push32(global_cpu, (uint32_t*)&hWnd);

		//call the function
		return_value = (LRESULT)cpu_reversethunk(global_cpu, wndproc_addr, escape_addr); //it should return into the unthunker

		//pop the arguments off of the stack
		//global_cpu->esp += 16;

		printf(" Finished TimerProc, EIP=%p!", global_cpu->eip);

		return return_value;
	}
}

BOOL CALLBACK enumproc_icons(HWND hwnd, LPARAM lParam) {
	char string[400];
	LONG wndlong = GetWindowLongA(hwnd, GWL_STYLE);
	HICON hIcon = LoadIconA(NULL, IDI_ASTERISK);

	if ((wndlong & SS_ICON) == SS_ICON) {
		SendMessageA(hwnd, STM_SETICON, (WPARAM)hIcon, 0);
	}

	return 1;
}

LRESULT CALLBACK dummy_DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ACTIVE_WINDOW* window = find_window((uint32_t)hWnd);
	uint32_t wndproc_addr;
	LRESULT return_value;
	HICON hIcon;

	log_instructions = 1;

	printf("\nThunking DlgProc(%p, %p, %p, %p) to ", hWnd, msg, wParam, lParam);

	if (window) {
		wndproc_addr = registered_window_classes[window->wndclass].wndproc;
	}
	else {
		if (temp_window_class) { //the window is being created as we speak
			wndproc_addr = registered_window_classes[temp_window_class].wndproc;
			add_window((uint32_t)hWnd, temp_window_class);
			EnumChildWindows(hWnd, enumproc_icons, 0);
		}
		else {
			printf("No DlgProc found! (temp=%x)\n", temp_window_class);
			return DefDlgProcA(hWnd, msg, wParam, lParam);
		}
	}

	//push the args onto the stack
	cpu_push32(global_cpu, &(global_cpu->eip));
	cpu_push32(global_cpu, (uint32_t*)&lParam);
	cpu_push32(global_cpu, (uint32_t*)&wParam);
	cpu_push32(global_cpu, (uint32_t*)&msg);
	cpu_push32(global_cpu, (uint32_t*)&hWnd);
	printf("0x%p ", wndproc_addr);

	//call the function
	return_value = (LRESULT)cpu_reversethunk(global_cpu, wndproc_addr, escape_addr);

	printf(" Finished DlgProc=%p!", return_value);

	log_instructions = 0;

	return return_value;
}

LRESULT CALLBACK dummy_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	ACTIVE_WINDOW* window = find_window((uint32_t)hWnd);
	uint32_t wndproc_addr;
	LRESULT return_value;

	printf("\nThunking WndProc(%p, %p, %p, %p) to ", hWnd, msg, wParam, lParam);

	if (window) {
		wndproc_addr = registered_window_classes[window->wndclass].wndproc;
	}
	else {
		if (temp_window_class) { //the window is being created as we speak
			wndproc_addr = registered_window_classes[temp_window_class].wndproc;
			add_window((uint32_t)hWnd, temp_window_class);
		}
		else {
			printf("No WndProc found! (temp=%x)\n", temp_window_class);
			return DefWindowProcA(hWnd, msg, wParam, lParam);
		}
	}

	//push the args onto the stack
	cpu_push32(global_cpu, &(global_cpu->eip));
	cpu_push32(global_cpu, (uint32_t*)&lParam);
	cpu_push32(global_cpu, (uint32_t*)&wParam);
	cpu_push32(global_cpu, (uint32_t*)&msg);
	cpu_push32(global_cpu, (uint32_t*)&hWnd);
	printf("0x%p ", wndproc_addr);

	//call the function
	return_value = (LRESULT)cpu_reversethunk(global_cpu, wndproc_addr, escape_addr);
	printf(" Finished WndProc, EIP=%p!", global_cpu->eip);

	return return_value;
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

#ifndef HEADLESS
	cpu->running = 0;
	printf("\nThe process has been halted, but all of its handles are still active, its memory allocated, and its threads merely paused. To fully close the program, press e at the debug prompt.");
#else
	ExitProcess(uExitCode);
#endif

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
	
	if (lpModuleName) { //this indicates the name of another module, so we should find its HINSTANCE
	}

	return (uint32_t)root_instance->image_base;
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
	HICON hIcon;
	IMAGE_RESOURCE_DATA_ENTRY* data_entry;
	EMU_HINSTANCE* instance;

	GRPICONDIR *icon_dir;

	uint32_t icon_id;
	uint32_t icon_size;
	
	uint32_t hInstance = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpIconName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);

	printf("\nCalling LoadIconA(%p, %p)", hInstance, lpIconName);

	instance = find_instance(hInstance);

	if (instance == 0) {
		return (uint32_t)LoadIconA((HINSTANCE)hInstance, (LPCSTR)lpIconName);
	}
	else { //support string names
		data_entry = (IMAGE_RESOURCE_DATA_ENTRY*)virtual_to_physical_addr(cpu, find_resource(cpu, instance->image_base + instance->root_rsdir, (LPWSTR)RT_GROUP_ICON, (LPWSTR)lpIconName, 0, 0)); //fill in 3 & 0

		printf("Data entry points to %p\n", data_entry->OffsetToData);
		icon_dir = (GRPICONDIR*)virtual_to_physical_addr(cpu, data_entry->OffsetToData + instance->image_base);

		//now the smart thing to do would be to find the best icon in the icon directory but I'm just going to use the first one because lol

		icon_id = icon_dir->idEntries[0].nId;
		icon_size = icon_dir->idEntries[0].dwBytesInRes;

		data_entry = (IMAGE_RESOURCE_DATA_ENTRY*)virtual_to_physical_addr(cpu, find_resource(cpu, instance->image_base + instance->root_rsdir, (LPWSTR)RT_ICON, (LPWSTR)icon_id, 0, 0));

		hIcon = CreateIconFromResource(virtual_to_physical_addr(cpu, data_entry->OffsetToData + instance->image_base), icon_size, 1, 0x00030000);

		return (uint32_t)hIcon;
	}
}

uint32_t thunk_RegisterClassA(i386* cpu){
	IMAGE_RESOURCE_DATA_ENTRY* menu_data_entry;
	LPWSTR menu_name;

	REGISTERED_WINDOW_CLASS window_class;
	uint16_t return_atom = 0;
	uint32_t lpWndClass = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	WNDCLASSA* wc = (WNDCLASSA*)virtual_to_physical_addr(cpu, lpWndClass);

	printf("\nCalling RegisterClassA(%p) with result %04x", lpWndClass);

	//replace wc->lpszClassName
	if (wc->lpszClassName != 0) {
		wc->lpszClassName = (LPSTR)virtual_to_physical_addr(cpu, (uint32_t)wc->lpszClassName);
		window_class.class_name = (LPWSTR)malloc(strlen(wc->lpszClassName) * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, wc->lpszClassName, -1, window_class.class_name, strlen(wc->lpszClassName) + 1);
	}
	
	//fill in window_class with the core variables (including a copy of the class name)
	window_class.global = 0;
	window_class.wndproc = (uint32_t)wc->lpfnWndProc;
	window_class.hInstance = find_instance((uint32_t)wc->hInstance);
	window_class.used = 1;
	window_class.hMenu = 0;
	
	//find the menu and then use LoadMenuIndirectA
	if (wc->lpszMenuName) {
		if ((uint32_t)wc->lpszMenuName > 65535) {
			menu_name = (LPWSTR)malloc(strlen(wc->lpszMenuName) * 2 + 2);
			MultiByteToWideChar(CP_ACP, 0, wc->lpszMenuName, -1, menu_name, strlen(wc->lpszMenuName) + 1);
		}
		else {
			menu_name = (LPWSTR)wc->lpszMenuName;
		}
		menu_data_entry = (IMAGE_RESOURCE_DATA_ENTRY*)virtual_to_physical_addr(cpu, find_resource(cpu, window_class.hInstance->image_base + window_class.hInstance->root_rsdir, (LPWSTR)RT_MENU, menu_name, 0, (uint32_t)wc->lpszMenuName > 65535));
		window_class.hMenu = LoadMenuIndirectA(virtual_to_physical_addr(cpu, window_class.hInstance->image_base + menu_data_entry->OffsetToData));
		printf("\n%p\n", window_class.hMenu);
		wc->lpszMenuName = 0;
	}

	//dummy out WndProc & replace HINSTANCE with GetModuleHandle(NULL)
	wc->lpfnWndProc = dummy_WndProc;
	wc->hInstance = GetModuleHandle(NULL);

	//call RegisterClassA and store the returned atom in window_class
	return_atom = RegisterClassA(wc);
	//store into the window class table and return
	if (return_atom) {
		registered_window_classes[return_atom] = window_class;
	}

	return return_atom;
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

	uint16_t wndclass;
	LPSTR _lpClassName;
	LPWSTR wide_lpClassName;
	LPSTR _lpWindowName = 0;
	LPVOID _lpParam = 0;

	printf("\nCalling CreateWindowExA(%p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p)", dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	if (lpClassName) {
		if (lpClassName < 65536) { //atom
			_lpClassName = (LPSTR)lpClassName;
			wndclass = (uint16_t)_lpClassName;
		}
		else { //string
			_lpClassName = (LPSTR)virtual_to_physical_addr(cpu, lpClassName);
			wide_lpClassName = (LPWSTR)malloc(strlen(_lpClassName) * 2 + 2);
			MultiByteToWideChar(CP_ACP, 0, _lpClassName, -1, wide_lpClassName, strlen(_lpClassName) + 1);
			wndclass = find_window_class(hInstance, wide_lpClassName);
			free(wide_lpClassName);
		}
	}

	if (lpWindowName) {
		_lpWindowName = (LPSTR)virtual_to_physical_addr(cpu, lpWindowName);
	}

	if (hMenu == 0) {
		hMenu = (uint32_t)registered_window_classes[wndclass].hMenu;
	}

	if (lpParam) {
		_lpParam = (LPVOID)virtual_to_physical_addr(cpu, lpParam);
	}

	hInstance = (uint32_t)GetModuleHandle(NULL);

	//we need to save the window class into a temporary variable so that WndProc can work before the application is done being set up
	temp_window_class = wndclass;

	uint32_t retval = (uint32_t)CreateWindowExA(dwExStyle, _lpClassName, _lpWindowName, dwStyle, X, Y, nWidth, nHeight, (HWND)hWndParent, (HMENU)hMenu, (HINSTANCE)hInstance, _lpParam);
	temp_window_class = 0;

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
	//map command line into memory
	LPSTR cmdline;
	uint32_t value;

	printf("\nCalling GetCommandLineA()");

	if (Address_CommandLine == 0){
		cmdline = GetCommandLineA();
		value = scan_free_address_space(cpu, 0x1, 0x1);
		virtual_mmap(cpu, value, (uint8_t*)cmdline);
		reserve_address_space(cpu, value, 0x1);
		Address_CommandLine = value;
	}

	return Address_CommandLine;
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

	printf("\nNot really, this function is broken!\n");

	while (1);

	return RegisterClassExA(wc);
}

uint32_t thunk_GetMessageA(i386* cpu){
	uint32_t lpMsg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
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
	uint32_t cchText = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
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

uint32_t thunk_HeapAlloc(i386* cpu){
	uint32_t hHeap = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t dwFlags = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t dwBytes = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling HeapAlloc(%p, %p, %p)", hHeap, dwFlags, dwBytes);
	uint32_t pointer = heap_alloc(cpu, hHeap, dwBytes);

	if (pointer && (1 || (dwFlags & HEAP_ZERO_MEMORY))){
		memset(virtual_to_physical_addr(cpu, pointer), 0, dwBytes);
		printf(" (zeroing memory)");
	}

	return pointer;
}

uint32_t thunk_HeapCreate(i386* cpu){
	uint32_t flOptions = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t dwInitialSize = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t dwMaximumSize = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling HeapCreate(%p, %p, %p)", flOptions, dwInitialSize, dwMaximumSize);
	uint32_t heap_handle = alloc_heap(cpu, dwInitialSize, dwMaximumSize);
	/*while (1);
	return (uint32_t)HeapCreate((DWORD)flOptions, (SIZE_T)dwInitialSize, (SIZE_T)dwMaximumSize);*/
	return heap_handle;
}

uint32_t thunk_GetStdHandle(i386* cpu){
	uint32_t nStdHandle = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetStdHandle(%p)", nStdHandle);
	return (uint32_t)GetStdHandle((DWORD)nStdHandle);
}

uint32_t thunk_GetFileType(i386* cpu){
	uint32_t hFile = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetFileType(%p)", hFile);
	return (uint32_t)GetFileType((HANDLE)hFile);
}

uint32_t thunk_SetHandleCount(i386* cpu){
	uint32_t uNumber = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling SetHandleCount(%p)", uNumber);
	return (uint32_t)SetHandleCount((UINT)uNumber);
}

uint32_t thunk_GetACP(i386* cpu){
	printf("\nCalling GetACP()");
	return (uint32_t)GetACP();
}

uint32_t thunk_GetCPInfo(i386* cpu){
	uint32_t CodePage = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpCPInfo = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPCPINFO _lpCPInfo = (LPCPINFO)virtual_to_physical_addr(cpu, lpCPInfo);
	printf("\nCalling GetCPInfo(%p, %p)", CodePage, lpCPInfo);
	return (uint32_t)GetCPInfo((UINT)CodePage, (LPCPINFO)_lpCPInfo);
}

uint32_t length_EnvironmentStringsA(LPCH env){ //returns length in pages
	int i = 0;
	WORD* ptr = (WORD*)env;

	while (*ptr){
		ptr++;
		i += 2;
	}

	return ALIGN(i, 4096) / 4096;
}

uint32_t length_EnvironmentStringsW(LPWCH env){ //returns length in pages
	int i = 0;
	DWORD* ptr = (DWORD*)env;

	while (*ptr){
		ptr++;
		i+=4;
	}

	return ALIGN(i, 4096) / 4096;
}

uint32_t thunk_GetEnvironmentStringsW(i386* cpu){
	//map environment variables into RAM
	LPWCH env;
	uint32_t value;
	uint32_t length;
	
	if (Address_EnvironmentStringsW == 0){
		env = GetEnvironmentStringsW();
		length = length_EnvironmentStringsW(env);
		value = scan_free_address_space(cpu, length, 0x1);
		map_section(cpu, value, (uint8_t*)env, length * 0x1000);
		reserve_address_space(cpu, value, length);
		Address_EnvironmentStringsW = value;
	}
	printf("\nCalling GetEnvironmentStringsW()");
	return Address_EnvironmentStringsW;
}

uint32_t thunk_FreeEnvironmentStringsW(i386* cpu){
	uint32_t penv = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	LPWCH _penv = (LPWCH)virtual_to_physical_addr(cpu, penv);
	printf("\nCalling FreeEnvironmentStringsW(%p)", penv);
	return (uint32_t)FreeEnvironmentStringsW((LPWCH)_penv);
}

uint32_t thunk_WideCharToMultiByte(i386* cpu){
	uint32_t CodePage = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t dwFlags = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t lpWideCharStr = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t cchWideChar = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t lpMultiByteStr = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	uint32_t cbMultiByte = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 24);
	uint32_t lpDefaultChar = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 28);
	uint32_t lpUsedDefaultChar = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 32);
	LPCWCH _lpWideCharStr = (LPCWCH)virtual_to_physical_addr(cpu, lpWideCharStr);
	LPSTR _lpMultiByteStr = 0;
	if (lpMultiByteStr){
		_lpMultiByteStr = (LPSTR)virtual_to_physical_addr(cpu, lpMultiByteStr);
	}
	LPCCH _lpDefaultChar = 0;
	if (lpDefaultChar){
		_lpDefaultChar = (LPCCH)virtual_to_physical_addr(cpu, lpDefaultChar);
	}
	LPBOOL _lpUsedDefaultChar = 0;
	if (lpUsedDefaultChar){
		_lpUsedDefaultChar = (LPBOOL)virtual_to_physical_addr(cpu, lpUsedDefaultChar);
	}
	printf("\nCalling WideCharToMultiByte(%p, %p, %p, %p, %p, %p, %p, %p)", CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
	return (uint32_t)WideCharToMultiByte((UINT)CodePage, (DWORD)dwFlags, (LPCWCH)_lpWideCharStr, (int)cchWideChar, (LPSTR)_lpMultiByteStr, (int)cbMultiByte, (LPCCH)_lpDefaultChar, (LPBOOL)_lpUsedDefaultChar);
}

uint32_t thunk_GetModuleFileNameA(i386* cpu){
	uint32_t hModule = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpFilename = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t nSize = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	LPSTR _lpFilename = (LPSTR)virtual_to_physical_addr(cpu, lpFilename);
	printf("\nCalling GetModuleFileNameA(%p, %p, %p)", hModule, lpFilename, nSize);
	return (uint32_t)GetModuleFileNameA((HMODULE)hModule, (LPSTR)_lpFilename, (DWORD)nSize);
}

uint32_t thunk_WriteFile(i386* cpu){
	uint32_t hFile = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpBuffer = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t nNumberOfBytesToWrite = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lpNumberOfBytesWritten = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t lpOverlapped = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	LPCVOID _lpBuffer = (LPCVOID)virtual_to_physical_addr(cpu, lpBuffer);
	LPDWORD _lpNumberOfBytesWritten = 0;
	if (lpNumberOfBytesWritten){
		_lpNumberOfBytesWritten = (LPDWORD)virtual_to_physical_addr(cpu, lpNumberOfBytesWritten);
	}
	LPOVERLAPPED _lpOverlapped = 0;
	if (lpOverlapped){
		_lpOverlapped = (LPOVERLAPPED)virtual_to_physical_addr(cpu, lpOverlapped);
	}
	printf("\nCalling WriteFile(%p, %p, %p, %p, %p)", hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);

	return (uint32_t)WriteFile((HANDLE)hFile, (LPCVOID)_lpBuffer, (DWORD)nNumberOfBytesToWrite, (LPDWORD)_lpNumberOfBytesWritten, (LPOVERLAPPED)_lpOverlapped);
}

uint32_t thunk_HeapFree(i386* cpu){
	uint32_t hHeap = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t dwFlags = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t lpMem = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nStubbing HeapFree(%p, %p, %p)", hHeap, dwFlags, lpMem);
	return 1;
}

uint32_t thunk_GetDeviceCaps(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t index = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling GetDeviceCaps(%p, %p)", hdc, index);
	return (uint32_t)GetDeviceCaps((HDC)hdc, (int)index);
}

uint32_t thunk_CreateSolidBrush(i386* cpu){
	uint32_t color = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling CreateSolidBrush(%p)", color);
	return (uint32_t)CreateSolidBrush((COLORREF)color);
}

uint32_t thunk_LoadStringA(i386* cpu){
	IMAGE_RESOURCE_DATA_ENTRY* data_entry;
	uint32_t string_table;
	uint16_t* string_ptr;

	uint32_t hInstance = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t uID = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t lpBuffer = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t cchBufferMax = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	LPSTR _lpBuffer = (LPSTR)virtual_to_physical_addr(cpu, lpBuffer);

	data_entry = (IMAGE_RESOURCE_DATA_ENTRY*)virtual_to_physical_addr(cpu, find_resource(cpu, root_instance->image_base + root_instance->root_rsdir, (LPWSTR)RT_STRING, (LPWSTR)1, 0, 0));
	string_table = data_entry->OffsetToData + root_instance->image_base;
	string_ptr = (uint16_t*)virtual_to_physical_addr(cpu, find_string(cpu, string_table, uID));

	printf("\nCalling LoadStringA(%p, %p, %p, %p)", hInstance, uID, lpBuffer, cchBufferMax);

	return WideCharToMultiByte(CP_ACP, 0, (LPWCH)(string_ptr + 1), *string_ptr, _lpBuffer, cchBufferMax, NULL, NULL);
}

uint32_t thunk_GetTextMetricsA(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lptm = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPTEXTMETRICA _lptm = (LPTEXTMETRICA)virtual_to_physical_addr(cpu, lptm);
	printf("\nCalling GetTextMetricsA(%p, %p)", hdc, lptm);
	return (uint32_t)GetTextMetricsA((HDC)hdc, (LPTEXTMETRICA)_lptm);
}

uint32_t thunk_SelectObject(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t h = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling SelectObject(%p, %p)", hdc, h);
	return (uint32_t)SelectObject((HDC)hdc, (HGDIOBJ)h);
}

uint32_t thunk_PatBlt(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t w = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t h = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	uint32_t rop = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 24);
	printf("\nCalling PatBlt(%p, %p, %p, %p, %p, %p)", hdc, x, y, w, h, rop);
	return (uint32_t)PatBlt((HDC)hdc, (int)x, (int)y, (int)w, (int)h, (DWORD)rop);
}

uint32_t thunk_SetWindowTextA(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpString = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPCSTR _lpString = 0;
	if (lpString){
		_lpString = (LPCSTR)virtual_to_physical_addr(cpu, lpString);
	}
	printf("\nCalling SetWindowTextA(%p, %p)", hWnd, lpString);
	return (uint32_t)SetWindowTextA((HWND)hWnd, (LPCSTR)_lpString);
}

uint32_t thunk_GetMenu(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetMenu(%p)", hWnd);
	return (uint32_t)GetMenu((HWND)hWnd);
}

uint32_t thunk_CheckMenuItem(i386* cpu){
	uint32_t hMenu = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t uIDCheckItem = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t uCheck = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling CheckMenuItem(%p, %p, %p)", hMenu, uIDCheckItem, uCheck);
	return (uint32_t)CheckMenuItem((HMENU)hMenu, (UINT)uIDCheckItem, (UINT)uCheck);
}

uint32_t thunk_SetCursorPos(i386* cpu){
	uint32_t X = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t Y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling SetCursorPos(%p, %p)", X, Y);
	return (uint32_t)SetCursorPos((int)X, (int)Y);
}

uint32_t thunk_ClientToScreen(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpPoint = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	LPPOINT _lpPoint = (LPPOINT)virtual_to_physical_addr(cpu, lpPoint);
	printf("\nCalling ClientToScreen(%p, %p)", hWnd, lpPoint);
	return (uint32_t)ClientToScreen((HWND)hWnd, (LPPOINT)_lpPoint);
}

uint32_t thunk_LineTo(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling LineTo(%p, %p, %p)", hdc, x, y);
	return (uint32_t)LineTo((HDC)hdc, (int)x, (int)y);
}

uint32_t thunk_MoveToEx(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lppt = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	LPPOINT _lppt = 0;
	if (lppt){
		_lppt = (LPPOINT)virtual_to_physical_addr(cpu, lppt);
	}
	printf("\nCalling MoveToEx(%p, %p, %p, %p)", hdc, x, y, lppt);
	return (uint32_t)MoveToEx((HDC)hdc, (int)x, (int)y, (LPPOINT)_lppt);
}

uint32_t thunk_TextOutA(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lpString = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t c = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	LPCSTR _lpString = (LPCSTR)virtual_to_physical_addr(cpu, lpString);
	printf("\nCalling TextOutA(%p, %p, %p, %p, %p)", hdc, x, y, lpString, c);
	return (uint32_t)TextOutA((HDC)hdc, (int)x, (int)y, (LPCSTR)_lpString, (int)c);
}

uint32_t thunk_GetTextExtentPointA(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpString = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t c = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lpsz = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	LPCSTR _lpString = (LPCSTR)virtual_to_physical_addr(cpu, lpString);
	LPSIZE _lpsz = (LPSIZE)virtual_to_physical_addr(cpu, lpsz);
	printf("\nCalling GetTextExtentPointA(%p, %p, %p, %p)", hdc, lpString, c, lpsz);
	return (uint32_t)GetTextExtentPointA((HDC)hdc, (LPCSTR)_lpString, (int)c, (LPSIZE)_lpsz);
}

uint32_t thunk_DeleteObject(i386* cpu){
	uint32_t ho = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling DeleteObject(%p)", ho);
	return (uint32_t)DeleteObject((HGDIOBJ)ho);
}

uint32_t thunk_SetCapture(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling SetCapture(%p)", hWnd);
	return (uint32_t)SetCapture((HWND)hWnd);
}

uint32_t thunk_ReleaseCapture(i386* cpu){
	printf("\nCalling ReleaseCapture()");
	return (uint32_t)ReleaseCapture();
}

uint32_t thunk_ShowCursor(i386* cpu){
	uint32_t bShow = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling ShowCursor(%p)", bShow);
	return (uint32_t)ShowCursor((BOOL)bShow);
}

uint32_t thunk_SetCursor(i386* cpu){
	uint32_t hCursor = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling SetCursor(%p)", hCursor);
	return (uint32_t)SetCursor((HCURSOR)hCursor);
}

uint32_t thunk_SetFocus(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling SetFocus(%p)", hWnd);
	return (uint32_t)SetFocus((HWND)hWnd);
}

uint32_t thunk_PostMessageA(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t Msg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t wParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	//LPARAM _lParam = (LPARAM)virtual_to_physical_addr(cpu, lParam);
	printf("\nCalling PostMessageA(%p, %p, %p, %p)", hWnd, Msg, wParam, lParam);
	return (uint32_t)PostMessageA((HWND)hWnd, (UINT)Msg, (WPARAM)wParam, (LPARAM)lParam);
}

uint32_t thunk_EnableMenuItem(i386* cpu){
	uint32_t hMenu = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t uIDEnableItem = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t uEnable = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling EnableMenuItem(%p, %p, %p)", hMenu, uIDEnableItem, uEnable);
	return (uint32_t)EnableMenuItem((HMENU)hMenu, (UINT)uIDEnableItem, (UINT)uEnable);
}

uint32_t thunk_IsIconic(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling IsIconic(%p)", hWnd);
	return (uint32_t)IsIconic((HWND)hWnd);
}

uint32_t thunk_GetKeyState(i386* cpu){
	uint32_t nVirtKey = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetKeyState(%p)", nVirtKey);
	return (uint32_t)GetKeyState((int)nVirtKey);
}

uint32_t thunk_SetTimer(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t nIDEvent = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t uElapse = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lpTimerFunc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	TIMERPROC _lpTimerFunc = 0;

	if (lpTimerFunc){
		_lpTimerFunc = (TIMERPROC)dummy_TimerProc;
		register_window_timer((HWND)hWnd, lpTimerFunc, nIDEvent);
	}

	printf("\nCalling SetTimer(%p, %p, %p, %p)", hWnd, nIDEvent, uElapse, lpTimerFunc);
	return (uint32_t)SetTimer((HWND)hWnd, (UINT_PTR)nIDEvent, (UINT)uElapse, (TIMERPROC)_lpTimerFunc);
}

uint32_t thunk_KillTimer(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t uIDEvent = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling KillTimer(%p, %p)", hWnd, uIDEvent);
	return (uint32_t)KillTimer((HWND)hWnd, (UINT_PTR)uIDEvent);
}

uint32_t thunk_Ellipse(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t left = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t top = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t right = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t bottom = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	printf("\nCalling Ellipse(%p, %p, %p, %p, %p)", hdc, left, top, right, bottom);
	return (uint32_t)Ellipse((HDC)hdc, (int)left, (int)top, (int)right, (int)bottom);
}

uint32_t thunk_ShellAboutA(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t szApp = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t szOtherStuff = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t hIcon = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	LPCSTR _szApp = (LPCSTR)virtual_to_physical_addr(cpu, szApp);
	LPCSTR _szOtherStuff = 0;
	if (szOtherStuff){
		_szOtherStuff = (LPCSTR)virtual_to_physical_addr(cpu, szOtherStuff);
	}
	printf("\nCalling ShellAboutA(%p, %p, %p, %p)", hWnd, szApp, szOtherStuff, hIcon);
	return (uint32_t)ShellAboutA((HWND)hWnd, (LPCSTR)_szApp, (LPCSTR)_szOtherStuff, (HICON)hIcon);
}

uint32_t thunk_GetWindowDC(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling GetWindowDC(%p)", hWnd);
	return (uint32_t)GetWindowDC((HWND)hWnd);
}

uint32_t thunk_SendMessageA(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t Msg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t wParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	printf("\nCalling SendMessageA(%p, %p, %p, %p)", hWnd, Msg, wParam, lParam);
	return (uint32_t)SendMessageA((HWND)hWnd, (UINT)Msg, (WPARAM)wParam, (LPARAM)lParam);
}

uint32_t thunk_GetDlgItem(i386* cpu){
	uint32_t hDlg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t nIDDlgItem = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling GetDlgItem(%p, %p)", hDlg, nIDDlgItem);
	return (uint32_t)GetDlgItem((HWND)hDlg, (int)nIDDlgItem);
}

uint32_t thunk_GetDlgItemInt(i386* cpu){
	uint32_t hDlg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t nIDDlgItem = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t lpTranslated = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t bSigned = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	BOOL * _lpTranslated = 0;
	if (lpTranslated){
		_lpTranslated = (BOOL *)virtual_to_physical_addr(cpu, lpTranslated);
	}
	printf("\nCalling GetDlgItemInt(%p, %p, %p, %p)", hDlg, nIDDlgItem, lpTranslated, bSigned);
	return (uint32_t)GetDlgItemInt((HWND)hDlg, (int)nIDDlgItem, (BOOL *)_lpTranslated, (BOOL)bSigned);
}

uint32_t thunk_DrawMenuBar(i386* cpu){
	uint32_t hWnd = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling DrawMenuBar(%p)", hWnd);
	return (uint32_t)DrawMenuBar((HWND)hWnd);
}

uint32_t thunk_GetEnvironmentStringsA(i386* cpu){
	//map environment variables into RAM
	LPCH env;
	uint32_t value;
	uint32_t length;

	if (Address_EnvironmentStringsA == 0){
		env = GetEnvironmentStringsA();
		length = length_EnvironmentStringsA(env);
		value = scan_free_address_space(cpu, length, 0x1);
		map_section(cpu, value, (uint8_t*)env, length * 0x1000);
		reserve_address_space(cpu, value, length);
		Address_EnvironmentStringsA = value;
	}
	printf("\nCalling GetEnvironmentStringsA()");
	return Address_EnvironmentStringsA;
}

uint32_t thunk_CreateICA(i386* cpu){
	uint32_t pszDriver = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t pszDevice = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t pszPort = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t pdm = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	LPCSTR _pszDriver = 0;
	if (pszDriver){
		_pszDriver = (LPCSTR)virtual_to_physical_addr(cpu, pszDriver);
	}
	LPCSTR _pszDevice = 0;
	if (pszDevice){
		_pszDevice = (LPCSTR)virtual_to_physical_addr(cpu, pszDevice);
	}
	LPCSTR _pszPort = 0;
	if (pszPort){
		_pszPort = (LPCSTR)virtual_to_physical_addr(cpu, pszPort);
	}
	CONST DEVMODEA * _pdm = 0;
	if (pdm){
		_pdm = (CONST DEVMODEA *)virtual_to_physical_addr(cpu, pdm);
	}
	printf("\nCalling CreateICA(%p, %p, %p, %p)", pszDriver, pszDevice, pszPort, pdm);
	return (uint32_t)CreateICA((LPCSTR)_pszDriver, (LPCSTR)_pszDevice, (LPCSTR)_pszPort, (CONST DEVMODEA *)_pdm);
}

uint32_t thunk_StretchBlt(i386* cpu){
	uint32_t hdcDest = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t xDest = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t yDest = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t wDest = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t hDest = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	uint32_t hdcSrc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 24);
	uint32_t xSrc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 28);
	uint32_t ySrc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 32);
	uint32_t wSrc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 36);
	uint32_t hSrc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 40);
	uint32_t rop = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 44);
	printf("\nCalling StretchBlt(%p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p)", hdcDest, xDest, yDest, wDest, hDest, hdcSrc, xSrc, ySrc, wSrc, hSrc, rop);
	return (uint32_t)StretchBlt((HDC)hdcDest, (int)xDest, (int)yDest, (int)wDest, (int)hDest, (HDC)hdcSrc, (int)xSrc, (int)ySrc, (int)wSrc, (int)hSrc, (DWORD)rop);
}

uint32_t thunk_GetPixel(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling GetPixel(%p, %p, %p)", hdc, x, y);
	return (uint32_t)GetPixel((HDC)hdc, (int)x, (int)y);
}

uint32_t thunk_BitBlt(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t x = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t y = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t cx = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t cy = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	uint32_t hdcSrc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 24);
	uint32_t x1 = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 28);
	uint32_t y1 = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 32);
	uint32_t rop = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 36);
	printf("\nCalling BitBlt(%p, %p, %p, %p, %p, %p, %p, %p, %p)", hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	return (uint32_t)BitBlt((HDC)hdc, (int)x, (int)y, (int)cx, (int)cy, (HDC)hdcSrc, (int)x1, (int)y1, (DWORD)rop);
}

uint32_t thunk_CreateCompatibleDC(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling CreateCompatibleDC(%p)", hdc);
	return (uint32_t)CreateCompatibleDC((HDC)hdc);
}

uint32_t thunk_CreateCompatibleBitmap(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t cx = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t cy = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling CreateCompatibleBitmap(%p, %p, %p)", hdc, cx, cy);
	return (uint32_t)CreateCompatibleBitmap((HDC)hdc, (int)cx, (int)cy);
}

uint32_t thunk_CreatePen(i386* cpu){
	uint32_t iStyle = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t cWidth = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t color = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	printf("\nCalling CreatePen(%p, %p, %p)", iStyle, cWidth, color);
	return (uint32_t)CreatePen((int)iStyle, (int)cWidth, (COLORREF)color);
}

uint32_t thunk_DeleteDC(i386* cpu){
	uint32_t hdc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	printf("\nCalling DeleteDC(%p)", hdc);
	return (uint32_t)DeleteDC((HDC)hdc);
}

uint32_t thunk_ReadFile(i386* cpu){
	uint32_t hFile = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpBuffer = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t nNumberOfBytesToRead = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lpNumberOfBytesRead = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t lpOverlapped = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	LPVOID _lpBuffer = (LPVOID)virtual_to_physical_addr(cpu, lpBuffer);
	LPDWORD _lpNumberOfBytesRead = 0;
	if (lpNumberOfBytesRead){
		_lpNumberOfBytesRead = (LPDWORD)virtual_to_physical_addr(cpu, lpNumberOfBytesRead);
	}
	LPOVERLAPPED _lpOverlapped = 0;
	if (lpOverlapped){
		_lpOverlapped = (LPOVERLAPPED)virtual_to_physical_addr(cpu, lpOverlapped);
	}
	printf("\nCalling ReadFile(%p, %p, %p, %p, %p)", hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	return (uint32_t)ReadFile((HANDLE)hFile, (LPVOID)_lpBuffer, (DWORD)nNumberOfBytesToRead, (LPDWORD)_lpNumberOfBytesRead, (LPOVERLAPPED)_lpOverlapped);
}

uint32_t thunk_DialogBoxParamA(i386* cpu) {
	IMAGE_RESOURCE_DATA_ENTRY *data_entry;
	HWND return_value;
	LPCDLGTEMPLATEA lpTemplate;
	LPWSTR resource_id;
	EMU_HINSTANCE* instance;

	uint32_t hInstance = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t lpTemplateName = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	uint32_t hWndParent = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 12);
	uint32_t lpDialogFunc = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 16);
	uint32_t dwInitParam = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 20);
	printf("\nCalling DialogBoxParamA(%p, %p, %p, %p, %p)", hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	
	LPCTSTR _lpTemplateName = 0;
	if (!lpTemplateName) return 0;

	if (lpTemplateName < 65536) { //ID
		resource_id = (LPWSTR)lpTemplateName;
	}
	else { //name
		_lpTemplateName = (LPCTSTR)virtual_to_physical_addr(cpu, lpTemplateName);
		resource_id = (LPWSTR)malloc(strlen(_lpTemplateName) * 2 + 2);
		MultiByteToWideChar(CP_ACP, 0, _lpTemplateName, -1, resource_id, strlen(_lpTemplateName) + 1);
	}

	instance = find_instance(hInstance);
	if (instance == 0) return 0;

	data_entry = (IMAGE_RESOURCE_DATA_ENTRY*)virtual_to_physical_addr(cpu, find_resource(cpu, instance->image_base + instance->root_rsdir, (LPWSTR)RT_DIALOG, resource_id, 0, lpTemplateName > 65535));
	lpTemplate = (LPCDLGTEMPLATEA)virtual_to_physical_addr(cpu, data_entry->OffsetToData + instance->image_base);

	//parse through the structure and find all of the icons - what to do with that info???
		
	temp_window_class = create_dialog_box_window_class(lpDialogFunc, instance);
	//return_value = CreateDialogIndirectParamA(GetModuleHandle(NULL), lpTemplate, (HWND)hWndParent, (DLGPROC)dummy_DlgProc, dwInitParam);
	return_value = (HWND)DialogBoxIndirectParamA(GetModuleHandle(NULL), lpTemplate, (HWND)hWndParent, (DLGPROC)dummy_DlgProc, dwInitParam);
	temp_window_class = 0;

	return (uint32_t)return_value;
}

uint32_t thunk_EndDialog(i386* cpu) {
	uint32_t hDlg = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 4);
	uint32_t nResult = *(uint32_t*)virtual_to_physical_addr(cpu, cpu->esp + 8);
	printf("\nCalling EndDialog(%p, %p)", hDlg, nResult);
	return (uint32_t)EndDialog((HWND)hDlg, (INT_PTR)nResult);
}

uint32_t(*thunk_table[256])(i386*) = { thunk_MessageBoxA, thunk_ExitProcess, thunk_SetBkMode, thunk_GetModuleHandleA, thunk_LoadCursorA, thunk_LoadIconA, thunk_RegisterClassA, thunk_CreateWindowExA, //0x00 - 0x07
									   thunk_ShowWindow, thunk_UpdateWindow, thunk_DefWindowProcA, thunk_PeekMessageA, thunk_TranslateMessage, thunk_DispatchMessageA, thunk_GetCommandLineA, thunk_GetStartupInfoA, //0x08 - 0x0F
									   thunk_RegisterClassExA, thunk_GetMessageA, thunk_GetStockObject, thunk_PostQuitMessage, thunk_EndPaint, thunk_DrawTextA, thunk_GetClientRect, thunk_BeginPaint, //0x10 - 0x17
									   thunk_GetSystemMetrics, thunk_TranslateAcceleratorA, thunk_GetDC, thunk_ReleaseDC, thunk_SetPixel, thunk_InvalidateRect, thunk_HeapAlloc, thunk_HeapCreate, //0x18 - 0x1F
									   thunk_GetStdHandle, thunk_GetFileType, thunk_SetHandleCount, thunk_GetACP, thunk_GetCPInfo, thunk_GetEnvironmentStringsW, thunk_FreeEnvironmentStringsW, thunk_WideCharToMultiByte, //0x20 - 0x27
									   thunk_GetModuleFileNameA, thunk_WriteFile, thunk_HeapFree, thunk_GetDeviceCaps, thunk_CreateSolidBrush, thunk_LoadStringA, thunk_GetTextMetricsA, thunk_SelectObject, //0x28 - 0x2F
									   thunk_PatBlt, thunk_SetWindowTextA, thunk_GetMenu, thunk_CheckMenuItem, thunk_SetCursorPos, thunk_ClientToScreen, thunk_LineTo, thunk_MoveToEx, //0x30 - 0x37
									   thunk_TextOutA, thunk_GetTextExtentPointA, thunk_DeleteObject, thunk_SetCapture, thunk_ReleaseCapture, thunk_ShowCursor, thunk_SetCursor, thunk_SetFocus, //0x38 - 0x3F
									   thunk_PostMessageA, thunk_EnableMenuItem, thunk_IsIconic, thunk_GetKeyState, thunk_SetTimer, thunk_KillTimer, thunk_Ellipse, thunk_ShellAboutA, //0x40 - 0x47
									   thunk_GetWindowDC, thunk_SendMessageA, thunk_GetDlgItem, thunk_GetDlgItemInt, thunk_DrawMenuBar, thunk_GetEnvironmentStringsA, thunk_CreateICA, thunk_StretchBlt, //0x48 - 0x4F
									   thunk_GetPixel, thunk_BitBlt, thunk_CreateCompatibleDC, thunk_CreateCompatibleBitmap, thunk_CreatePen, thunk_DeleteDC, thunk_ReadFile, thunk_DialogBoxParamA, //0x50 - 0x57
									   thunk_EndDialog, 0, 0, 0, 0, 0, 0, 0};

void handle_syscall(i386* cpu){
	int function_id = cpu->eax;

	if (thunk_table[function_id] == 0){
		printf("\nUnimplemented thunk %x!", function_id);
		cpu->running = 0;
	}
	else{
		//printf("Thunk ID %02x\n", function_id);
		//printf("Function pointer %p\n", thunk_table[function_id]);
		cpu->eax = thunk_table[function_id](cpu);
		printf(" and returning %08x", cpu->eax, cpu->eip);
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
	uint32_t stack_base;
	uint32_t lowest_committed_stack_address;

	_IMAGE_OPTIONAL_HEADER* optional_header = &(image->nt_headers->OptionalHeader);	

	if (taken_bases == 0){
		image->image_base = optional_header->ImageBase;
		taken_bases = (TAKEN_BASE*)malloc(sizeof(TAKEN_BASE));
		taken_bases->base = image->image_base;
		taken_bases->next = 0;
		reserve_address_space(cpu, image->image_base, 0x400);
	}
	else{
		image->image_base = find_free_base(optional_header->ImageBase);
		TAKEN_BASE* temp = taken_bases;
		taken_bases = (TAKEN_BASE*)malloc(sizeof(TAKEN_BASE));
		taken_bases->base = image->image_base;
		taken_bases->next = temp;
		reserve_address_space(cpu, image->image_base, 0x400);
	}

	image->entry_point = optional_header->AddressOfEntryPoint + image->image_base;
	image->stack_commit = optional_header->SizeOfStackCommit;
	image->stack_reserve = optional_header->SizeOfStackReserve;
	image->heap_commit = optional_header->SizeOfHeapCommit;
	image->heap_reserve = optional_header->SizeOfHeapReserve;
	
	printf("  Image Base: %p\n", image->image_base);
	printf("  Entry Point: %p\n", image->entry_point);
	printf("  Heap Size: %d bytes committed (%d reserved)\n", image->heap_commit, image->heap_reserve);
	printf("  Total Size: %d bytes (section alignment %d bytes, file alignment %d bytes)\n", optional_header->SizeOfImage, optional_header->SectionAlignment, optional_header->FileAlignment);

	//allocate a heap
	alloc_heap(cpu, image->heap_commit, image->heap_reserve);

	//allocate a stack if none exists
	stack_base = scan_free_address_space(cpu, image->stack_commit >> 12, 0x100);
	reserve_address_space(cpu, stack_base, image->stack_commit >> 12);
	lowest_committed_stack_address = stack_base + image->stack_reserve - image->stack_commit;

	printf("  Reserving %d bytes for stack from %p (%d committed from %p)\n", image->stack_reserve, stack_base, image->stack_commit, lowest_committed_stack_address);
	
	uint32_t stack_pde = GET_PDE(stack_base);

	if (cpu->page_dir.entries[stack_pde] == 0) {
		cpu->page_dir.entries[stack_pde] = (i386_PT*)malloc(sizeof(i386_PT));
		memset(cpu->page_dir.entries[stack_pde], 0, sizeof(i386_PT));
	}

	map_section(cpu, lowest_committed_stack_address, (uint8_t*)malloc(image->stack_commit), image->stack_commit);
	cpu->esp = stack_base + image->stack_reserve; //set to stack top
	cpu->stack_page_table = stack_pde;
	cpu->lowest_committed_page = GET_PTE(lowest_committed_stack_address);
}

void parse_id(LOADED_PE_IMAGE* image, IMAGE_IMPORT_DESCRIPTOR* import_desc, BYTE* address_base, i386* cpu);

//pass a 0 for ID and a 1 for name

IMAGE_RESOURCE_DIRECTORY_ENTRY* find_resource_entry(i386* cpu, uint32_t pRootDirectory, IMAGE_RESOURCE_DIRECTORY* rsDir, LPWSTR id, int name_or_id) {
	IMAGE_RESOURCE_DIRECTORY_ENTRY* dirEntry = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(rsDir + 1);
	LPWSTR name;

	for (int i = 0; i < rsDir->NumberOfNamedEntries; i++) {
		if (name_or_id && dirEntry->NameIsString) {
			name = (LPWSTR)virtual_to_physical_addr(cpu, pRootDirectory + dirEntry->NameOffset);
			if (wcscmp(name, id) == 0) return dirEntry;
		}

		dirEntry++;
	}

	for (int i = 0; i < rsDir->NumberOfIdEntries; i++) {
		if (!name_or_id && !dirEntry->NameIsString) {
			if ((WORD)id == dirEntry->Id) return dirEntry;
		}

		dirEntry++;
	}

	return 0;
}

uint32_t find_resource(i386* cpu, uint32_t pRootDirectory, LPWSTR resourceType, LPWSTR resourceName, int type_id, int name_id) {
	IMAGE_RESOURCE_DIRECTORY* imageResourceDirectory = (IMAGE_RESOURCE_DIRECTORY*)virtual_to_physical_addr(cpu, pRootDirectory);
	IMAGE_RESOURCE_DIRECTORY_ENTRY* dirEntry;
	IMAGE_RESOURCE_DIRECTORY* rsDir2;
	IMAGE_RESOURCE_DIRECTORY_ENTRY* dirEntry2;
	IMAGE_RESOURCE_DIRECTORY* rsDir3;
	IMAGE_RESOURCE_DIRECTORY_ENTRY* dirEntry3;

	//parse the top-level directory to find the directory with the list of resources of the current type
	dirEntry = find_resource_entry(cpu, pRootDirectory, imageResourceDirectory, resourceType, type_id);
	//parse the layer 1 directory to find the resource of the correct name/ID
	rsDir2 = (IMAGE_RESOURCE_DIRECTORY*)virtual_to_physical_addr(cpu, pRootDirectory + (dirEntry->OffsetToData & 0x7FFFFFFF));
	dirEntry2 = find_resource_entry(cpu, pRootDirectory, rsDir2, resourceName, name_id);
	rsDir3 = (IMAGE_RESOURCE_DIRECTORY*)virtual_to_physical_addr(cpu, pRootDirectory + (dirEntry2->OffsetToData & 0x7FFFFFFF));
	dirEntry3 = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(rsDir3 + 1);

	return pRootDirectory + (dirEntry3->OffsetToData & 0x7FFFFFFF);
}

HICON load_icon(i386* cpu, uint32_t pRootDirectory, LPWSTR string_name, int name_or_id) {
	return (HICON)0;
}

uint32_t find_string(i386* cpu, uint32_t string_base, uint32_t string_id) {
	uint32_t current_ptr = string_base;
	WORD current_length;

	for (int i = 0; i < string_id; i++) {
		current_length = *(WORD*)virtual_to_physical_addr(cpu, current_ptr);
		current_ptr += (current_length * 2) + 2;
	}

	return current_ptr;
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
	data_dir = &(optional_header->DataDirectory[1]);
	BYTE* address_base;
	IMAGE_IMPORT_DESCRIPTOR* import_desc;
	uint32_t section_virtual_addr;

	if (data_dir->Size != 0){
		section_virtual_addr = data_dir->VirtualAddress & 0xfffff000;
		printf("  Import Table Size: %d bytes (%p) Section %p\n", data_dir->Size, data_dir->VirtualAddress, section_virtual_addr);
		//import_desc = (IMAGE_IMPORT_DESCRIPTOR*)virtual_to_physical_addr(cpu, data_dir->VirtualAddress + image->image_base);
		/*address_base = (BYTE*)import_desc - section_virtual_addr;
		parse_id(image, import_desc, address_base, cpu);*/
	}	

	/*IMAGE_IMPORT_DIRECTORY* imageImportDirectory;
	data_dir = &(optional_header->DataDirectory[1]);*/
	
	//parse resource directory
	data_dir = &(optional_header->DataDirectory[2]);
	add_instance(image->image_base, data_dir->VirtualAddress);

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

void parse_id(LOADED_PE_IMAGE* image, IMAGE_IMPORT_DESCRIPTOR* import_desc, BYTE* address_base, i386* cpu){
	IMPORT_TABLE* temp;
	IMAGE_IMPORT_DESCRIPTOR null_desc;
	memset(&null_desc, 0, sizeof(null_desc));

	while (1){
		//check if it's zeroed out - if it is, quit the loop
		if (memcmp(import_desc, &null_desc, sizeof(IMAGE_IMPORT_DESCRIPTOR)) == 0){
			//printf("    Done parsing IDT\n");
			break;
		}

		printf("    %p: IAT RVA = %x, ILT RVA = %x ", import_desc, import_desc->OriginalFirstThunk, import_desc->FirstThunk);
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

void parse_idt(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	IMPORT_TABLE* temp;
	IMAGE_IMPORT_DESCRIPTOR* import_desc = (IMAGE_IMPORT_DESCRIPTOR*)(image->data + sec->PointerToRawData);
	IMAGE_IMPORT_DESCRIPTOR null_desc;
	BYTE* address_base = image->data + sec->PointerToRawData - sec->VirtualAddress;
	memset(&null_desc, 0, sizeof(null_desc));
	printf("%p\n", sec->PointerToRawData);

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
		//cpu->eip = sec->VirtualAddress + image->image_base;
	}
}

void parse_data(LOADED_PE_IMAGE* image, IMAGE_SECTION_HEADER* sec, i386* cpu){
	printf("    %d bytes initialized data, %d bytes uninitialized data\n", sec->SizeOfRawData, sec->Misc.VirtualSize - sec->SizeOfRawData);
}

void parse_section_headers(LOADED_PE_IMAGE* image, i386* cpu){
	uint8_t* lpSection;
	uint32_t sectionSize;

	IMAGE_SECTION_HEADER current_section; //reloc_pointer & num_relocs don't matter for EXEs
	image->section_headers = (IMAGE_SECTION_HEADER*)((BYTE*)&(image->nt_headers->OptionalHeader) + image->nt_headers->FileHeader.SizeOfOptionalHeader);

	for (int i = 0; i < image->nt_headers->FileHeader.NumberOfSections; i++){
		current_section = image->section_headers[i];
		printf("  Section %s\n", current_section.Name);
		printf("    Physical Address: %p Virtual Address %p Size: %d %d\n", current_section.PointerToRawData, current_section.VirtualAddress, current_section.SizeOfRawData, current_section.Misc.VirtualSize);

		if (current_section.Misc.VirtualSize > current_section.SizeOfRawData){
			sectionSize = ALIGN(current_section.Misc.VirtualSize, 4096);
		}
		else{
			sectionSize = ALIGN(current_section.SizeOfRawData, 4096);
		}
		lpSection = (uint8_t*)malloc(sectionSize);

		if (strcmp((const char*)current_section.Name, ".text") == 0){
			parse_text(image, &current_section, cpu);
		} else if(strcmp((const char*)current_section.Name, ".idata") == 0){
			parse_idt(image, &current_section, cpu);
		} else if (strcmp((const char*)current_section.Name, ".reloc") == 0){
			parse_rt(image, &current_section, cpu);
		} else if (strcmp((const char*)current_section.Name, ".data") == 0){
			parse_data(image, &current_section, cpu);
		}else if (strcmp((const char*)current_section.Name, ".rsrc") == 0){
			image->resource_offset = current_section.VirtualAddress;
		}
		else if (strcmp((const char*)current_section.Name, ".bss") == 0){
			current_section.SizeOfRawData = 0;
		}

		memset(lpSection, 0, sectionSize); //zero it out
		memcpy(lpSection, image->data + current_section.PointerToRawData, current_section.SizeOfRawData);
		map_section(cpu, current_section.VirtualAddress + image->image_base, lpSection, sectionSize);
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
	uint32_t addr;

	while (export_table){
		if (strcmp(name, (const char*)virtual_to_physical_addr(cpu, export_table->name)) == 0){
			return export_table->address;
		}

		export_table = export_table->next;
	}

	return 0;
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

			if (addr){
				printf(" (resolved to address %p)\n", addr);
			}
			else{
				printf(" (UNRESOLVED EXPORT)\n");
			}

			*(uint32_t*)(virtual_to_physical_addr(cpu, image->image_base + function->address)) = addr;

			function = function->next;
		}

		table = table->next;
	}
}

uint32_t debug_step(i386* cpu){
#ifndef NO_DEBUGGER
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

		gets(buf);
		sscanf(buf, "%s %x %x", option, &value, &value_2);

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
		else if (strcmp(option, "pt") == 0) {
			if (cpu->page_dir.entries[value]) {
				printf("Page %p:%p (VMA %Xxxx) is mapped to address %p\n", value, value_2, (value << 10) + (value_2), cpu->page_dir.entries[value]->entries[value_2]);
			} else {
				printf("Page table %p is not mapped into virtual memory.\n", value);
			}
		}
		else if (strcmp(option, "pd") == 0) {
			printf("The page table %p (VMA %p - %p) is located at host memory address %p\n", value, value << 22, ((value + 1) << 22) - 1, cpu->page_dir.entries[value]);
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
			printf("pd hex:addr - Lists the address of page directory entry addr\n");
			printf("pt hex:addr1 hex:addr2 - Lists the address of page table entry addr2 inside page table addr1\n");
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

	/*if (cpu->running && cpu->single_step == 0 && kbhit()) {
		getch();
		printf("Break.\n");
		cpu->running = 0;
		cpu->single_step = 1;
	}*/

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
#else
	if (cpu->running) {
		cpu_step(cpu);
	}
	else {
		if (cpu->escaping) {
			cpu->escaping = 0;
			cpu->running = 1;
			return 1;
		}
	}
	return 0;
#endif
}

uint8_t escape_routine[3] = {0xcd, 0xff, 0xc3};

LOADED_PE_IMAGE load_pe_executable(char* filename, i386* cpu){
	reserve_address_space(cpu, 0x00000000, 0x400);
	LOADED_PE_IMAGE image = load_pe_file(filename);
	parse_headers(&image, cpu);
	cpu->running = 0;
	cpu->single_step = 1;
	cpu->eip = image.entry_point;

	//map the escape routine into ram
	escape_addr = 0xFFFF0000;
	global_cpu = cpu;
	virtual_mmap(cpu, escape_addr, escape_routine);

	//allocate the heap
	heap_init();
	alloc_heap(cpu, image.heap_commit, image.heap_reserve); //put it in the PEB

#ifdef NO_DEBUGGER
	cpu->running = 1;
	cpu->single_step = 0;
#endif

	return image;
}

int main(int argc, char* argv[])
{
	IMAGE_RESOURCE_DATA_ENTRY* data_entry;
	uint32_t string_table;
	FILE* fp;

	int single_step = 1;
	char option[100];
	char reg[25];
	uint32_t value;

	char buf[100];

	i386 CPU;
	cpu_init(&CPU);

	LOADED_PE_IMAGE hello = load_pe_executable(argv[1], &CPU);
	log_instructions = 0;

	memset(registered_window_classes, 0, sizeof(registered_window_classes));
	add_window(0, 0);

	while (1) {
		debug_step(&CPU);
	}

	puts("We're done here!\n");
}