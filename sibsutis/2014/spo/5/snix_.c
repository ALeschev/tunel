#include <stdio.h>

#include "la.h"

list_t *lex_l = NULL;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf ("arg error. filename missing\n");
        return 1;
    }

    lax_analiz(argv[1], &lex_l);

    print_lex();

    list_clear(&lex_l);

    return 0;
}
