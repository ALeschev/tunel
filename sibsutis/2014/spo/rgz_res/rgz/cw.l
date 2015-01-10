%option noyywrap nounput noinput

%{
#include <stdio.h>
#include <string.h>
#include "cw.tab.h"

extern void yyerror(char *);
extern char* _fname;

%}

letter [a-zA-Z]
digit [0-9]
arifm [\-\+\*\/]
compare [\<\>\=]
bools [\&\|\!]
delim [{}();,]
ws [ \t\n]

%%

(\")[^(\")]*(\") { yylval=(YYSTYPE)strdup(yytext);	return STR; }

"int" {printf ("MOTHERFUCKER: INT\n" );return TVAR1;}
"string" return TVAR2;
"double" {printf ("MOTHERFUCKER: DOUBLE\n" );return TVAR3;}
"write" return WRITE;
"read" return READ;

({letter}|"_")({letter}|{digit}|"_")* {
	if (strcmp("while",yytext)==0) { return WHILE; }
	else if (strcmp("if",yytext)==0) { return IF; }
	else if (strcmp("else",yytext)==0) { return ELSE; }
	else { yylval=(YYSTYPE)strdup(yytext); return ID; 
		printf ("ELSE MOTHERFUCKER: %s\n", yytext);}
}

{digit}+ {
	yylval.integer=atoi(yytext);
	printf ("INT MOTHERFUCKER: %d\n", yylval.integer);
	return INTEGER;
}

{digit}*\.{digit}* {
	yylval.dbl=atof(yytext);
	printf ("DOUBLE MOTHERFUCKER: %f\n", yylval.dbl);
	return DOUBLE;
}

{arifm}		{printf ("arifm MOTHERFUCKER: %s\n", yytext); return *yytext;}
{compare}	{printf ("compare MOTHERFUCKER: %s\n", yytext);return *yytext;}
{bools}		{printf ("bools MOTHERFUCKER: %s\n", yytext);return *yytext;}

":=" { printf ("MOTHERFUCKER: :=\n"); return ASSIGN; }
"!=" { return NEQ; }

{delim} { return *yytext; }

{ws}+ {}

(\$)[^(\$)]*(\$) {}


. {
	char buf[1024];
	sprintf(buf, "line:%d Unknown character: %s", yylineno, yytext);
	yyerror(buf);
}
