all: a.out asm

a.out: language.y language.l lib_oper.hpp
	bison -dy language.y
	flex language.l  
	gcc lex.yy.c y.tab.c
	rm lex.yy.c
	rm y.tab.c
	rm y.tab.h

asm: a.out tst 
	./a.out tst
	nasm -f elf out.asm
	gcc out.o -o asm
	rm out.o
	./asm
