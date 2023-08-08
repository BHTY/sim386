global ExitProcess
extern ExitProcess
export ExitProcess

global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret

ExitProcess:
	mov eax, THUNK_KERNEL32_EXITPROCESS
	int SYSCALL_THUNK
	ret

THUNK_KERNEL32_EXITPROCESS equ 0x01
SYSCALL_THUNK equ 0x80