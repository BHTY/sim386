global SetBkMode
extern SetBkMode
export SetBkMode

global GetStockObject
extern GetStockObject
export GetStockObject

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

THUNK_GDI32_SETBKMODE equ 0x02
THUNK_GDI32_GETSTOCKOBJECT equ 0x12
SYSCALL_THUNK equ 0x80