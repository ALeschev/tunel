all: cw.y cw.l
	bison -y cw.y
	flex cw.l
	gcc -g lex.yy.c cw.tab.c -o analyzer