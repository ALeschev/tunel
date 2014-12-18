#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define NUM_OF_KW 3

char *keywords[NUM_OF_KW] = {"if", "then", "else"};

enum state {H, DLM, NM, ID, ASGN, ERR} CS;
enum tok_names {KWORD, IDENT, NUM, OPER, DELIM};

struct token
{
    enum tok_names token_name;
    char *token_value;
};

struct lexeme_table
{
    struct token tok;
    struct lexeme_table *next;
};

struct lexeme_table *lt = NULL;
struct lexeme_table *lt_head = NULL;

int is_keyword(char *id)
{
    int i;
    for (i = 0; i < NUM_OF_KW;  i++)
    {
	if (!strcmp(keywords[i], id))
	    return 1;
    }
    return 0;
}

int add_token(struct token *tok)
{
    lt = lt_head;
    if (lt == NULL)
    {
	lt = (struct lexeme_table*) malloc(sizeof(struct lexeme_table));
	lt->tok.token_name = tok->token_name;

	if ((lt->tok.token_value = (char *)malloc(strlen(tok->token_value))) == NULL)
	{
	    printf("error add_token\n");
	    return -1;
	}
	strcpy(lt->tok.token_value, tok->token_value);

	lt->next = NULL;
	lt_head = lt;
    } else {
	for (lt = lt_head; lt->next != NULL; lt = lt->next);
		lt->next = (struct lexeme_table *) malloc(sizeof(struct lexeme_table));
		lt = lt->next;
		lt->tok.token_name = tok->token_name;

		if((lt->tok.token_value = (char *) malloc(strlen(tok->token_value))) == NULL) {
			printf("error add_token\n");
			return -1;
		}
		strcpy(lt->tok.token_value, tok->token_value);
		lt->next = NULL;
	}
	return 0;
}

int lexer(char *filename)
{
    FILE *f;
    int c, err_symbol;
    int size, ch;
    struct token tok;
    char buf[256];

    if((f = fopen(filename, "r")) == NULL)
    {
	printf("No enough argements %s\n", filename);
	return -1;
    }

    CS = H;
    c = fgetc(f);

    while(!feof(f))
    {
	switch(CS)
	{
	    case H:
		while (c == ' ' || c == '\n' || c == '\t')
		{
		    c = fgetc(f);
		}
		if (isdigit(c)){
		    CS = NM;
		}else if (isalpha(c)) {
		    CS = ID;
		}else if (c == ':') {
			CS = ASGN;
		}else {CS = DLM;}
		break;

	    case ASGN:
		ch = c;
		c = fgetc(f);
		if(c == '=')
		{
		    tok.token_name = OPER;
		    if ((tok.token_value = (char *)malloc(sizeof(char) * 2)) == NULL)
		    {
			printf("error lexer\n");
			return -1;
		    }
		    strcpy(tok.token_value, ":=");
		    add_token(&tok);
		    c  = fgetc(f);
		    CS = H;
		} else {
//
		    err_symbol = ch;
		    CS = ERR;
		}
		break;

	    case DLM:
		if (c == '(' || c == ')' || c == ';')
		{
		    tok.token_name = DELIM;
		    if ((tok.token_value = (char *)malloc(sizeof(char))) == NULL)
		    {
			printf("error lexer");
			return -1;
		    }
		    sprintf(tok.token_value, "%c", c);
		    add_token(&tok);
		    c = fgetc(f);
		    CS = H;
		}
		else if(c == '<' || c == '>' || c == '=' || c == '+' || c =='-')
		{
		    tok.token_name = OPER;
		    if ((tok.token_value = (char *)malloc(sizeof(char))) == NULL)
		    {
			printf("error lexer");
			return -1;
		    }
		    sprintf(tok.token_value, "%c", c);
		    add_token(&tok);
		    c = fgetc(f);
		    CS = H;
		}
		else {
		    err_symbol = c;
		    c = fgetc(f);
		    CS = ERR;
		}
		break;

	    case ID:
		size = 0;
		do {
		    buf[size] = c;
		    size++;
		    c = fgetc(f);
		}
		while (isalpha(c) || isdigit(c) || c == '_');
		buf[size] = '\0';
		if(is_keyword(buf))
		{
		    tok.token_name = KWORD;
		} else {
		    tok.token_name = IDENT;
		}

		if ((tok.token_value = (char *)malloc(strlen(buf))) == NULL)
		{
		    printf("error lexer");
		    return -1;
		}
		strcpy(tok.token_value, buf);
		add_token(&tok);
		CS = H;
		break;

	    case NM:
		size = 0;
		do {
		    buf[size] = c;
		    size++;
		    c = fgetc(f);
		}while (isdigit(c) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E');
		buf[size] = '\0';
		if((isspace(c)) || c == '<' || c == '>' || c == '=' || c == '(' || c == ')' || 
		    c == ';' || c == '-' || c == '+')
		{
		    tok.token_name = NUM;
		    if ((tok.token_value = (char *)malloc(strlen(buf))) == NULL)
		    {
			printf("error lexer");
			return -1;
		    }
		    strcpy(tok.token_value, buf);
		    add_token(&tok);
		    CS = H;
		}
		else {
		    err_symbol = c;
		    CS = ERR;
		}
		break;
	    case ERR:
		printf("Unknown character: %c\n", err_symbol);
		CS = H;
		break;
	}
    }
    fclose(f);
    return 0;
}

int main()
{
	lexer("1");

	for(lt = lt_head; lt != NULL; lt = lt->next) {
		switch (lt->tok.token_name) {
			case KWORD:
				printf("KEYWORDS: ");
				break;
			case IDENT:
				printf("IDENTIFIER: ");
				break;
			case NUM:
				printf("NUMBER: ");
				break;
			case OPER:
				printf("OPERATION: ");
				break;
			case DELIM:
				printf("DELIMITER: ");
				break;
		}
		printf("%s\n", lt->tok.token_value);
	}
    return 0;
}







