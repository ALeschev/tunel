%option noyywrap

%{
#include <stdlib.h>
void yyerror (char *);
#include “y.tab.h”
%}

%%

[0–9]+ {
	yylval = atoi(yytext);
	return INTEGER;
	}
[+–\n] {return *yytext;}
[ \t] ; /* пропускаем пробелы и табуляции */
. {yyerror(“Неизвестный символ!”);}
%%
