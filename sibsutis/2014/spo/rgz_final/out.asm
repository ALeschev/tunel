%include "asm_io.inc"

EXTERN printf
EXTERN scanf

section .data
	x dd 0
	y dd 0
	z dd 0
format_p db '%d', 0ah, 0
format_s db '%d'

section .text
global asm_main

asm_main:

	push dword x
	mov eax, y
	push eax
	mov eax, format_s
	push eax
	call scanf

	push dword z
	push dword 6
	push dword [y]

	pop eax
	pop ebx

	add eax, ebx
	push eax


	pop eax
	pop ebx

	mov [ebx], eax

	push dword x
	push dword [y]

	pop eax
	pop ebx

	cmp ebx, eax
	test ebx,ebx
	jne metka_0
	mov eax, [x]
	push eax
	mov eax, format_p
	push eax
	call printf

	metka_0:
	mov eax, [y]
	push eax
	mov eax, format_p
	push eax
	call printf

	metka_1:
	mov eax, [z]
	push eax
	mov eax, format_p
	push eax
	call printf

	mov ebx, 0
	mov eax, 1
	int 0x80

