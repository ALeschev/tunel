#define MAX_NAME_BUF 1024

void yyerror(char *s);
extern FILE *f;

enum {tId,tCon,tOp};

/* идентификаторы */
typedef struct
{
	char name[MAX_NAME_ID];
} nodetId;

/* константы */
typedef struct
{
	char type;	/* тип вершины */
	union
	{
		int integer;
		char *string;
	};
} nodetCon;

/* операторы */
typedef struct
{
	int oper;	/* оператор */
	int count_oper;	/* кол-во операндов */
	struct node *nodes[1]; /* операнды */
} nodetoper;

typedef struct node
{
	int type;	/* тип вершины */
	/* объединение должно быть последним в nodet, */
	/* так как nodetoper может динамически увеличиваться */
	union{
		nodetId  id;
		nodetCon con;
		nodetoper oper;
	};
} nodet;

nodet *creat_Id(char value[MAX_NAME_ID])
{
	nodet *point=malloc(sizeof(int)+sizeof(nodetId));
	if(!point)	yyerror("out of memory!");
	point->type=tId;
	strncpy(point->id.name,value,MAX_NAME_ID);

	return point;
}

nodet *creat_int(int value)
{
	nodet *point=malloc(sizeof(int)+sizeof(nodetCon));
	
	if(!point)	yyerror("out of memory!");
	point->type=tCon;
	point->con.type='1';
	point->con.integer=value;

	return point;
}

nodet *creat_string(char *value)
{
	nodet *point=malloc(sizeof(char)+sizeof(nodetCon));
	
	if(!point)	yyerror("out of memory!");
	point->type=tCon;
	point->con.type='2';
	point->con.string=value;

	return point;
}

nodet *creat_oper(int oper, int count_oper, ...)
{
	va_list ap;
	int i=0;
	nodet *point=malloc(sizeof(int)+sizeof(nodetoper)+(count_oper-1)*sizeof(nodet*));

	if(!point)	yyerror("out of memory!");
	point->type=tOp;
	point->oper.oper=oper;
	point->oper.count_oper=count_oper;
	
	va_start(ap, count_oper);
	for (i=0;i<count_oper;++i)
		point->oper.nodes[i]=va_arg(ap, nodet*);
	va_end(ap);
	
	return point;
}

static int lable=0;
static int stack_del=0;
static char buf[MAX_NAME_BUF];
static int cstr=0;
static int op;
static int type;

