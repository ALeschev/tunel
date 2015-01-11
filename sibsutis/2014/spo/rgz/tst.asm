global main
extern printf
extern scanf
extern exit

SECTION .bss
	_buf resb 10

SECTION .data
	ifmt db "%d", 0
	sfmt db "%d", 0
	dfmt db "%lf", 0
	h dd 0
	r dd 0
	a dd 0
	b dd 0
	c dd 0
	dou dq 0

	dou_temp dq 0


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
SECTION .data
	dou_tmp0 dq 2.123000
SECTION .text
	fld qword [dou_tmp0]
	fstp qword [dou]

	;Присваивание
SECTION .data
	dou_temp_tmp1 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp1]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg2 db "enter double value for sum test:", ' ', 0
SECTION .text
	push dword msg2
	call printf
	add esp, 4
	;READ()
	push dword _buf
	push dword dfmt
	call scanf
	add esp, 8
	fld qword [_buf]
	fstp qword [dou]
	;WRITE()
SECTION .data
	msg3 db "sum test: 100.123456 + ", ' ', 0
SECTION .text
	push dword msg3
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg4 db "%f", 0xA, 0
SECTION .text
	push dword msg4
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg5 db "= ", ' ', 0
SECTION .text
	push dword msg5
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fld qword [dou]
	;Сложение [3][3]
	fadd st0, st1
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg6 db "%f", 0xA, 0
SECTION .text
	push dword msg6
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg7 db "enter double value for sub test:", ' ', 0
SECTION .text
	push dword msg7
	call printf
	add esp, 4
	;READ()
	push dword _buf
	push dword dfmt
	call scanf
	add esp, 8
	fld qword [_buf]
	fstp qword [dou]
	;WRITE()
SECTION .data
	msg8 db "sub test: 100.123456 - ", ' ', 0
SECTION .text
	push dword msg8
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg9 db "%f", 0xA, 0
SECTION .text
	push dword msg9
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg10 db "= ", ' ', 0
SECTION .text
	push dword msg10
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fld qword [dou]
	;Вычитание [3][3]
	fsubp st1, st0
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg11 db "%f", 0xA, 0
SECTION .text
	push dword msg11
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg12 db "enter double value for mul test:", ' ', 0
SECTION .text
	push dword msg12
	call printf
	add esp, 4
	;READ()
	push dword _buf
	push dword dfmt
	call scanf
	add esp, 8
	fld qword [_buf]
	fstp qword [dou]
	;Присваивание
SECTION .data
	dou_temp_tmp13 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp13]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg14 db "mul test: 100.123456 * ", ' ', 0
SECTION .text
	push dword msg14
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg15 db "%f", 0xA, 0
SECTION .text
	push dword msg15
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg16 db "= ", ' ', 0
SECTION .text
	push dword msg16
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fld qword [dou]
	;Умножение [3][3]
	fmul st0, st1
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg17 db "%f", 0xA, 0
SECTION .text
	push dword msg17
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg18 db "enter double value for div test:", ' ', 0
SECTION .text
	push dword msg18
	call printf
	add esp, 4
	;READ()
	push dword _buf
	push dword dfmt
	call scanf
	add esp, 8
	fld qword [_buf]
	fstp qword [dou]
	;Присваивание
SECTION .data
	dou_temp_tmp19 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp19]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg20 db "div test: 100.123456 / ", ' ', 0
SECTION .text
	push dword msg20
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg21 db "%f", 0xA, 0
SECTION .text
	push dword msg21
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg22 db "= ", ' ', 0
SECTION .text
	push dword msg22
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fdiv qword [dou]
	;Деление [3][3]
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg23 db "%f", 0xA, 0
SECTION .text
	push dword msg23
	call printf
	add esp, 12
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
	;Сложение [1][1]
	pop ecx
	pop eax
	add eax, ecx
	push eax
	pop dword [h]

	;WRITE()
SECTION .data
	msg24 db " r:", ' ', 0
SECTION .text
	push dword msg24
	call printf
	add esp, 4
	;WRITE()
	push dword [r]
SECTION .data
	msg25 db "%d", 0xA, 0
SECTION .text
	push dword msg25
	call printf
	add esp, 8
	;WRITE()
SECTION .data
	msg26 db " h:", ' ', 0
SECTION .text
	push dword msg26
	call printf
	add esp, 4
	;WRITE()
	push dword [h]
SECTION .data
	msg27 db "%d", 0xA, 0
SECTION .text
	push dword msg27
	call printf
	add esp, 8
	;WRITE()
SECTION .data
	msg28 db "25/5: ", ' ', 0
SECTION .text
	push dword msg28
	call printf
	add esp, 4
	;Присваивание
	push dword [a]
	push dword [b]
	;Деление [1][1]
	xor edx, edx
	pop ecx
	pop eax
	div ecx
	push eax
	pop dword [c]

	;WRITE()
	push dword [c]
SECTION .data
	msg29 db "%d", 0xA, 0
SECTION .text
	push dword msg29
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
	msg30 db "if test", ' ', 0
SECTION .text
	push dword msg30
	call printf
	add esp, 4
	jmp	L004
L003:
	;WRITE()
SECTION .data
	msg31 db "else test", ' ', 0
SECTION .text
	push dword msg31
	call printf
	add esp, 4
L004:
	;Завершение
	push 0
	call exit
