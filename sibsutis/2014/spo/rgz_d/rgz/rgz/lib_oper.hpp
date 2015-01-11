#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define hash_P 839
#define MAX_CHAR 64

extern void yyerror(char *);

int if_count=0,while_count=0,flag_error=0;

FILE *f_asm;

typedef enum { CONSTANTA, ID, OPER, IF_OPER, LIST_OPER } view_node;
typedef enum { add, dim, mult, divd, assign, more, less, comp, uncomp, more_comp, less_comp, neg, print, scan, while_do, if_then } type_oper;
typedef union{
	int id;
	int int_val;
	unsigned int uint_val;
	type_oper opr;
	void *ptr;
}val_type;

typedef enum { integer, uinteger, no_type } Type;

typedef struct Node{
	view_node view;	
	Type type;
	val_type val;
	struct Node *left,*right;
} node;

void free_tree(node *ver)
{
	if (ver!=NULL)
	{
		if (ver->left!=NULL)
			free_tree(ver->left);
		if (ver->right!=NULL)
			free_tree(ver->right);
		free(ver);
	}
}

typedef struct List_Node{
	node *begin;
	struct List_Node *next;
} list_node;

typedef struct{
	char *name;
	Type type;
	char asm_name[7];
}symbol_tabl;

symbol_tabl tabl[hash_P];

list_node *begin_list_node=NULL,*last_list_node=NULL;

void init_tabl()
{
	int i;
	for (i=0;i<hash_P;i++)
		tabl[i].name=NULL;
}

int hash(const char sName[])
{
	int s=0,i;
	for (i=0;(sName[i]!='\0')&&(i<MAX_CHAR);i++)
		s=(s*257+sName[i])%hash_P;
	return s;
}

int rehash(int iNum)
{
	srand(iNum);
	return iNum*rand()%hash_P;
}

int add_hash(char sName[],Type tp)
{
	int a=hash(sName);
	while (tabl[a].name!=NULL)
	{
		if (strncmp(tabl[a].name,sName,MAX_CHAR)==0)
			return -1;
		a=rehash(a);
	}
	tabl[a].name=sName;
	tabl[a].type=tp;
	sprintf(tabl[a].asm_name,"v%d",a);
	return a;
}

int find(const char sName[])
{
	int a=hash(sName);
	while (tabl[a].name!=NULL)
	{
		if (strncmp(tabl[a].name,sName,MAX_CHAR)==0)
			return a;
		a=rehash(a);
	}
	return -1;
}

void free_table()
{
	int i;
	for (i=0;i<hash_P;i++)
		if (tabl[i].name!=NULL)
			free(tabl[i].name);
}

typedef struct List_id{
	char *name;
	struct List_id *next;
} list_id;

list_id *ver_str_stack=NULL;

void push_id(const char str_id[])
{
	list_id *temp=malloc( sizeof(list_id) );
	temp->name=malloc( sizeof(char)*MAX_CHAR );
	strncpy(temp->name,str_id,MAX_CHAR);
	temp->next=ver_str_stack;
	ver_str_stack=temp;
}

list_id *pop_id()
{
	list_id *temp;
	if (ver_str_stack!=NULL)
	{
		temp=ver_str_stack;
		ver_str_stack=ver_str_stack->next;
		return temp;
	}
	else
		return NULL;
}

void id_declaration(Type tp)
{
	list_id *temp;
	int id;
	while ( (temp=pop_id())!=NULL )
	{
		if ((id=add_hash(temp->name,tp))==-1)
			yyerror("Повторное объявленние переменной");
		free(temp);
	}
}

node *add_int_const(int val)
{
	node *temp=malloc( sizeof(node) );
	temp->view=CONSTANTA;
	temp->type=integer;
	if ((temp->val.int_val=val)==-1)
	{
		yyerror("использование необъявленной переменной");
		free(temp);
		return NULL;
	}
	temp->right=temp->left=NULL;
	return temp;
}

node *add_uint_const(int val)
{
	node *temp=malloc( sizeof(node) );
	temp->view=CONSTANTA;
	temp->type=uinteger;
	if ((temp->val.uint_val=val)==-1)
	{
		yyerror("использование необъявленной переменной");
		free(temp);
		return NULL;
	}
	temp->right=temp->left=NULL;
	return temp;
}

node *add_identifier()
{
	node *temp=malloc( sizeof(node) );
	list_id *temp_id=pop_id();
	temp->view=ID;
	temp->val.id=find(temp_id->name);
	temp->type=tabl[temp->val.id].type;
	temp->right=temp->left=NULL;
	free(temp_id);
	return temp;
}

node* add_print()
{
	node *temp=malloc( sizeof(node) );
	temp->view=OPER;
	temp->val.opr=print;
	temp->left=(node *)pop_id();
	return temp;
}

