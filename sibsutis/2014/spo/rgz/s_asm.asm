extern printf          ; the C function to be called

SECTION .data          ; Data section

msg  db "sum = %e",0x0a,0x00
x    dd 1.5
y    dd 2.5
z    dd 0
temp dq 0

SECTION .text          ; Code section.

global  main           ; "C" main program 
main:                  ; label, start of main program

fld     dword [x]      ; need to convert 32-bit to 64-bit
fld     dword [y]
fadd
fstp    dword [z]      ; store sum in z

mov     eax, [z]

fld     dword [z]      ; transform z in 64-bit word by pushing in stack
fstp    qword [temp]   ; and popping it back as 64-bit quadword

         
push    dword [temp+4] ; push temp as 2 32-bit words
push    dword [temp]
push    dword msg      ; address of format string
call    printf         ; Call C function
add     esp, 12        ; pop stack 3*4 bytes

mov     eax, 1         ; exit code, 0=normal
mov     ebx, 0
int     0x80