global main
extern printf
extern scanf
extern exit

SECTION .bss
	_buf resb 10

SECTION .data
	ifmt db "%d", 0
	sfmt db "%d", 0
	dbl: dq 1.123

SECTION .text
	main:
	;Присваивание
	push dword 5
	pop dword [h]

	;Присваивание
	push dword 1
	pop dword [r]

	;Присваивание
	push dword 25
	pop dword [a]

	;Присваивание
	push dword 5
	pop dword [b]

	;Присваивание
	push dword dbl

	;WRITE()
SECTION .data
	msg0 db "dou:", ' ', 0
SECTION .text
	push dword msg0
	call printf
	add esp, 4
	;WRITE()
	push dword [dou]
SECTION .data
	msg1 db "%f", 0xA, 0
SECTION .text
	push dword msg1
	call printf
	add esp, 8
	;цикл WHILE
L000:
	push dword [r]
	push dword 5
	pop ecx
	pop eax
	mov ebx, 1
	cmp eax, ecx
	jb	L002

	xor ebx, ebx
L002:
	push ebx
	pop eax
	cmp eax, 0
	je	L001
	;WRITE()
SECTION .data
	msg2 db "do{}while test", ' ', 0
SECTION .text
	push dword msg2
	call printf
	add esp, 4
	;READ()
	push dword _buf
	push dword ifmt
	call scanf
	add esp, 8
	mov eax, [_buf]
	mov [r], eax
	;Присваивание
	push dword [h]
	push dword [r]
	;Сложение
	pop ecx
	pop eax
	add eax, ecx
	push eax
	pop dword [h]

	;WRITE()
SECTION .data
	msg3 db " r:", ' ', 0
SECTION .text
	push dword msg3
	call printf
	add esp, 4
	;WRITE()
	push dword [r]
SECTION .data
	msg4 db "%d", 0xA, 0
SECTION .text
	push dword msg4
	call printf
	add esp, 8
	;WRITE()
SECTION .data
	msg5 db " h:", ' ', 0
SECTION .text
	push dword msg5
	call printf
	add esp, 4
	;WRITE()
	push dword [h]
SECTION .data
	msg6 db "%d", 0xA, 0
SECTION .text
	push dword msg6
	call printf
	add esp, 8
	;WRITE()
SECTION .data
	msg7 db "25/5: ", ' ', 0
SECTION .text
	push dword msg7
	call printf
	add esp, 4
	;Присваивание
	push dword [a]
	push dword [b]
	;Деление
	xor edx, edx
	pop ecx
	pop eax
	div ecx
	push eax
	pop dword [c]

	;WRITE()
SECTION .data
	msg8 db " c:", ' ', 0
SECTION .text
	push dword msg8
	call printf
	add esp, 4
	;WRITE()
	push dword [c]
SECTION .data
	msg9 db "%d", 0xA, 0
SECTION .text
	push dword msg9
	call printf
	add esp, 8
	jmp	L000
L001:
	;цикл ELSE
	push dword [r]
	push dword 5
	pop ecx
	pop eax
	mov ebx, 1
	cmp eax, ecx
	je	L005

	xor ebx, ebx
L005:
	push ebx
	pop eax
	cmp eax, 0
	je	L003
	;READ()
	push dword _buf
	push dword ifmt
	call scanf
	add esp, 8
	mov eax, [_buf]
	mov [r], eax
	;WRITE()
SECTION .data
	msg10 db "if test", ' ', 0
SECTION .text
	push dword msg10
	call printf
	add esp, 4
	jmp	L004
L003:
	;WRITE()
SECTION .data
	msg11 db "else test", ' ', 0
SECTION .text
	push dword msg11
	call printf
	add esp, 4
L004:
	;Завершение
	push 0
	call exit
