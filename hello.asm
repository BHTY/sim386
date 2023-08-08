global _main
extern _MessageBoxA@16
extern _ExitProcess@4

section .text

_main:
	mov ebp, esp
	sub esp, 4

	; MessageBox()
	push 1 ;MB_OKCANCEL
	push title_msg ;lpCaption
	push message ;lpText
	push 0 ;NULL HWND
	call _MessageBoxA@16

	; ExitProcess(0)
	push 0
	call _ExitProcess@4
	hlt

title_msg: db '80386 emulator', 10, 0
message: db 'Hello World from sim386!', 10, 0