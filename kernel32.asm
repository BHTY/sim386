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
	ret 0x4

global HeapAlloc
extern HeapAlloc
export HeapAlloc

HeapAlloc:
	mov eax, THUNK_KERNEL32_HEAPALLOC
	int SYSCALL_THUNK
	ret 0xc

global GetVersion
extern GetVersion
export GetVersion

GetVersion:
	mov eax, 0x23f00206
	ret

global HeapCreate
extern HeapCreate
export HeapCreate

HeapCreate:
	mov eax, THUNK_KERNEL32_HEAPCREATE
	int SYSCALL_THUNK
	ret 0xc

global GetStdHandle
extern GetStdHandle
export GetStdHandle

GetStdHandle:
	mov eax, THUNK_KERNEL32_GETSTDHANDLE
	int SYSCALL_THUNK
	ret 0x4
	
global GetFileType
extern GetFileType
export GetFileType

GetFileType:
	mov eax, THUNK_KERNEL32_GETFILETYPE
	int SYSCALL_THUNK
	ret 0x4

global SetHandleCount
extern SetHandleCount
export SetHandleCount

SetHandleCount:
	mov eax, THUNK_KERNEL32_SETHANDLECOUNT
	int SYSCALL_THUNK
	ret 0x4
	
global GetACP
extern GetACP
export GetACP

GetACP:
	mov eax, THUNK_KERNEL32_GETACP
	int SYSCALL_THUNK
	ret 0x0

global GetCPInfo
extern GetCPInfo
export GetCPInfo

GetCPInfo:
	mov eax, THUNK_KERNEL32_GETCPINFO
	int SYSCALL_THUNK
	ret 0x8

global GetEnvironmentStringsW
extern GetEnvironmentStringsW
export GetEnvironmentStringsW

GetEnvironmentStringsW:
	mov eax, THUNK_KERNEL32_GETENVIRONMENTSTRINGSW
	int SYSCALL_THUNK
	ret 0x0
	
global FreeEnvironmentStringsW
extern FreeEnvironmentStringsW
export FreeEnvironmentStringsW

FreeEnvironmentStringsW:
	mov eax, THUNK_KERNEL32_FREEENVIRONMENTSTRINGSW
	int SYSCALL_THUNK
	ret 0x4
	
global WideCharToMultiByte
extern WideCharToMultiByte
export WideCharToMultiByte

WideCharToMultiByte:
	mov eax, THUNK_KERNEL32_WIDECHARTOMULTIBYTE
	int SYSCALL_THUNK
	ret 0x20
	
global GetModuleFileNameA
extern GetModuleFileNameA
export GetModuleFileNameA

GetModuleFileNameA:
	mov eax, THUNK_KERNEL32_GETMODULEFILENAMEA
	int SYSCALL_THUNK
	ret 0xc

global WriteFile
extern WriteFile
export WriteFile

WriteFile:
	mov eax, THUNK_KERNEL32_WRITEFILE
	int SYSCALL_THUNK
	ret 0x14

global GetProcessHeap
extern GetProcessHeap
export GetProcessHeap

GetProcessHeap:
	mov eax, 1
	ret

global HeapFree
extern HeapFree
export HeapFree

HeapFree:
	mov eax, THUNK_KERNEL32_HEAPFREE
	int SYSCALL_THUNK
	ret 0xc

global LocalAlloc
extern LocalAlloc
export LocalAlloc

LocalAlloc:
	push ebp
	mov ebp, esp
	mov eax, [ebp+0xc]
	push eax
	mov eax, [ebp+0x8]
	push eax
	call GetProcessHeap
	push eax
	call HeapAlloc
	leave
	ret 0x8

global LocalFree
extern LocalFree
export LocalFree

LocalFree:
	push ebp
	mov ebp, esp	
	mov eax, [ebp+0x8]
	push eax
	push 0
	call GetProcessHeap
	push eax
	call HeapFree
	leave
	ret 0x4

global GetEnvironmentStringsA
extern GetEnvironmentStringsA
export GetEnvironmentStringsA

GetEnvironmentStringsA:
	hlt
	mov eax, THUNK_KERNEL32_GETENVIRONMENTSTRINGSA
	int SYSCALL_THUNK
	ret 0x0

global GetEnvironmentStrings
extern GetEnvironmentStrings
export GetEnvironmentStrings

GetEnvironmentStrings:
	jmp GetEnvironmentStringsA

global ReadFile
extern ReadFile
export ReadFile

ReadFile:
	mov eax, THUNK_KERNEL32_READFILE
	int SYSCALL_THUNK
	ret 0x14
	
global GetLocalTime
extern GetLocalTime
export GetLocalTime

GetLocalTime:
	mov eax, THUNK_KERNEL32_GETLOCALTIME
	int SYSCALL_THUNK
	ret 0x4

global GetSystemTime
extern GetSystemTime
export GetSystemTime

GetSystemTime:
	mov eax, THUNK_KERNEL32_GETSYSTEMTIME
	int SYSCALL_THUNK
	ret 0x4

global GetTimeZoneInformation
extern GetTimeZoneInformation
export GetTimeZoneInformation

GetTimeZoneInformation:
	mov eax, THUNK_KERNEL32_GETTIMEZONEINFORMATION
	int SYSCALL_THUNK
	ret 0x4

THUNK_KERNEL32_EXITPROCESS equ 0x01
THUNK_KERNEL32_GETMODULEHANDLEA equ 0x03
THUNK_KERNEL32_GETCOMMANDLINEA equ 0x0E
THUNK_KERNEL32_GETSTARTUPINFOA equ 0x0F
THUNK_KERNEL32_HEAPALLOC equ 0x1E
THUNK_KERNEL32_HEAPCREATE equ 0x1F
THUNK_KERNEL32_GETSTDHANDLE equ 0x20
THUNK_KERNEL32_GETFILETYPE equ 0x21
THUNK_KERNEL32_SETHANDLECOUNT equ 0x22
THUNK_KERNEL32_GETACP equ 0x23
THUNK_KERNEL32_GETCPINFO equ 0x24
THUNK_KERNEL32_GETENVIRONMENTSTRINGSW equ 0x25
THUNK_KERNEL32_FREEENVIRONMENTSTRINGSW equ 0x26
THUNK_KERNEL32_WIDECHARTOMULTIBYTE equ 0x27
THUNK_KERNEL32_GETMODULEFILENAMEA equ 0x28
THUNK_KERNEL32_WRITEFILE equ 0x29
THUNK_KERNEL32_HEAPFREE equ 0x2A
THUNK_KERNEL32_GETENVIRONMENTSTRINGSA equ 0x4D
THUNK_KERNEL32_READFILE equ 0x56
THUNK_KERNEL32_GETLOCALTIME equ 0x59
THUNK_KERNEL32_GETSYSTEMTIME equ 0x5A
THUNK_KERNEL32_GETTIMEZONEINFORMATION equ 0x5B
SYSCALL_THUNK equ 0x80