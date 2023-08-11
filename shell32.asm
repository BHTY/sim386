global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret
	
global ShellAboutA
extern ShellAboutA
export ShellAboutA

ShellAboutA:
	mov eax, THUNK_SHELL32_SHELLABOUTA
	int SYSCALL_THUNK
	ret 0x10
	
THUNK_SHELL32_SHELLABOUTA equ 0x47
SYSCALL_THUNK equ 0x80