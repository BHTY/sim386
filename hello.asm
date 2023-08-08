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

	cmp eax, 1
	je print_ok
	push 0
	push title_msg
	push cancel_msg
	push 0
	call _MessageBoxA@16
	jmp done
print_ok:
	push 0
	push title_msg
	push ok_msg
	push 0
	call _MessageBoxA@16
done:
	; ExitProcess(0)
	push 0
	call _ExitProcess@4
	hlt

title_msg: db '80386 emulator', 0
message: db 'Hello World from sim386!', 0
ok_msg: db 'You pressed OK!', 0
cancel_msg: db 'You pressed cancel!', 0