void Print(node* add)
{
	list_id *temp_id=(list_id *)add->left;
	int id=find(temp_id->name);
	if (tabl[id].type==integer)
	{
		fprintf(f_asm,"\tmov ecx, [%s]\n\tpush ecx\n",tabl[id].asm_name);
		fprintf(f_asm,"\tpush fmt_pr_i\n\tcall printf\n\tadd esp, 8\n");
	}
	else
	{
		fprintf(f_asm,"\tmov ecx, [%s]\n\tpush ecx\n",tabl[id].asm_name);
		fprintf(f_asm,"\tpush fmt_pr_ui\n\tcall printf\n\tadd esp, 8\n");
	}
	free(temp_id);
}

node* add_scan()
{
	node *temp=malloc( sizeof(node) );
	temp->view=OPER;
	temp->val.opr=scan;
	temp->left=(node *)pop_id();
	return temp;
}

void Scan(node* add)
{
	list_id *temp_id=(list_id *)add->left;
	int id=find(temp_id->name);
	if (id==-1)	
	{
		yyerror("использование необъявленной переменной");
		return;
	}
	fprintf(f_asm,"\tpush message\n\tcall printf\n\tadd esp, 4\n\tpush buff\n");
	if (tabl[id].type==integer)
		fprintf(f_asm,"\tpush fmt_sc_i\n");
	else
		fprintf(f_asm,"\tpush fmt_sc_ui\n");
	fprintf(f_asm,"\tcall scanf\n\tadd esp, 8\n\tmov eax, [buff]\n\tmov [%s], eax\n",tabl[id].asm_name);
}

node *add_operator(type_oper opr,node *left,node *right)
{
	node *temp=malloc( sizeof(node) );
	if ((right!=NULL) && (left->view==CONSTANTA) && (right->view==CONSTANTA))
	{
		temp->view=CONSTANTA;
		temp->type=left->type;
		switch (opr)
		{
			case add:
				if (left->type==integer)
					temp->val.int_val=left->val.int_val+right->val.int_val;
				else
					temp->val.uint_val=left->val.uint_val+right->val.uint_val;
				break;
			case dim:
				if (left->type==integer)
					temp->val.int_val=left->val.int_val-right->val.int_val;
				else
					temp->val.uint_val=left->val.uint_val-right->val.uint_val;
				break;
			case mult:
				if (left->type==integer)
					temp->val.int_val=left->val.int_val*right->val.int_val;
				else
					temp->val.uint_val=left->val.uint_val*right->val.uint_val;
				break;
			case divd:
				if (left->type==integer)
					temp->val.int_val=left->val.int_val/right->val.int_val;
				else
					temp->val.uint_val=left->val.uint_val/right->val.uint_val;
				break;
		}
		free(left);
		free(right);
		temp->left=temp->right=NULL;
		return temp;
	}
	if ((left->view==CONSTANTA) && (opr==neg))
	{
		temp->view=CONSTANTA;
		temp->type=left->type;
		if (left->type==integer)
			temp->val.int_val=-1*left->val.int_val;
		else
			temp->val.uint_val=-1*left->val.uint_val;
		free(left);
		return temp;
	}
	temp->view=OPER;
	temp->val.opr=opr;
	temp->left=left;
	temp->right=right;
	return temp;
}

node * add_while(node * usl,node * body)
{
	node *temp=malloc( sizeof(node) );
	temp->view=OPER;
	temp->val.opr=while_do;
	temp->left=usl;
	temp->right=body;
	return temp;
}

void compl_while(node * ver)
{
	char begin_label_name[20],end_label_name[20];
	while_count++;
	sprintf(begin_label_name,"begin_while_%d",while_count);
	sprintf(end_label_name,"end_while_%d",while_count);
	fprintf(f_asm,"%s:\n",begin_label_name);
	beat_node(ver->left);
	fprintf(f_asm," %s\n",end_label_name);
	beat_node(ver->right);
	fprintf(f_asm,"\tjmp %s\n",begin_label_name);
	fprintf(f_asm,"%s:\n",end_label_name);
}
	
node * add_if(node * usl, node * true_body, node * false_body)
{
	node *temp=malloc( sizeof(node) );
	temp->view=IF_OPER;
	temp->val.ptr=(void *)false_body;
	temp->left=usl;
	temp->right=true_body;
	return temp;
}

void compl_if(node * ver)
{
	char else_if[20],end_if[20];
	sprintf(else_if,"else_if_%d",if_count);
	sprintf(end_if,"end_if_%d",if_count);
	beat_node(ver->left);
	fprintf(f_asm," %s\n",else_if);
	beat_node(ver->right);
	if (ver->val.ptr!=NULL)
	{
		fprintf(f_asm,"\tjmp %s\n",end_if);
		fprintf(f_asm,"%s:\n",else_if);
		beat_node((node *)ver->val.ptr);
		fprintf(f_asm,"%s:\n",end_if);
	}
	else
		fprintf(f_asm,"%s:\n",else_if);
}

