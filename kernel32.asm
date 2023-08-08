global ExitProcess
extern ExitProcess
export ExitProcess

global GetModuleHandleA
extern GetModuleHandleA
export GetModuleHandleA

global GetCommandLineA
extern GetCommandLineA
export GetCommandLineA

global GetStartupInfoA
extern GetStartupInfoA
export GetStartupInfoA

global __DllMainCRTStartup@12
extern __DllMainCRTStartup@12
export __DllMainCRTStartup@12

section .text

__DllMainCRTStartup@12:
	ret

ExitProcess:
	mov eax, THUNK_KERNEL32_EXITPROCESS
	int SYSCALL_THUNK
	ret 0x04

GetModuleHandleA:
	mov eax, THUNK_KERNEL32_GETMODULEHANDLEA
	int SYSCALL_THUNK
	ret 0x04

GetCommandLineA:
	mov eax, THUNK_KERNEL32_GETCOMMANDLINEA
	int SYSCALL_THUNK
	ret

GetStartupInfoA:
	mov eax, THUNK_KERNEL32_GETSTARTUPINFOA
	int SYSCALL_THUNK
	ret

THUNK_KERNEL32_EXITPROCESS equ 0x01
THUNK_KERNEL32_GETMODULEHANDLEA equ 0x03
THUNK_KERNEL32_GETCOMMANDLINEA equ 0x0E
THUNK_KERNEL32_GETSTARTUPINFOA equ 0x0F
SYSCALL_THUNK equ 0x80