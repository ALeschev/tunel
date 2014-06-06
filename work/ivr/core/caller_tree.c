#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "ivr_caller_tree.h"
#include "ivr_proc.h"
#include "header.h"

//typedef unsigned char uint8_t;
typedef int ElementType;
typedef struct AvlNode *Position;
typedef struct AvlNode *AvlTree;
typedef struct AvlData ElementData;

struct AvlData
{
	caller_t caller;

	int ditits_needed;
	//uint8_t digits[MAX_DIG_COLLECT];
	char digits[MAX_DIG_COLLECT];
};

struct AvlNode {
	ElementType Element;

	AvlTree Left;
	AvlTree Right;
	int Height;

	ElementData Data;
};

static AvlTree caller_tree = NULL;
static int UserProc_cnt = 0;

static AvlTree MakeEmpty( AvlTree T );
static Position Find( ElementType X, AvlTree T );
static AvlTree Insert( ElementType X, ElementData D, AvlTree T );
static AvlTree DeleteElem ( AvlTree T, ElementType X );
// static ElementData RetrieveData( Position P );
static ElementType Retrieve( Position P );
static void printTree(AvlTree T, int l);

/*------------------------------------------------------------------------------*/
void tree_print ()
{
	printTree (caller_tree, 0);
}

/*------------------------------------------------------------------------------*/
void tree_add_caller (int callref, caller_t *caller)
{
	ElementData D;
	memset (&D, 0, sizeof (D));

	D.caller.state = caller->state;
	D.caller.ivr_idx = caller->ivr_idx;
	D.caller.length = caller->length;
	memcpy(&D.caller.sym, caller->sym, MAX_DIG_COLLECT);

	caller_tree = Insert (callref, D, caller_tree);

	UserProc_cnt++;
}

/*------------------------------------------------------------------------------*/
ElementType tree_find_caller (int callref)
{
	Position c_call = Find(callref, caller_tree);

	if(c_call)
		return Retrieve(c_call);

	return 0;
}

/*------------------------------------------------------------------------------*/
int tree_get_state (int callref)
{
	Position c_call = Find(callref, caller_tree);

	if(c_call)
		return c_call->Data.caller.state;

	return -2;
}
/*------------------------------------------------------------------------------*/
int tree_get_ivr_idx (int callref)
{
	Position c_call = Find(callref, caller_tree);

	if(c_call)
		return c_call->Data.caller.ivr_idx;

	return -1;
}

/*------------------------------------------------------------------------------*/
int tree_set_state (int callref, int state)
{
	Position c_call = Find(callref, caller_tree);

	if(c_call)
	{
		c_call->Data.caller.state = state;
		return 0;
	}

	return -1;
}

int tree_set_digit (int callref, uint8_t dig)
{
	Position c_call = Find(callref, caller_tree);

	if (!c_call)
		return 2;

	if (dig == 10)
		dig -= 10;

	printf ("tree_set_digit+\n");

	char t_str[1] = {0};
	sprintf (t_str, "%d", dig);

	// printf ("callref %04x c_call->Data.ditits_needed %d\n", callref, c_call->Data.ditits_needed);

	if (c_call->Data.ditits_needed > 0)
	{
		strcat (c_call->Data.digits, t_str);
		//c_call->Data.digits[c_call->Data.ditits_needed-1] = dig;
		c_call->Data.ditits_needed--;
	}

	if (!c_call->Data.ditits_needed && (c_call->Data.caller.state == CMD_COLLECT_DIG))
		return 0; /*this was last one. that's enough*/

	return 1; /*need moooore digits!*/
}

/*------------------------------------------------------------------------------*/

int tree_set_gitits_need (int callref, int num)
{
	Position c_call = Find(callref, caller_tree);

	if (!c_call)
		return -1;

	if (num > MAX_DIG_COLLECT)
		num = MAX_DIG_COLLECT;

	memset (c_call->Data.digits, 0, sizeof (uint8_t) * MAX_DIG_COLLECT);

	c_call->Data.ditits_needed = num;

	return 0;
}

/*------------------------------------------------------------------------------*/

char *tree_get_digits (int callref)
{
	// static char s_digits[MAX_DIG_COLLECT] = {0};
	// char t_str[1] = {0};
	// int i;

	Position c_call = Find(callref, caller_tree);

	if (!c_call)
		return NULL;

	// for (i = 0; i < MAX_DIG_COLLECT; i++)
	// {
	// 	sprintf (t_str, "%d", c_call->Data.digits[i]);
	// 	strcat (s_digits, t_str);
	// }

	//return s_digits;

	return c_call->Data.digits;
}

/*------------------------------------------------------------------------------*/

char *tree_get_caller_sym (int callref)
{
	Position c_call = Find(callref, caller_tree);

	if (!c_call)
		return NULL;

	return (char *)c_call->Data.caller.sym;
}

