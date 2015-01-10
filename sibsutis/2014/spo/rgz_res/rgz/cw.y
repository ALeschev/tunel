%{
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>

#define MAX_NAME_ID 32

#include "cw.tab.h"
#include "chaintable.h"
#include "assemgener.h"

extern int yylineno;

FILE *ifile,*ofile;

int yylex();

%}

%union {
	struct node *nodet;
	int integer;
	double dbl;
	char *string;
}

%token IF WHILE READ WRITE TVAR1 TVAR2 TVAR3
%token <integer> INTEGER
%token <double> DOUBLE
%token <string> STR ID

%nonassoc IFX
%nonassoc ELSE

%right ASSIGN
%left  '&' '|'
%left '<' '>' '=' NEQ
%left  '+' '-'
%left  '*' '/'

%type <nodet> declar funct state block expr bool

%%
program:
	  declar block {
	  printf ("[%d] ---- \n", __LINE__);
			printf("global main\n");
			printf("extern printf\n");
			printf("extern scanf\n");
			printf("extern exit\n\n");
			printf("SECTION .bss\n\t_buf resb 10\n\n");
			printf("SECTION .data\n");
			printf("\tifmt db \"%cd\", 0\n",'%');
			printf("\tsfmt db \"%cd\", 0\n",'%');
			ex($1);
			freeNode($1);
			printf("\n");

			printf("SECTION .text\n\tmain:\n");
			ex($2);
			printf("\t;Завершение\n\tpush 0\n\tcall exit\n"); 
			freeNode($2);
		}
	;

declar:
	  declar TVAR1 ID ';' {
			if (!table_add($3, TVAR1))
			{
				char _buf[MAX_NAME_BUF];
				sprintf(_buf, "line:%d Redefined identifier: %s", yylineno, $3);
				yyerror(__LINE__, _buf);
			}
			printf ("[%d] ---- \n", __LINE__);
			yyerror(__LINE__, "int NNNOOORRRMM\n");
			$$=creat_oper('I', 2, $1, creat_Id($3));
		}
	| declar TVAR3 ID ';' {
			if (!table_add($3, TVAR3))
			{
				char _buf[MAX_NAME_BUF];
				sprintf(_buf, "line:%d Redefined identifier: %s", yylineno, $3);
				yyerror(__LINE__, _buf);
			}
			printf ("[%d] ---- \n", __LINE__);
			yyerror(__LINE__, "double NNNOOORRRMM\n");
			$$=creat_oper('D', 2, $1, creat_Id($3));
		}
	| declar TVAR2 ID ';' {
			if (!table_add($3, TVAR2))
			{
				char _buf[MAX_NAME_BUF];
				sprintf(_buf, "line:%d Redefined identifier: %s", yylineno, $3);
				yyerror(__LINE__, _buf);
			}
			printf ("[%d] ---- \n", __LINE__);
			yyerror(__LINE__, "string NNNOOORRRMM\n");
			$$=creat_oper('S', 2, $1, creat_Id($3));
		}
	| {$$=NULL;}
	;
	
funct:
	funct state {$$=creat_oper('F', 2, $1, $2);}
	| {$$=NULL;}
	;	
	
state:
	  ID ASSIGN expr ';' {
			if (table_find($1)==NULL)
			{
				char _buf[MAX_NAME_BUF];
				sprintf(_buf, "line:%d Undefined identifier: %s", yylineno, $1);
				yyerror(__LINE__, _buf);
			}
			printf ("[%d] ---- \n", __LINE__);
			$$=creat_oper(ASSIGN, 2, creat_Id($1), $3);
		}
	| WRITE '(' ID ')' ';' {
			if (table_find($3)==NULL)
			{
				char _buf[MAX_NAME_BUF];
				sprintf(_buf, "line:%d Undefined identifier: %s", yylineno, $3);
				yyerror(__LINE__, _buf);
			}
			printf ("[%d] ---- \n", __LINE__);
			$$=creat_oper(WRITE, 1, creat_Id($3));
		}
	| WRITE '(' STR ')' ';' { $$=creat_oper(WRITE, 1, creat_string($3));}
	| READ '(' ID ')' ';' {
			if (table_find($3)==NULL)
			{
				char _buf[MAX_NAME_BUF];
				sprintf(_buf, "line:%d Undefined identifier: %s", yylineno, $3);
				yyerror(__LINE__, _buf);
			}
			$$=creat_oper(READ, 1, creat_Id($3));
		}
	| WHILE '(' bool ')' block {$$=creat_oper(WHILE, 2, $3, $5);}
	| IF '(' bool ')' block %prec IFX {$$=creat_oper(IF, 2, $3, $5);}
	| IF '(' bool ')' block ELSE block {$$=creat_oper(ELSE, 3, $3, $5, $7);}
	;

block:
	  '{' funct '}' {$$=$2;}
	;

expr:
	  expr '+' expr {$$=creat_oper('+', 2, $1, $3);}
	| expr '-' expr {$$=creat_oper('-', 2, $1, $3);}
	| expr '*' expr {$$=creat_oper('*', 2, $1, $3);}
	| expr '/' expr {$$=creat_oper('/', 2, $1, $3);}
	| ID {$$=creat_Id($1);}
	| STR {printf ("[%d] ---- \n", __LINE__); $$=creat_string($1);}
	| INTEGER {printf ("[%d] ---- \n", __LINE__); $$=creat_int($1);}
	| DOUBLE {printf ("[%d] ---- \n", __LINE__); $$=creat_double($1);}
	| '(' expr ')' {$$=$2;}
	;

bool:
	  expr '=' expr {$$=creat_oper('=', 2, $1, $3);}
	| expr '<' expr {$$=creat_oper('<', 2, $1, $3);}
	| expr '>' expr {$$=creat_oper('>', 2, $1, $3);}
	| expr NEQ expr {$$=creat_oper('!', 2, $1, $3);}
	| bool '&' bool {$$=creat_oper('&', 2, $1, $3);}
	| bool '|' bool {$$=creat_oper('|', 2, $1, $3);}
	;

%%
void yyerror(int line, char *s)
{
	fprintf(stderr, "[%d] error: %s\n", line, s);
}

int main(int argc, char **argv)
{
	yylineno=1;
	
/*	const char *sh_opt="i:o:";
	char *ofile=NULL,*ifile=NULL;
	int opt;
	
	while ((opt=getopt_long(argc, argv, sh_opt))!=(-1))
	{
		switch (opt)
		{
		case 'i':
			{
				ifile=fopen(optarg,"r");
				if (!ifile)
				{
					printf("Нудалось открыть файл %s\n",optarg);
					return -1;
				}
			break;
			}
		case 'o':
			{
				ofile=fopen(optarg,"w");
			break;
			}
		default:
			printf("Я не знаю такой опции!\n");
			printf("Есть только:\n\t -i - входной файл\n\t -o - выходной файл\n");
			break;
		};
	};
	if (ofile!=NULL && ifile==NULL) ifile=fopen("tst.t","r");
	if (ofile==NULL && ifile!=NULL) ofile=fopen("tst.asm","w");
	if (ofile==NULL && ifile==NULL)
	{
		ifile=fopen("tst.t","r");
		ofile=fopen("tst.asm","w");
	}
*/

	freopen (argv[1],"r",stdin);
	freopen (argv[2],"w+",stdout);
	
	table_init();
	yyparse();
	table_free_start();
	
	return 0;
}
