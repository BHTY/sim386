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

global GetSystemMetrics
extern GetSystemMetrics
export GetSystemMetrics

GetSystemMetrics:
	mov eax, THUNK_USER32_GETSYSTEMMETRICS
	int SYSCALL_THUNK
	ret 0x04

global TranslateAcceleratorA
extern TranslateAcceleratorA
export TranslateAcceleratorA

TranslateAcceleratorA:
	mov eax, THUNK_USER32_GETSYSTEMMETRICS
	int SYSCALL_THUNK
	ret 0x0C

global GetDC
extern GetDC
export GetDC

GetDC:
	mov eax, THUNK_USER32_GETDC
	int SYSCALL_THUNK
	ret 0x04

global ReleaseDC
extern ReleaseDC
export ReleaseDC

ReleaseDC:
	mov eax, THUNK_USER32_RELEASEDC
	int SYSCALL_THUNK
	ret 0x08
	
global InvalidateRect
extern InvalidateRect
export InvalidateRect

InvalidateRect:
	mov eax, THUNK_USER32_INVALIDATERECT
	int SYSCALL_THUNK
	ret 0x0C
	
global LoadAcceleratorsA
extern LoadAcceleratorsA
export LoadAcceleratorsA

LoadAcceleratorsA:
	mov eax, 0
	ret 0x8
	
global LoadStringA
extern LoadStringA
export LoadStringA

LoadStringA:
	mov eax, THUNK_USER32_LOADSTRINGA
	int SYSCALL_THUNK
	ret 0x10
	
global SetWindowTextA
extern SetWindowTextA
export SetWindowTextA

SetWindowTextA:
	mov eax, THUNK_USER32_SETWINDOWTEXTA
	int SYSCALL_THUNK
	ret 0x8
	
global GetMenu
extern GetMenu
export GetMenu

GetMenu:
	mov eax, THUNK_USER32_GETMENU
	int SYSCALL_THUNK
	ret 0x4

global CheckMenuItem
extern CheckMenuItem
export CheckMenuItem

CheckMenuItem:
	mov eax, THUNK_USER32_CHECKMENUITEM
	int SYSCALL_THUNK
	ret 0xc
	
global SetCursorPos
extern SetCursorPos
export SetCursorPos

SetCursorPos:
	mov eax, THUNK_USER32_SETCURSORPOS
	int SYSCALL_THUNK
	ret 0x8
	
global ClientToScreen
extern ClientToScreen
export ClientToScreen

ClientToScreen:
	mov eax, THUNK_USER32_CLIENTTOSCREEN
	int SYSCALL_THUNK
	ret 0x8

global SetCapture
extern SetCapture
export SetCapture

SetCapture:
	mov eax, THUNK_USER32_SETCAPTURE
	int SYSCALL_THUNK
	ret 0x4

global ReleaseCapture
extern ReleaseCapture
export ReleaseCapture

ReleaseCapture:
	mov eax, THUNK_USER32_RELEASECAPTURE
	int SYSCALL_THUNK
	ret 0x0

global ShowCursor
extern ShowCursor
export ShowCursor

ShowCursor:
	mov eax, THUNK_USER32_SHOWCURSOR
	int SYSCALL_THUNK
	ret 0x4

global SetCursor
extern SetCursor
export SetCursor

SetCursor:
	mov eax, THUNK_USER32_SETCURSOR
	int SYSCALL_THUNK
	ret 0x4

global SetFocus
extern SetFocus
export SetFocus

SetFocus:
	mov eax, THUNK_USER32_SETFOCUS
	int SYSCALL_THUNK
	ret 0x4

global PostMessageA
extern PostMessageA
export PostMessageA

PostMessageA:
	mov eax, THUNK_USER32_POSTMESSAGEA
	int SYSCALL_THUNK
	ret 0x10

global EnableMenuItem
extern EnableMenuItem
export EnableMenuItem

EnableMenuItem:
	mov eax, THUNK_USER32_ENABLEMENUITEM
	int SYSCALL_THUNK
	ret 0xc

global IsIconic
extern IsIconic
export IsIconic

