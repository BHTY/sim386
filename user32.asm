global MessageBoxA
extern MessageBoxA
export MessageBoxA

global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret

MessageBoxA:
	mov eax, THUNK_USER32_MESSAGEBOX
	int SYSCALL_THUNK
	ret

THUNK_USER32_MESSAGEBOX equ 0x00
SYSCALL_THUNK equ 0x80