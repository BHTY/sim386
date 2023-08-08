global MessageBoxA
extern MessageBoxA
export MessageBoxA

global LoadIconA
extern LoadIconA
export LoadIconA

global LoadCursorA
extern LoadCursorA
export LoadCursorA

global RegisterClassA
extern RegisterClassA
export RegisterClassA

global CreateWindowExA
extern CreateWindowExA
export CreateWindowExA

global ShowWindow
extern ShowWindow
export ShowWindow

global UpdateWindow
extern UpdateWindow
export UpdateWindow

global DefWindowProcA
extern DefWindowProcA
export DefWindowProcA

global PeekMessageA
extern PeekMessageA
export PeekMessageA

global TranslateMessage
extern TranslateMessage
export TranslateMessage

global DispatchMessageA
extern DispatchMessageA
export DispatchMessageA

global RegisterClassExA
extern RegisterClassExA
export RegisterClassExA

global GetMessageA
extern GetMessageA
export GetMessageA

global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret

MessageBoxA:
	mov eax, THUNK_USER32_MESSAGEBOXA
	int SYSCALL_THUNK
	ret 0x10

LoadCursorA:
	mov eax, THUNK_USER32_LOADCURSORA
	int SYSCALL_THUNK
	ret 0x08

LoadIconA:
	mov eax, THUNK_USER32_LOADICONA
	int SYSCALL_THUNK
	ret 0x08

RegisterClassA:
	mov eax, THUNK_USER32_REGISTERCLASSA
	int SYSCALL_THUNK
	ret 0x04

CreateWindowExA:
	mov eax, THUNK_USER32_CREATEWINDOWEXA
	int SYSCALL_THUNK
	ret 0x30

ShowWindow:
	mov eax, THUNK_USER32_SHOWWINDOW
	int SYSCALL_THUNK
	ret 0x08

UpdateWindow:
	mov eax, THUNK_USER32_UPDATEWINDOW
	int SYSCALL_THUNK
	ret 0x04

DefWindowProcA:
	mov eax, THUNK_USER32_DEFWINDOWPROCA
	int SYSCALL_THUNK
	ret

PeekMessageA:
	mov eax, THUNK_USER32_PEEKMESSAGEA
	int SYSCALL_THUNK
	ret 0x10

TranslateMessage:
	mov eax, THUNK_USER32_TRANSLATEMESSAGE
	int SYSCALL_THUNK
	ret 0x04

DispatchMessageA:
	mov eax, THUNK_USER32_DISPATCHMESSAGEA
	int SYSCALL_THUNK
	ret 0x04

RegisterClassExA:
	mov eax, THUNK_USER32_REGISTERCLASSEXA
	int SYSCALL_THUNK
	ret 0x04

GetMessageA:
	mov eax, THUNK_USER32_GETMESSAGEA
	int SYSCALL_THUNK
	ret 0x10

global PostQuitMessage
extern PostQuitMessage
export PostQuitMessage

PostQuitMessage:
	mov eax, THUNK_USER32_POSTQUITMESSAGE
	int SYSCALL_THUNK
	ret 0x04

global EndPaint
extern EndPaint
export EndPaint

EndPaint:
	mov eax, THUNK_USER32_ENDPAINT
	int SYSCALL_THUNK
	ret 0x08

global DrawTextA
extern DrawTextA
export DrawTextA

DrawTextA:
	mov eax, THUNK_USER32_DRAWTEXTA
	int SYSCALL_THUNK
	ret 0x14

global GetClientRect
extern GetClientRect
export GetClientRect

GetClientRect:
	mov eax, THUNK_USER32_GETCLIENTRECT
	int SYSCALL_THUNK
	ret 0x08

global BeginPaint
extern BeginPaint
export BeginPaint

BeginPaint:
	mov eax, THUNK_USER32_BEGINPAINT
	int SYSCALL_THUNK
	ret 0x08


THUNK_USER32_MESSAGEBOXA equ 0x00
THUNK_USER32_LOADCURSORA equ 0x04
THUNK_USER32_LOADICONA equ 0x05
THUNK_USER32_REGISTERCLASSA equ 0x06
THUNK_USER32_CREATEWINDOWEXA equ 0x07
THUNK_USER32_SHOWWINDOW equ 0x08
THUNK_USER32_UPDATEWINDOW equ 0x09
THUNK_USER32_DEFWINDOWPROCA equ 0x0A
THUNK_USER32_PEEKMESSAGEA equ 0x0B
THUNK_USER32_TRANSLATEMESSAGE equ 0x0C
THUNK_USER32_DISPATCHMESSAGEA equ 0x0D
THUNK_USER32_REGISTERCLASSEXA equ 0x10
THUNK_USER32_GETMESSAGEA equ 0x11
THUNK_USER32_POSTQUITMESSAGE equ 0x13
THUNK_USER32_ENDPAINT equ 0x14
THUNK_USER32_DRAWTEXTA equ 0x15
THUNK_USER32_GETCLIENTRECT equ 0x16
THUNK_USER32_BEGINPAINT equ 0x17
SYSCALL_THUNK equ 0x80