void ex(nodet *p)
{
	int lbl1, lbl2, toper1, toper2;
	if (!p) return ;
	switch(p->type)
	{
	//	printf("\nFFF\n\n");
		char buf2[MAX_NAME_BUF];
	case tCon:
		switch(op)
		{
		case WRITE:
			//printf("\nAAA\n\n");
			if (p->con.type=='1')
			{
				sprintf(buf2, "%s", buf);
				sprintf(buf, "%d%s", p->con.integer, buf2);
				type='1';
			}
			else
			{
				sprintf(buf2, "%s", buf);
				sprintf(buf, "%s%s", p->con.string, buf2);
				type='2';
			}
			break;
		default:
			if (p->con.type=='1')
			{
				printf("\tpush dword %d\n", p->con.integer);
				type='1';
			}
			else
			{
				printf("SECTION .data\n");
				if (strcmp((p->con.string),"%s")==0 || strcmp((p->con.string),"%d")==0)
					printf("\tmsg%d db \"%s\", 0xA, 0\n", cstr, p->con.string);
				else
				printf("\tmsg%d db %s, ' ', 0\n", cstr, p->con.string);
				printf("SECTION .text\n");
				printf("\tpush dword msg%d\n", cstr++);
				type='2';
			}
			break;
		}
		break;
	case tId:
		switch(op)
		{
		case READ:
			{
				a *found=table_find(p->id.name);
				if (found==NULL || found->type==TVAR1)
				{
					printf("\tpush dword _buf\n");
					printf("\tpush dword ifmt\n");
					printf("\tcall scanf\n\tadd esp, 8\n");
					printf("\tmov eax, [_buf]\n\tmov [%s], eax\n", p->id.name);
					type='1';
				}
				else if (found==NULL || found->type==TVAR2)
				{
					printf("\tpush dword _buf\n");
					printf("\tpush dword ifmt\n");
					printf("\tcall scanf\n\tadd esp, 8\n");
					printf("\tmov eax, [_buf]\n\tmov [%s], eax\n", p->id.name);
					type='2';
				}
			}
			break;
		case WRITE:
			{
				a *found=table_find(p->id.name);
				sprintf(buf2, "%s", buf);
				if (found==NULL || found->type==TVAR1)
				{
					sprintf(buf, "%s%s", "%d", buf2);
					printf("\tpush dword [%s]\n", p->id.name);
					stack_del+=4;
				}
				else if (found->type==TVAR2)
				{
					sprintf(buf,"%s%s", "%s", buf2);
					printf("\tpush dword [%s]\n", p->id.name);
					stack_del+=4;
				}
			}
			break;
		case ASSIGN:
			{
				a *found=table_find(p->id.name);
				if(found==NULL || found->type==TVAR1)
				{
					printf("\tpop dword [%s]\n", p->id.name);
				}
				else
				{
					if (type=='2')
						printf("\tpop dword [%s]\n", p->id.name);
					sprintf(buf, "%s", p->id.name);
				}
			}
			break;
		default:
			{
				a *found=table_find(p->id.name);
				if (found==NULL || found->type==TVAR1)	type='1';
				else type='2';
				printf("\tpush dword [%s]\n", p->id.name);
			}
			break;
		}
		break;
	case tOp:
		switch(p->oper.oper)
		{
		case 'I':
			ex(p->oper.nodes[0]);
			printf("\t%s dd 0\n", p->oper.nodes[1]->id.name);
			break;
		case 'S':
			ex(p->oper.nodes[0]);
			printf("SECTION .bss\n");
			printf("\t%s resb 1024\n\n", p->oper.nodes[1]->id.name);
			break; 
		case 'F':	//Для funct
			ex(p->oper.nodes[0]);
			ex(p->oper.nodes[1]);
			break;
		case WHILE:
			printf("\t;цикл WHILE\n");
			op=p->oper.oper;
			printf("L%03d:\n",lbl1=lable++);
			lbl2=lable++;
			ex(p->oper.nodes[0]);
			printf("\tpop eax\n");
			printf("\tcmp eax, 0\n");
			printf("\tje\tL%03d\n",lbl2);
			ex(p->oper.nodes[1]);
			printf("\tjmp\tL%03d\n",lbl1);
			printf("L%03d:\n",lbl2);
			break;
		case IF:
			printf("\t;цикл IF\n");
			op=p->oper.oper;
			lbl1=lable++;
			ex(p->oper.nodes[0]);
			printf("\tpop eax\n");
			printf("\tcmp eax, 0\n");
			printf("\tje\tL%03d\n",lbl1);
			ex(p->oper.nodes[1]);
			printf("L%03d:\n", lbl1);
			break;
		case ELSE:
			printf("\t;цикл ELSE\n");
			op=p->oper.oper;
			lbl1=lable++;
			lbl2=lable++;
			ex(p->oper.nodes[0]);
			printf("\tpop eax\n");
			printf("\tcmp eax, 0\n");
			printf("\tje\tL%03d\n",lbl1);
			ex(p->oper.nodes[1]);
			printf("\tjmp\tL%03d\n", lbl2);
			printf("L%03d:\n", lbl1);
			ex(p->oper.nodes[2]);
			printf("L%03d:\n", lbl2);
			break;
		case WRITE:
			printf("\t;WRITE()\n");
			op=p->oper.oper;
			buf[0]='\0';
			stack_del=4;
			ex(p->oper.nodes[0]);
			printf("SECTION .data\n");
			if (strcmp(buf,"%s")==0 || strcmp(buf,"%d")==0)
				printf("\tmsg%d db \"%s\", 0xA, 0\n", cstr, buf);
			else
				printf("\tmsg%d db %s, ' ', 0\n", cstr, buf);
			printf("SECTION .text\n");
			printf("\tpush dword msg%d\n\tcall printf\n\tadd esp, %d\n", cstr, stack_del);
			++cstr;
			break;
		case READ:
			printf("\t;READ()\n");
			stack_del=0;
			op=p->oper.oper;
			ex(p->oper.nodes[0]);
			break;
		case ASSIGN:
			printf("\t;Присваивание\n");
			op=0;
			ex(p->oper.nodes[1]);
			op=p->oper.oper;
			ex(p->oper.nodes[0]);
			printf("\n");
			break;
		default:
			switch(p->oper.oper)
			{
			case '+':
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Сложение\n");
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tadd eax, ecx\n");
					printf("\tpush eax\n");
					type='1';
				}
				else if (toper1=='2' && toper2=='2')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tadd eax, ecx\n");
					printf("\tpush eax\n");
					type='2';
				}
				break;
			case '-':
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Вычитание\n");
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tsub eax, ecx\n");
					printf("\tpush eax\n");
					type='1';
				}
				break;
			case '*':
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Умножение\n");
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tmul ecx\n");
					printf("\tpush eax\n");
					type='1';
				}
				break;
			case '/':
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Деление\n");
				printf("\txor edx, edx\n");
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tdiv ecx\n");
					printf("\tpush eax\n");
					type='1';
				}
				break;
			case '|':
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Или\n");
				printf("\tpop ecx\n");
				printf("\tpop eax\n");
				printf("\tor eax, ecx\n");
				printf("\tpush eax\n");
				if (toper1=='1' || toper2=='1') type='1';
				break;
			case '&':
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;И\n");
				printf("\tpop ecx\n");
				printf("\tpop eax\n");
				printf("\tand eax, ecx\n");
				printf("\tpush eax\n");
				if (toper1=='1' || toper2=='1') type='1';
				break;
			case '<':
			case '>':
			case '!':
			case '=':
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\tpop ecx\n");
				printf("\tpop eax\n");
				printf("\tmov ebx, 1\n");			
				printf("\tcmp eax, ecx\n");
				switch(p->oper.oper)
				{
					case '<':	printf("\tjb"); break;
					case '>':	printf("\tja"); break;
					case '!':	printf("\tjne"); break;
					case '=':	printf("\tje"); break;
				}
				printf("\tL%03d\n", lable);
				printf("\n\txor ebx, ebx\n");
				printf("L%03d:\n", lable++);
				printf("\tpush ebx\n");
				break;
			default:
				printf("\nFuuu\n");
				ex(p->oper.nodes[0]);
				ex(p->oper.nodes[1]);
				break;
			}
		}
	}
	return ;
}

void freeNode(nodet *point)
{
	int i=0;
	if (!point) return;
	if (point->type==tOp)
		for (i=0;i<(point->oper.count_oper);++i)
			freeNode(point->oper.nodes[i]);
	free(point);
}
