%expect 1
%{
#include "lib_oper.hpp"

#define YYSTYPE node *

int yylex();
void yyerror(char *);

extern int if_count,while_count,flag_error;

extern node *list_line_begin;
extern FILE *f_asm;

%}

%token IDENTIFIER
%token INT_CONSTANT
%token FLOAT_CONSTANT
%token WHILE
%token IF
%token THEN
%token ELSE
%token INT
%token UINT
%token PRINT
%token SCAN
%token COMPARE
%token NO_COMPARE
%token MORE_COM
%token LESS_COM

%left '<' '>' COMPARE NO_COMPARE MORE_COM LESS_COM NEG
%right '='
%left '+' '-'
%left '*' '/'

%%
programm:
	programm line		{ if (flag_error==0) beat_node($2); }
	|
;

list_line:
	list_line line		{ add_list_line($2); }
	|
;

line:
	expr ';' 				{ $$=$1; }
	|WHILE '(' logic_expr ')' line		{ $$=add_while($3,$5); }
	|IF '(' logic_expr ')' THEN line		{ if_count++;$$=add_if($3,$6,NULL); }
	|IF '(' logic_expr ')' THEN line ELSE line	{ if_count++;$$=add_if($3,$6,$8); }
	|'{' list_line '}'			{ $$=add_list_operator($2); }
	|INT id_list ';'			{ id_declaration(integer); }
	|UINT id_list ';'			{ id_declaration(uinteger); }
	|PRINT '(' id ')' ';'			{ $$=add_print(); }
	|SCAN '(' id ')' ';'			{ $$=add_scan(); }
	|error ';'			{ flag_error=1; }
;

id_list:
	 id dop_id_list
;

dop_id_list:
	',' id dop_id_list
	|
;

id:
	IDENTIFIER	{ push_id((char *)yylval); }
;

logic_expr:
	expr '>' expr		{ $$=add_operator(more, $1, $3); }
	| expr '<' expr		{ $$=add_operator(less, $1, $3); }
	| expr COMPARE expr	{ $$=add_operator(comp, $1, $3); }
	| expr NO_COMPARE expr	{ $$=add_operator(uncomp, $1, $3); }
	| expr MORE_COM expr	{ $$=add_operator(more_comp, $1, $3); }
	| expr LESS_COM expr	{ $$=add_operator(less_comp, $1, $3); }
;

expr:
	id			{ $$=add_identifier(); }
	|id '=' expr		{ $$=add_operator(assign ,add_identifier(), $3); }
	|INT_CONSTANT		{ $$=add_int_const((int)yylval); }
	| '(' expr ')'		{ $$=$2; }
	| expr '+' expr		{ $$=add_operator(add ,$1 , $3); }
	| expr '-' expr		{ $$=add_operator(dim ,$1 , $3); }
	| expr '*' expr		{ $$=add_operator(mult ,$1 , $3); }
	| expr '/' expr		{ $$=add_operator(divd ,$1 , $3); }
	| '-' expr %prec NEG	{ $$=add_operator(neg ,$2 , NULL); }
;

%%

void yyerror(char *s) {
	fprintf(stderr,"Error: %s\n",s);
}

int main(int argc, char** argv)
{
	int i;
	if(argc < 2) 
	{ 
		perror("Input file name is not specified"); 
		return 1; 
	} 
	if ( !fileopen(argv[1]) ) return -1;
	if ( !(f_asm = fopen("out.asm", "w")) ) return -2;
	fprintf(f_asm,"extern printf\nextern scanf\nextern exit\nSECTION .text\nglobal main\nmain:\n");
	yyparse();
	fprintf(f_asm,"\txor eax, eax\n\tpush eax\n\tcall exit\nSECTION .data\n");
	for (i=0;i<hash_P;i++)
		if (tabl[i].name!=NULL)
			fprintf(f_asm,"\t%s: dd 0\n",tabl[i].asm_name);
	fprintf(f_asm,"\tmessage db \":>\", 0\n\tlen equ $ - message\n\tfmt_pr_i db \"%%d\", 0xA, 0\n\tfmt_pr_ui db \"%%u\", 0xA, 0\n\tfmt_sc_i db \"%%d\", 0\n\tfmt_sc_ui db \"%%u\", 0\nSECTION .bss\n\tbuff resb 1");
	fclose(f_asm);
	free_table();
	if (flag_error==0)
		return 0;
	else
		return 1;
}

