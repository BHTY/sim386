#define WIN32_LEAN_AND_MEAN

#include <windows.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	HDC hdc;
	int i;
	int p;
	
	switch(msg){
		case WM_KEYDOWN:
			MessageBoxA(hWnd, "Yo", "yo", MB_OK);
			return 0;
		case WM_PAINT:
			hdc = GetDC(hWnd);
			
			for(i = 0; i < 220; i++){
				for(p = 0; p < 320; p++){
					SetPixel(hdc, p, i, 0x0);
				}
			}
			
			ReleaseDC(hWnd, hdc);
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

__declspec(dllexport) HWND init(int x, int y, char* name){
	WNDCLASS wc;
	HWND hwnd;

	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = name;
	RegisterClass(&wc);

	hwnd = CreateWindow(name, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, x, y, NULL, NULL, GetModuleHandle(NULL), NULL);
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	InvalidateRect(hwnd, NULL, 0);
	return hwnd;
}

__declspec(dllexport) void handle_messages(HWND hwnd){
	MSG msg;
	GetMessage(&msg, hwnd, 0, 0);
	TranslateMessage(&msg);
	DispatchMessage(&msg);
}