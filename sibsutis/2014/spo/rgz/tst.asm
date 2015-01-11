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

	t_D dq 0


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
; t_double 2.123000
SECTION .data
	dou_tmp0 dq 2.123000
SECTION .text
	fld qword [dou_tmp0]
	fstp qword [dou]

	;Присваивание
; t_double 0.000000
SECTION .data
	t_D_tmp1 dq 0.000000
SECTION .text
	fld qword [t_D_tmp1]
	fstp qword [t_D]

	;Присваивание
; t_double 100.123456
SECTION .data
	dou_temp_tmp2 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp2]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg3 db "static sum: 100.123456 + 75.123 = ", ' ', 0
SECTION .text
	push dword msg3
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	;Сложение [3][3]
; t_double 75.123000
SECTION .data
	dou_temp_tmp4 dq 75.123000
SECTION .text
	fld qword [dou_temp_tmp4]
	fadd st0, st1
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg5 db "%f", 0xA, 0
SECTION .text
	push dword msg5
	call printf
	add esp, 12
	;Присваивание
; t_double 100.123456
SECTION .data
	dou_temp_tmp6 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp6]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg7 db "static sub: 100.123456 - 75.123 = ", ' ', 0
SECTION .text
	push dword msg7
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	;Вычитание [3][3]
; t_double 75.123000
SECTION .data
	dou_temp_tmp8 dq 75.123000
SECTION .text
	fld qword [dou_temp_tmp8]
	fsubp st1, st0
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg9 db "%f", 0xA, 0
SECTION .text
	push dword msg9
	call printf
	add esp, 12
	;Присваивание
; t_double 100.123456
SECTION .data
	dou_temp_tmp10 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp10]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg11 db "static mul: 100.123456 * 75.123 = ", ' ', 0
SECTION .text
	push dword msg11
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	;Умножение [3][3]
; t_double 75.123000
SECTION .data
	dou_temp_tmp12 dq 75.123000
SECTION .text
	fld qword [dou_temp_tmp12]
	fmul st0, st1
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg13 db "%f", 0xA, 0
SECTION .text
	push dword msg13
	call printf
	add esp, 12
	;Присваивание
; t_double 100.123456
SECTION .data
	dou_temp_tmp14 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp14]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg15 db "static div: 100.123456 / 75.123 = ", ' ', 0
SECTION .text
	push dword msg15
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	;Деление [3][3]
; t_double 75.123000
SECTION .data
	dou_temp_tmp16 dq 75.123000
SECTION .text
	fld qword [dou_temp_tmp16]
	fdiv st1, st0
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
	msg18 db "----------------------------------------", ' ', 0
SECTION .text
	push dword msg18
	call printf
	add esp, 4
	;WRITE()
	push dword [t_D+4]
	push dword [t_D]
SECTION .data
	msg19 db "%f", 0xA, 0
SECTION .text
	push dword msg19
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg20 db "enter double value for sum test:", ' ', 0
SECTION .text
	push dword msg20
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
; t_double 100.123456
SECTION .data
	dou_temp_tmp21 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp21]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg22 db "sum test: 100.123456 + ", ' ', 0
SECTION .text
	push dword msg22
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg23 db "%f", 0xA, 0
SECTION .text
	push dword msg23
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg24 db "= ", ' ', 0
SECTION .text
	push dword msg24
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fld qword [dou]
	;Сложение [3][3]
	fadd st0, st1
; t_double 99999.999990
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg25 db "%f", 0xA, 0
SECTION .text
	push dword msg25
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg26 db "enter double value for sub test:", ' ', 0
SECTION .text
	push dword msg26
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
; t_double 100.123456
SECTION .data
	dou_temp_tmp27 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp27]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg28 db "sub test: 100.123456 - ", ' ', 0
SECTION .text
	push dword msg28
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg29 db "%f", 0xA, 0
SECTION .text
	push dword msg29
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg30 db "= ", ' ', 0
SECTION .text
	push dword msg30
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fld qword [dou]
	;Вычитание [3][3]
	fsubp st1, st0
; t_double 99999.999990
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg31 db "%f", 0xA, 0
SECTION .text
	push dword msg31
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg32 db "enter double value for mul test:", ' ', 0
SECTION .text
	push dword msg32
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
; t_double 100.123456
SECTION .data
	dou_temp_tmp33 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp33]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg34 db "mul test: 100.123456 * ", ' ', 0
SECTION .text
	push dword msg34
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg35 db "%f", 0xA, 0
SECTION .text
	push dword msg35
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg36 db "= ", ' ', 0
SECTION .text
	push dword msg36
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fld qword [dou]
	;Умножение [3][3]
	fmul st0, st1
; t_double 99999.999990
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg37 db "%f", 0xA, 0
SECTION .text
	push dword msg37
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg38 db "enter double value for div test:", ' ', 0
SECTION .text
	push dword msg38
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
; t_double 100.123456
SECTION .data
	dou_temp_tmp39 dq 100.123456
SECTION .text
	fld qword [dou_temp_tmp39]
	fstp qword [dou_temp]

	;WRITE()
SECTION .data
	msg40 db "div test: 100.123456 / ", ' ', 0
SECTION .text
	push dword msg40
	call printf
	add esp, 4
	;WRITE()
	push dword [dou+4]
	push dword [dou]
SECTION .data
	msg41 db "%f", 0xA, 0
SECTION .text
	push dword msg41
	call printf
	add esp, 12
	;WRITE()
SECTION .data
	msg42 db "= ", ' ', 0
SECTION .text
	push dword msg42
	call printf
	add esp, 4
	;Присваивание
	fld qword [dou_temp]
	fdiv qword [dou]
	;Деление [3][3]
; t_double 99999.999990
	fstp qword [dou_temp]

	;WRITE()
	push dword [dou_temp+4]
	push dword [dou_temp]
SECTION .data
	msg43 db "%f", 0xA, 0
SECTION .text
	push dword msg43
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
	msg44 db " r:", ' ', 0
SECTION .text
	push dword msg44
	call printf
	add esp, 4
	;WRITE()
	push dword [r]
SECTION .data
	msg45 db "%d", 0xA, 0
SECTION .text
	push dword msg45
	call printf
	add esp, 8
	;WRITE()
SECTION .data
	msg46 db " h:", ' ', 0
SECTION .text
	push dword msg46
	call printf
	add esp, 4
	;WRITE()
	push dword [h]
SECTION .data
	msg47 db "%d", 0xA, 0
SECTION .text
	push dword msg47
	call printf
	add esp, 8
	;WRITE()
SECTION .data
	msg48 db "25/5: ", ' ', 0
SECTION .text
	push dword msg48
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
	msg49 db "%d", 0xA, 0
SECTION .text
	push dword msg49
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
	msg50 db "if test", ' ', 0
SECTION .text
	push dword msg50
	call printf
	add esp, 4
	jmp	L004
L003:
	;WRITE()
SECTION .data
	msg51 db "else test", ' ', 0
SECTION .text
	push dword msg51
	call printf
	add esp, 4
L004:
	;Завершение
	push 0
	call exit
