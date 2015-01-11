extern printf
extern scanf
extern exit
SECTION .text
global main
main:
	push message
	call printf
	add esp, 4
	push buff
	push fmt_sc_i
	call scanf
	add esp, 8
	mov eax, [buff]
	mov [v97], eax
	mov eax, [v99]
	mov eax, 4
	mov [v99], eax
	mov eax, [v97]
	cmp eax, 5
	jle  else_if_1
	mov eax, [v97]
	imul eax, 5
	push eax
	mov eax, 23
	add eax, [v99]
	xor edx, edx
	mov ebx, 3
	idiv ebx
	mov ecx, eax
	pop eax
	add eax, ecx
	mov ecx, eax
	mov eax, [v98]
	mov eax, ecx
	mov [v98], eax
else_if_1:
	mov ecx, [v98]
	push ecx
	push fmt_pr_i
	call printf
	add esp, 8
	mov eax, [v97]
	mov eax, 1
	mov [v97], eax
begin_while_1:
	mov eax, [v97]
	cmp eax, 10
	jge  end_while_1
	mov eax, [v97]
	cmp eax, 5
	jg  else_if_2
	mov eax, 3
	imul eax, [v97]
	mov ecx, eax
	mov eax, [v98]
	sub eax, ecx
	mov ecx, eax
	mov eax, [v99]
	mov eax, ecx
	mov [v99], eax
	jmp end_if_2
else_if_2:
	mov eax, [v98]
	sub eax, 4
	mov ecx, eax
	mov eax, [v98]
	mov eax, ecx
	mov [v98], eax
	mov eax, [v97]
	add eax, 125
	xor edx, edx
	mov ebx, 5
	idiv ebx
	sub eax, [v98]
	push eax
	mov eax, [v97]
	imul eax, [v97]
	mov ecx, eax
	pop eax
	add eax, ecx
	mov ecx, eax
	mov eax, [v99]
	mov eax, ecx
	mov [v99], eax
end_if_2:
	mov eax, [v97]
	add eax, 1
	mov ecx, eax
	mov eax, [v97]
	mov eax, ecx
	mov [v97], eax
	mov ecx, [v97]
	push ecx
	push fmt_pr_i
	call printf
	add esp, 8
	mov ecx, [v99]
	push ecx
	push fmt_pr_i
	call printf
	add esp, 8
	jmp begin_while_1
end_while_1:
	mov ecx, [v98]
	push ecx
	push fmt_pr_i
	call printf
	add esp, 8
	xor eax, eax
	push eax
	call exit
SECTION .data
	v97: dd 0
	v98: dd 0
	v99: dd 0
	message db ":>", 0
	len equ $ - message
	fmt_pr_i db "%d", 0xA, 0
	fmt_pr_ui db "%u", 0xA, 0
	fmt_sc_i db "%d", 0
	fmt_sc_ui db "%u", 0
SECTION .bss
	buff resb 1