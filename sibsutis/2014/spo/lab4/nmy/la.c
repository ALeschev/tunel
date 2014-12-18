#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "list.h"

#define NUM_OF_KW 3

char *keywords[NUM_OF_KW] = {"if", "then", "else"};

list_t *lex_list = NULL;

enum state
{
    H,
    P_KEYWORDS,
    P_DELIMITER,
    P_NUMBER,
    P_OPERATION,
    ERR
};

enum entry_names
{
    KEYWORDS,
    IDENTIFIER,
    NUMBER,
    OPERATION,
    DELIMITER,
    ERROR
};

char *state_str(int state)
{
    switch (state)
    {
        case H: return "H";
        case P_KEYWORDS: return "Keywords";
        case P_DELIMITER: return "Delimiter";
        case P_NUMBER: return "Number";
        case P_OPERATION: return "Operation";
        case ERR: return "Err";
    }

    return "Undef";
}

char *entry_name_str(int entry_key)
{
    switch (entry_key)
    {
        case KEYWORDS: return "Keywords";
        case IDENTIFIER: return "Identifier";
        case NUMBER: return "Number";
        case OPERATION: return "Operation";
        case DELIMITER: return "Delimiter";
        case ERROR: return "Error";
    }

    return "Undef";
}

void print_lex()
{
    list_t *p_list;
    entry_t *p_entry;
    p_list = lex_list;

    while(p_list != NULL)
    {
        p_entry = &p_list->entry;

        printf("%s: '%s'\n", entry_name_str(p_entry->key), p_entry->value);

        p_list = p_list->next;
    }
}

void set_state(int *old, int state)
{
    // printf ("state changed: %s -> %s\n", state_str(*old), state_str(state));

    *old = state;
}

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

void file_skip_spaces(FILE **fd, int *ch)
{
    while (*ch == ' '  ||
           *ch == '\n' ||
           *ch == '\t')
    {
        *ch = fgetc(*fd);
    }
}

void set_start_state(int *state, int ch)
{
    if (isdigit(ch))
    {
        set_state(state, P_NUMBER);
    } else
    if (isalpha(ch))
    {
        set_state(state, P_KEYWORDS);
    } else
    if (ch == ':')
    {
        set_state(state, P_OPERATION);
    } else {
        set_state(state, P_DELIMITER);
    }
}

void process_keyword(int *state, FILE **fd, int ch, int *err)
{
    char buf[256];
    int size = 0;
    entry_t entry;

    do {
        buf[size] = ch;
        size++;
        ch = fgetc(*fd);
    }
    while (isalpha(ch) || isdigit(ch) || ch == '_');

    buf[size] = '\0';
    if(is_keyword(buf))
    {
        entry.key = KEYWORDS;
    } else {
        entry.key = IDENTIFIER;
    }

    strcpy(entry.value, buf);

    list_add(&lex_list, &entry);

    set_state(state, H);
}

void process_delimiter(int *state, FILE **fd, int ch, int *err)
{
    entry_t entry;

    if (ch == '(' || ch == ')' || ch == ';')
    {
        entry.key = DELIMITER;
        entry.value[0] = ch;

        list_add(&lex_list, &entry);

        set_state(state, H);
    } else
    if(ch == '<' || ch == '>' || ch == '=' || ch == '+' || ch =='-')
    {
        entry.key = OPERATION;
        entry.value[0] = ch;

        list_add(&lex_list, &entry);

        set_state(state, H);
    } else {
        *err = ch;
        set_state(state, ERR);
    }
}

void process_number(int *state, FILE **fd, int ch, int *err)
{
    entry_t entry;
    char buf[256];
    int size = 0;

    do {
        buf[size] = ch;
        size++;
        ch = fgetc(*fd);
    }while (isdigit(ch) || (ch >= 'a' && ch <= 'f'));

    buf[size] = '\0';
    if((isspace(ch)) || ch == '<' || ch == '>' || ch == '=' ||
        ch == '(' || ch == ')' || ch == ';' || ch == '-' || ch == '+')
    {
        fseek(*fd, ftell(*fd) - 1, SEEK_SET);

        entry.key = NUMBER;
        strcpy(entry.value, buf);

        list_add(&lex_list, &entry);

        set_state(state, H);
    }
    else {
        *err = ch;
        set_state(state, ERR);
    }
}

void process_operation(int *state, FILE **fd, int ch, int *err)
{
    int prev_ch = ch;
    entry_t entry;

    ch = fgetc(*fd);
    if(ch == '=')
    {
        entry.key = OPERATION;
        strcpy(entry.value, ":=");

        list_add(&lex_list, &entry);

        *state = H;
    } else {
        *err = prev_ch;
        set_state(state, ERR);
    }
}

void process_err(int *state, FILE **fd, int ch)
{
    entry_t entry;

    entry.key = ERROR;
    entry.value[0] = ch;

    list_add(&lex_list, &entry);

    set_state(state, H);
}

int lax_analiz(char *filename)
{
    FILE *f;
    int c, err_symbol;
    int state;

    if((f = fopen(filename, "r")) == NULL)
    {
        printf("Failed to open '%s'\n", filename);
        return 1;
    }

    state = H;

    while(!feof(f))
    {
        switch(state)
        {
            case H:
                c = fgetc(f);
                file_skip_spaces(&f, &c);
                set_start_state(&state, c);
            break;

            case P_KEYWORDS:
                process_keyword(&state, &f, c, &err_symbol);
            break;

            case P_DELIMITER:
                process_delimiter(&state, &f, c, &err_symbol);
            break;

            case P_NUMBER:
                process_number(&state, &f, c, &err_symbol);
            break;

            case P_OPERATION:
                process_operation(&state, &f, c, &err_symbol);
            break;

            case ERR:
                process_err(&state, &f, err_symbol);
            break;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf ("arg error. filename missing\n");
        return 1;
    }

    lax_analiz(argv[1]);

    print_lex();

    list_clear(&lex_list);

    return 0;
}