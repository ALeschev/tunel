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

"int" return TVAR1;
"string" return TVAR2;
"write" return WRITE;
"read" return READ;

({letter}|"_")({letter}|{digit}|"_")* {
	if (strcmp("while",yytext)==0) { return WHILE; }
	else if (strcmp("if",yytext)==0) { return IF; }
	else if (strcmp("else",yytext)==0) { return ELSE; }
	else { yylval=(YYSTYPE)strdup(yytext); return ID; }
}

{digit}+ {
	yylval.integer=atoi(yytext);
	return INTEGER;
}

{arifm}		return *yytext;
{compare}	return *yytext;
{bools}		return *yytext;

":=" { return ASSIGN; }
"!=" { return NEQ; }

{delim} { return *yytext; }

{ws}+ {}

(\$)[^(\$)]*(\$) {}


. {
	char buf[1024];
	sprintf(buf, "line:%d Unknown character: %s", yylineno, yytext);
	yyerror(buf);
}
