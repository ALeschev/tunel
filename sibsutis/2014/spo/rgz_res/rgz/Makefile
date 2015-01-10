all: cw.y cw.l
	bison -dy cw.y
	flex cw.l
	gcc -g lex.yy.c cw.tab.c -o analyzer