IsIconic:
	mov eax, THUNK_USER32_ISICONIC
	int SYSCALL_THUNK
	ret 0x4

global GetKeyState
extern GetKeyState
export GetKeyState

GetKeyState:
	mov eax, THUNK_USER32_GETKEYSTATE
	int SYSCALL_THUNK
	ret 0x4

global SetTimer
extern SetTimer
export SetTimer

SetTimer:
	mov eax, THUNK_USER32_SETTIMER
	int SYSCALL_THUNK
	ret 0x10

global KillTimer
extern KillTimer
export KillTimer

KillTimer:
	mov eax, THUNK_USER32_KILLTIMER
	int SYSCALL_THUNK
	ret 0x8
	
global GetWindowDC
extern GetWindowDC
export GetWindowDC

GetWindowDC:
	mov eax, THUNK_USER32_GETWINDOWDC
	int SYSCALL_THUNK
	ret 0x4

global SendMessageA
extern SendMessageA
export SendMessageA

SendMessageA:
	mov eax, THUNK_USER32_SENDMESSAGEA
	int SYSCALL_THUNK
	ret 0x10

global GetDlgItem
extern GetDlgItem
export GetDlgItem

GetDlgItem:
	mov eax, THUNK_USER32_GETDLGITEM
	int SYSCALL_THUNK
	ret 0x8

global GetDlgItemInt
extern GetDlgItemInt
export GetDlgItemInt

GetDlgItemInt:
	mov eax, THUNK_USER32_GETDLGITEMINT
	int SYSCALL_THUNK
	ret 0x10

global DrawMenuBar
extern DrawMenuBar
export DrawMenuBar

DrawMenuBar:
	mov eax, THUNK_USER32_DRAWMENUBAR
	int SYSCALL_THUNK
	ret 0x4

global DialogBoxParamA
extern DialogBoxParamA
export DialogBoxParamA

DialogBoxParamA:
	mov eax, THUNK_USER32_DIALOGBOXPARAMA
	int SYSCALL_THUNK
	ret 0x14

global EndDialog
extern EndDialog
export EndDialog

EndDialog:
	mov eax, THUNK_USER32_ENDDIALOG
	int SYSCALL_THUNK
	ret 0x8

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
THUNK_USER32_GETSYSTEMMETRICS equ 0x18
THUNK_USER32_TRANSLATEACCELERATORA equ 0x19
THUNK_USER32_GETDC equ 0x1A
THUNK_USER32_RELEASEDC equ 0x1B
THUNK_USER32_INVALIDATERECT equ 0x1D
THUNK_USER32_LOADSTRINGA equ 0x2D
THUNK_USER32_SETWINDOWTEXTA equ 0x31
THUNK_USER32_GETMENU equ 0x32
THUNK_USER32_CHECKMENUITEM equ 0x33
THUNK_USER32_SETCURSORPOS equ 0x34
THUNK_USER32_CLIENTTOSCREEN equ 0x35
THUNK_USER32_SETCAPTURE equ 0x3B
THUNK_USER32_RELEASECAPTURE equ 0x3C
THUNK_USER32_SHOWCURSOR equ 0x3D
THUNK_USER32_SETCURSOR equ 0x3E
THUNK_USER32_SETFOCUS equ 0x3F
THUNK_USER32_POSTMESSAGEA equ 0x40
THUNK_USER32_ENABLEMENUITEM equ 0x41
THUNK_USER32_ISICONIC equ 0x42
THUNK_USER32_GETKEYSTATE equ 0x43
THUNK_USER32_SETTIMER equ 0x44
THUNK_USER32_KILLTIMER equ 0x45
THUNK_USER32_GETWINDOWDC equ 0x48
THUNK_USER32_SENDMESSAGEA equ 0x49
THUNK_USER32_GETDLGITEM equ 0x4A
THUNK_USER32_GETDLGITEMINT equ 0x4B
THUNK_USER32_DRAWMENUBAR equ 0x4C
THUNK_USER32_DIALOGBOXPARAMA equ 0x57
THUNK_USER32_ENDDIALOG equ 0x58
SYSCALL_THUNK equ 0x80