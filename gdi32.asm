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

THUNK_GDI32_SETBKMODE equ 0x02
THUNK_GDI32_GETSTOCKOBJECT equ 0x12
THUNK_GDI32_SETPIXEL equ 0x1C
SYSCALL_THUNK equ 0x80