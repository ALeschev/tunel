all:
	bison -dy cw.y
	flex cw.l
	gcc lex.yy.c cw.tab.c -o analyzer