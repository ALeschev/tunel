all:
	bison -dy cw_nat_san.y
	flex cw_nat_san.l
	gcc lex.yy.c cw_nat_san.tab.c