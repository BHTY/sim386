global SetBkMode
extern SetBkMode
export SetBkMode

global GetStockObject
extern GetStockObject
export GetStockObject

global SetPixel
extern SetPixel
export SetPixel

global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret

SetBkMode:
	mov eax, THUNK_GDI32_SETBKMODE
	int SYSCALL_THUNK
	ret 0x8

GetStockObject:
	mov eax, THUNK_GDI32_GETSTOCKOBJECT
	int SYSCALL_THUNK
	ret 0x4

SetPixel:
	mov eax, THUNK_GDI32_SETPIXEL
	int SYSCALL_THUNK
	ret 0x10

global GetDeviceCaps
extern GetDeviceCaps
export GetDeviceCaps

GetDeviceCaps:
	mov eax, THUNK_GDI32_GETDEVICECAPS
	int SYSCALL_THUNK
	ret 0x8

global CreateSolidBrush
extern CreateSolidBrush
export CreateSolidBrush

CreateSolidBrush:
	mov eax, THUNK_GDI32_CREATESOLIDBRUSH
	int SYSCALL_THUNK
	ret 0x4

global GetTextMetricsA
extern GetTextMetricsA
export GetTextMetricsA

GetTextMetricsA:
	mov eax, THUNK_GDI32_GETTEXTMETRICSA
	int SYSCALL_THUNK
	ret 0x8

global SelectObject
extern SelectObject
export SelectObject

SelectObject:
	mov eax, THUNK_GDI32_SELECTOBJECT
	int SYSCALL_THUNK
	ret 0x8

global PatBlt
extern PatBlt
export PatBlt

PatBlt:
	mov eax, THUNK_GDI32_PATBLT
	int SYSCALL_THUNK
	ret 0x18

global LineTo
extern LineTo
export LineTo

LineTo:
	mov eax, THUNK_GDI32_LINETO
	int SYSCALL_THUNK
	ret 0xc
	
global MoveToEx
extern MoveToEx
export MoveToEx

MoveToEx:
	mov eax, THUNK_GDI32_MOVETOEX
	int SYSCALL_THUNK
	ret 0x10
	
global TextOutA
extern TextOutA
export TextOutA

TextOutA:
	mov eax, THUNK_GDI32_TEXTOUTA
	int SYSCALL_THUNK
	ret 0x14
	
global GetTextExtentPointA
extern GetTextExtentPointA
export GetTextExtentPointA

GetTextExtentPointA:
	mov eax, THUNK_GDI32_GETTEXTEXTENTPOINTA
	int SYSCALL_THUNK
	ret 0x10

global DeleteObject
extern DeleteObject
export DeleteObject

DeleteObject:
	mov eax, THUNK_GDI32_DELETEOBJECT
	int SYSCALL_THUNK
	ret 0x4

global Ellipse
extern Ellipse
export Ellipse

Ellipse:
	mov eax, THUNK_GDI32_ELLIPSE
	int SYSCALL_THUNK
	ret 0x14

global CreateICA
extern CreateICA
export CreateICA

CreateICA:
	mov eax, THUNK_GDI32_CREATEICA
	int SYSCALL_THUNK
	ret 0x10

global StretchBlt
extern StretchBlt
export StretchBlt

StretchBlt:
	mov eax, THUNK_GDI32_STRETCHBLT
	int SYSCALL_THUNK
	ret 0x2c

global GetPixel
extern GetPixel
export GetPixel

GetPixel:
	mov eax, THUNK_GDI32_GETPIXEL
	int SYSCALL_THUNK
	ret 0xc

global BitBlt
extern BitBlt
export BitBlt

BitBlt:
	mov eax, THUNK_GDI32_BITBLT
	int SYSCALL_THUNK
	ret 0x24

global CreateCompatibleDC
extern CreateCompatibleDC
export CreateCompatibleDC

CreateCompatibleDC:
	mov eax, THUNK_GDI32_CREATECOMPATIBLEDC
	int SYSCALL_THUNK
	ret 0x4

global CreateCompatibleBitmap
extern CreateCompatibleBitmap
export CreateCompatibleBitmap

CreateCompatibleBitmap:
	mov eax, THUNK_GDI32_CREATECOMPATIBLEBITMAP
	int SYSCALL_THUNK
	ret 0xc

global CreatePen
extern CreatePen
export CreatePen

CreatePen:
	mov eax, THUNK_GDI32_CREATEPEN
	int SYSCALL_THUNK
	ret 0xc

global DeleteDC
extern DeleteDC
export DeleteDC

DeleteDC:
	mov eax, THUNK_GDI32_DELETEDC
	int SYSCALL_THUNK
	ret 0x4

THUNK_GDI32_SETBKMODE equ 0x02
THUNK_GDI32_GETSTOCKOBJECT equ 0x12
THUNK_GDI32_SETPIXEL equ 0x1C
THUNK_GDI32_GETDEVICECAPS equ 0x2B
THUNK_GDI32_CREATESOLIDBRUSH equ 0x2C
THUNK_GDI32_GETTEXTMETRICSA equ 0x2E
THUNK_GDI32_SELECTOBJECT equ 0x2F
THUNK_GDI32_PATBLT equ 0x30
THUNK_GDI32_LINETO equ 0x36
THUNK_GDI32_MOVETOEX equ 0x37
THUNK_GDI32_TEXTOUTA equ 0x38
THUNK_GDI32_GETTEXTEXTENTPOINTA equ 0x39
THUNK_GDI32_DELETEOBJECT equ 0x3A
THUNK_GDI32_ELLIPSE equ 0x46
THUNK_GDI32_CREATEICA equ 0x4E
THUNK_GDI32_STRETCHBLT equ 0x4F
THUNK_GDI32_GETPIXEL equ 0x50
THUNK_GDI32_BITBLT equ 0x51
THUNK_GDI32_CREATECOMPATIBLEDC equ 0x52
THUNK_GDI32_CREATECOMPATIBLEBITMAP equ 0x53
THUNK_GDI32_CREATEPEN equ 0x54
THUNK_GDI32_DELETEDC equ 0x55
SYSCALL_THUNK equ 0x80