/*------------------------------------------------------------------------------*/

void tree_clean_all (void)
{
	caller_tree = MakeEmpty (caller_tree);
	UserProc_cnt = 0;
}

/*------------------------------------------------------------------------------*/

int tree_get_caller_cnt (void)
{
	return UserProc_cnt;
}

/*------------------------------------------------------------------------------*/

void tree_clean (void)
{
	caller_tree = MakeEmpty (caller_tree);
}

/*------------------------------------------------------------------------------*/

void tree_del_caller (int callref)
{
	caller_tree = DeleteElem (caller_tree, callref);

	if (UserProc_cnt > 0)
		UserProc_cnt--;
}

/*------------------------------------------------------------------------------*/

static AvlTree MakeEmpty( AvlTree T )
{
	if( T != NULL ){
		MakeEmpty( T->Left );
		MakeEmpty( T->Right );
		xfree( T );
	}
	return NULL;
}

static Position Find( ElementType X, AvlTree T )
{
	if( T == NULL ) return NULL;
	if( X < T->Element )
		return Find( X, T->Left );
	else
	if( X > T->Element )
		return Find( X, T->Right );
	else
		return T;
}

static int Height( Position P )
{
	return ( P == NULL ) ? -1 : P->Height;
}

static int Max( int Lhs, int Rhs )
{
  return Lhs > Rhs ? Lhs : Rhs;
}

static Position SingleRotateWithLeft( Position K2 )
{
	Position K1;
	K1 = K2->Left;
	K2->Left = K1->Right;
	K1->Right = K2;
	K2->Height = Max(Height(K2->Left), Height(K2->Right)) + 1;
	K1->Height = Max( Height( K1->Left ), K2->Height ) + 1;
	return K1;
}

static Position SingleRotateWithRight( Position K1 )
{
	Position K2;
	K2 = K1->Right;
	K1->Right = K2->Left;
	K2->Left = K1;
	K1->Height = Max(Height(K1->Left), Height(K1->Right)) + 1;
	K2->Height = Max( Height( K2->Right ), K1->Height ) + 1;
	return K2;
}

static Position DoubleRotateWithLeft( Position K3 )
{
	K3->Left = SingleRotateWithRight( K3->Left );

	return SingleRotateWithLeft( K3 );
}

static Position DoubleRotateWithRight( Position K1 )
{
	K1->Right = SingleRotateWithLeft( K1->Right );

	return SingleRotateWithRight( K1 );
}

static AvlTree Insert( ElementType X, ElementData D, AvlTree T )
{
	if( T == NULL ){
		T = (AvlTree) malloc(sizeof(struct AvlNode));
		if( T )
		{
			T->Element = X;
			T->Data = D;
			T->Height = 0;
			T->Left = T->Right = NULL;
		}
	}
	else if( X < T->Element ) {
		T->Left = Insert( X, D, T->Left );
		if( Height( T->Left ) - Height( T->Right ) == 2 )
		{
			if( X < T->Left->Element )
				T = SingleRotateWithLeft( T );
			else
				T = DoubleRotateWithLeft( T );
		}
	}
	else if( X > T->Element ) {
		T->Right = Insert( X, D, T->Right );
		if( Height( T->Right ) - Height( T->Left ) == 2 )
		{
			if( X > T->Right->Element )
				T = SingleRotateWithRight( T );
			else
				T = DoubleRotateWithRight( T );
		}
	}
	else if( X == T->Element )
	{
		T->Data = D;

		return T;
	}

	T->Height = Max(Height(T->Left), Height(T->Right)) + 1;

	return T;
}

static ElementType Retrieve( Position P )
{
	return P->Element;
}

// static ElementData RetrieveData( Position P )
// {
// 	return P->Data;
// }

static AvlTree DeleteElem ( AvlTree T, ElementType X )
{
	AvlTree P, v;

	if (!T)
		return T;
	else if (X < T->Element)
		T->Left = DeleteElem(T->Left, X);
	else if (X > T->Element)
		T->Right = DeleteElem(T->Right, X);
	else
	{
		P = T;
		if (!T->Right) T = T->Left;
		else if (!T->Left)
			T = T->Right;
		else
		{
			v = T->Left;
			if (v->Right)
			{
				while (v->Right->Right)
					v = v->Right;

				T->Element = v->Right->Element;
				P = v->Right;
				v->Right = v->Right->Left;
			} else {
				T->Element = v->Element;
				P = v;
				T->Left = T->Left->Left;
			}
		}
		xfree(P);
	}

	return T;
}

static void printTree(AvlTree T, int l)
{
	int i;
	if ( T != NULL ) {
		printTree(T->Right, l+1);
		for (i=0; i < l; i++) printf("    ");
		printf ("%04x", Retrieve ( T ));
		printTree(T->Left, l+1);
	}
	else printf("\n");
}