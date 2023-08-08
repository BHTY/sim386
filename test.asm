global _main
extern _MessageBoxA@16
extern _ExitProcess@4
extern _init
extern _handle_messages

section .text

_main:
	push name
	push 200
	push 320
	call _init

	mov ebx, eax

	push 0
	push name
	push name
	push eax
	call _MessageBoxA@16

	mov eax, ebx

loop:	push eax
	call _handle_messages
	add esp, 4
	jmp loop

	push 0
	call _ExitProcess@4
	hlt


name: db 'Test', 0