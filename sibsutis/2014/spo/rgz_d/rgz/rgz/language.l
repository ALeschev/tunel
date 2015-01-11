%{
#include <stdlib.h>
#include "y.tab.h"
void yyerror(char *);

#define SIZE 8

char* keywords[SIZE] = {"while", "if", "then", "else", "int", "uint", "print", "scan"};
%}

letter [a-zA-Z]
digit [0-9]
%%

({letter}|'_')({letter}|{digit}|'_')* {
	yylval = (int)yytext;
	switch (resWord(yytext))
	{
		case 0:
	          return WHILE;
		case 1:
	          return IF;
		case 2:
	          return THEN;
		case 3:
	          return ELSE;
		case 4:
	          return INT;
		case 5:
	          return UINT;
		case 6:
	          return PRINT;
		case 7:
	          return SCAN;
		default:
	          return IDENTIFIER;
	}
}

{digit}+ 	{ yylval = atoi (yytext);return INT_CONSTANT; }

[-+*/;(){}<>=,]		return *yytext;

"=="		{ return COMPARE; }

"!="		{ return NO_COMPARE; }

">="		{ return MORE_COM; }

"<="		{ return LESS_COM; }

[ \t\n]		;

.		yyerror("invalid character");

%%

int resWord(char* id) 
{ 
     int i;
     for(i = 0; i < SIZE; i++)
          if(strcmp(id, keywords[i]) == 0)
               return i;
     return -1;
}

int fileopen(char std[])
{
	yyin = fopen(std, "r"); 
	if(yyin == NULL) 
	{ 
		perror(std); 
		return 0; 
	} 
	return 1;
}

int yywrap()	{
	return 1;
}