int beat_node(node *ver)
{
	char operand2[10];
	list_node *temp=NULL;
	node *fr;
	if (ver->view==LIST_OPER)
	{
		temp=(list_node *)ver->val.ptr;
		while (temp!=NULL)
		{
			beat_node(temp->begin);
			temp=temp->next;
		}
		return 0;
	}
	if (ver->view==IF_OPER)
	{
		compl_if(ver);
		return 0;
	}
	if (ver->view!=OPER) return -1;
	switch (ver->val.opr)
	{
		case print:
			Print(ver);
			return 0;
		case scan:
			Scan(ver);
			return 0;
		case while_do:
			compl_while(ver);
			return 0;
	}
	if ( (ver->view==OPER) && (ver->val.opr==neg) )
	{
		if (ver->left->view==ID)
			fprintf(f_asm,"\tmov eax, [%s]\n",tabl[ver->left->val.id].asm_name);
		fprintf(f_asm,"\tneg eax\n");
		return 0;
	}
	if (ver->left->view==OPER)
	{
		beat_node(ver->left);
		if (ver->right->view==OPER)
		{
			fprintf(f_asm,"\tpush eax\n");
			beat_node(ver->right);
			fprintf(f_asm,"\tmov ecx, eax\n\tpop eax\n");
			strcpy(operand2,"ecx");
		}
		else
		{
			if (ver->right->view==CONSTANTA)
				sprintf(operand2,"%d",ver->right->val.int_val);
			else
				sprintf(operand2,"[%s]",tabl[ver->right->val.id].asm_name);
		}
	}
	else
	{
		if (ver->right->view==OPER)
		{
			beat_node(ver->right);
			fprintf(f_asm,"\tmov ecx, eax\n");
			strcpy(operand2,"ecx");
		}
		if (ver->left->view==CONSTANTA)
			fprintf(f_asm,"\tmov eax, %d\n",ver->left->val.int_val);
		if (ver->left->view==ID)
			fprintf(f_asm,"\tmov eax, [%s]\n",tabl[ver->left->val.id].asm_name);
		if (ver->right->view==CONSTANTA)
			sprintf(operand2,"%d",ver->right->val.int_val);
		if (ver->right->view==ID)
			sprintf(operand2,"[%s]",tabl[ver->right->val.id].asm_name);
	}
	switch (ver->val.opr)
	{
		case add:
			fprintf(f_asm,"\tadd eax, %s\n",operand2);
			break;
		case dim:
			fprintf(f_asm,"\tsub eax, %s\n",operand2);
			break;
		case mult:
			if (ver->left->type==integer)
				fprintf(f_asm,"\timul eax, %s\n",operand2);
			else
				fprintf(f_asm,"\tmov ebx, %s\n\tmul ebx\n",operand2);
			break;
		case divd:
			fprintf(f_asm,"\txor edx, edx\n\tmov ebx, %s\n",operand2);
			if (ver->left->type==integer)
				fprintf(f_asm,"\tidiv ebx\n");
			else
				fprintf(f_asm,"\tdiv ebx\n");
			break;
		case assign:
			fprintf(f_asm,"\tmov eax, %s\n\tmov [%s], eax\n",operand2,tabl[ver->left->val.id].asm_name);
			break;
		case more:
			fprintf(f_asm,"\tcmp eax, %s\n\tjle ",operand2);
			break;
		case less:
			fprintf(f_asm,"\tcmp eax, %s\n\tjge ",operand2);
			break;
		case comp:
			fprintf(f_asm,"\tcmp eax, %s\n\tjne ",operand2);
			break;
		case uncomp:
			fprintf(f_asm,"\tcmp eax, %s\n\tje ",operand2);
			break;
		case more_comp:
			fprintf(f_asm,"\tcmp eax, %s\n\tjl ",operand2);
			break;
		case less_comp:
			fprintf(f_asm,"\tcmp eax, %s\n\tjg ",operand2);
			break;
	}
	free(ver->right);
	free(ver->left);
}

node * add_list_operator()
{
	node *temp=malloc( sizeof(node) );
	temp->view=LIST_OPER;
	temp->val.ptr=(void *)begin_list_node;
	begin_list_node=NULL;
	return temp;
}

void add_list_line(node *add_node)
{
	list_node *temp=malloc( sizeof(list_node) );
	temp->begin=add_node;
	temp->next=NULL;
	if (begin_list_node==NULL)
		last_list_node=begin_list_node=temp;
	else
	{
		last_list_node->next=temp;
		last_list_node=temp;
	}
}

