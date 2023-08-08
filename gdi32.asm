global SetBkMode
extern SetBkMode
export SetBkMode

global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret

SetBkMode:
	mov eax, THUNK_GDI32_SETBKMODE
	int SYSCALL_THUNK
	ret

THUNK_GDI32_SETBKMODE equ 0x02
SYSCALL_THUNK equ 0x80