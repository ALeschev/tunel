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
		double dbl;
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

nodet *creat_double(double value)
{
	nodet *point=malloc(sizeof(double)+sizeof(nodetCon));

	if(!point)	yyerror("out of memory!");
	point->type=tCon;
	point->con.type='3';
	point->con.dbl=value;

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
	static double t_double = 0.0;
	static int prev_oper = 0;
	static char prev_devide_oper[64] = {0};
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
			} else
			if (p->con.type=='2')
			{
				sprintf(buf2, "%s", buf);
				sprintf(buf, "%s%s", p->con.string, buf2);
				type='2';
			} else {
				sprintf(buf2, "%s", buf);
				sprintf(buf, "%f%s", p->con.dbl, buf2);
				type='3';
			}
			break;
		default:
			if (p->con.type=='1')
			{
				printf("\tpush dword %d\n", p->con.integer);
				type='1';
			} else 
			if (p->con.type=='3')
			{
				t_double = p->con.dbl;
				type='3';
			} else {
				printf("SECTION .data\n");
				if (strcmp((p->con.string),"%s")==0 || strcmp((p->con.string),"%d")==0)
					printf("\tmsg%d db \"%s\", 0xA, 0\n", cstr, p->con.string);
				else
					printf("\tmsg%d db %s, ' ', 0\n", cstr, p->con.string);
				printf("SECTION .text\n");
				printf("\tpush dword msg%d\n", cstr++);
				type='2';
				t_double = 99999.99999;
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
				} else
				if (found==NULL || found->type==TVAR2)
				{
					printf("\tpush dword _buf\n");
					printf("\tpush dword ifmt\n");
					printf("\tcall scanf\n\tadd esp, 8\n");
					printf("\tmov eax, [_buf]\n\tmov [%s], eax\n", p->id.name);
					type='2';
				} else
				if (found==NULL || found->type==TVAR3)
				{
					printf("\tpush dword _buf\n\t"
					       "push dword dfmt\n\t"
					       "call scanf\n\tadd esp, 8\n\t"
					       "fld qword [_buf]\n\t"
					       "fstp qword [%s]\n",
					       p->id.name);
					type='3';
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
				else if (found->type==TVAR3)
				{
					sprintf(buf,"%s%s", "%f", buf2);
					printf("\tpush dword [%s+4]\n", p->id.name);
					printf("\tpush dword [%s]\n", p->id.name);
					stack_del+=8;
				}
			}
			break;
		case ASSIGN:
			{
				a *found=table_find(p->id.name);
				if(found==NULL || found->type==TVAR1)
				{
					printf("\tpop dword [%s]\n", p->id.name);
				} else
				if(found==NULL || found->type==TVAR3)
				{
					//printf("\tfstp qword [%s]\n", p->id.name);

					printf ("; t_double %f\n", t_double);

					if (t_double == 99999.99999)
					{
						if ((prev_oper >> 1) == '+' ||
						    (prev_oper >> 1) == '-' ||
						    (prev_oper >> 1) == '*' ||
						    (prev_oper >> 1) == '/')
						{
							printf ("\tfstp qword [%s]\n", p->id.name);
						}
						break;
					} else {
						if ((prev_oper >> 1) == '+' ||
						    (prev_oper >> 1) == '-' ||
						    (prev_oper >> 1) == '*' ||
						    (prev_oper >> 1) == '/')
						{
							printf ("SECTION .data\n\t"
							    "%s_tmp%d dq %f\n"
							    "SECTION .text\n\t"
							    "fld qword [%s_tmp%d]\n\t"
							    "%s\n\t"
							    "fstp qword [%s]\n",
							    p->id.name, cstr, t_double,
							    p->id.name, cstr,
							    ((prev_oper >> 1) == '+')? "fadd st0, st1":
							    ((prev_oper >> 1) == '-')? "fsubp st1, st0":
							    ((prev_oper >> 1) == '*')? "fmul st0, st1":
							    ((prev_oper >> 1) == '/')? "fdiv st1, st0": "ERR",
							    p->id.name);
							++cstr;
							t_double = 99999.99999;
							break;
						}
					}

					printf ("SECTION .data\n\t"
					    "%s_tmp%d dq %f\n"
					    "SECTION .text\n\t"
					    "fld qword [%s_tmp%d]\n\t"
					    "fstp qword [%s]\n",
					    p->id.name, cstr, t_double,
					    p->id.name, cstr, p->id.name);
					++cstr;
					t_double = 99999.99999;
				} else {
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
				else if (found==NULL || found->type==TVAR3)	type='3';
				else type='2';

				if (type == '3')
				{
					if ((prev_oper >> 1) != '/')
					{
						printf("\tfld qword [%s]\n", p->id.name);
						if (prev_oper & 1)
							prev_oper &= ~1;
						else
							prev_oper |= 1;
					} else {
						if (!(prev_oper & 1))
						{
							printf("\tfld qword [%s]\n", p->id.name);
							prev_oper |= 1;
						} else {
							printf("\tfdiv qword [%s]\n", p->id.name);
						}
					}
					strcpy (prev_devide_oper, p->id.name);
				} else {
					printf("\tpush dword [%s]\n", p->id.name);
				}
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
		case 'D':
			ex(p->oper.nodes[0]);
			printf("\t%s dq 0\n\n", p->oper.nodes[1]->id.name);
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
			// if (strcmp(buf,"%f")==0)
			// 	stack_del=12;
			ex(p->oper.nodes[0]);
			printf("SECTION .data\n");
			if (strcmp(buf,"%s")==0 || strcmp(buf,"%d")==0 || strcmp(buf,"%f")==0)
			{
				printf("\tmsg%d db \"%s\", 0xA, 0\n", cstr, buf);
			} else {
				printf("\tmsg%d db %s, ' ', 0\n", cstr, buf);
			}
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
			prev_oper = 99999.99999;
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
				prev_oper = p->oper.oper << 1;
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Сложение [%c][%c]\n", toper1, toper2);
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tadd eax, ecx\n");
					printf("\tpush eax\n");
					type='1';
				} else
				if (toper1=='2' && toper2=='2')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tadd eax, ecx\n");
					printf("\tpush eax\n");
					type='2';
				} else
				if (toper1=='3' && toper2=='3')
				{
					if (!(prev_oper & 1))
						printf("\tfadd st0, st1\n");
					type='3';
				}
				break;
			case '-':
				prev_oper = p->oper.oper << 1;
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Вычитание [%c][%c]\n", toper1, toper2);
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tsub eax, ecx\n");
					printf("\tpush eax\n");
					type='1';
				} else
				if (toper1=='3' && toper2=='3')
				{
					if (!(prev_oper & 1))
						printf("\tfsubp st1, st0\n");
					type='3';
				}
				break;
			case '*':
				prev_oper = p->oper.oper << 1;
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Умножение [%c][%c]\n", toper1, toper2);
				if (toper1=='1' && toper2=='1')
				{
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tmul ecx\n");
					printf("\tpush eax\n");
					type='1';
				} else
				if (toper1=='3' && toper2=='3')
				{
					if (!(prev_oper & 1))
						printf("\tfmul st0, st1\n");
					type='3';
				}
				break;
			case '/':
				prev_oper = p->oper.oper << 1;
				op=p->oper.oper;
				ex(p->oper.nodes[0]);
				toper1=type;
				ex(p->oper.nodes[1]);
				toper2=type;
				printf("\t;Деление [%c][%c]\n", toper1, toper2);
				if (toper1=='1' && toper2=='1')
				{
					printf("\txor edx, edx\n");
					printf("\tpop ecx\n");
					printf("\tpop eax\n");
					printf("\tdiv ecx\n");
					printf("\tpush eax\n");
					type='1';
				} else
				if (toper1=='3' && toper2=='3')
				{
					//printf("\tfdiv st1, st0\n");
					type='3';
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
				prev_oper = 0;
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
