#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "hpctimer.h"

enum e_state { EMPTY = 0 };

int max = 0;
int *arr = NULL;

int *arr_init()
{
    if (arr)
        return arr;

    arr = (int *) calloc (1, sizeof (int) * max);
    if (arr == NULL)
        printf ("[%s][%d] Failed to allocated memory\n", __func__, __LINE__);

    return arr;
}

void sieve_of_Eratosthenes()
{
    int i, j;

    for (i = 0; i < max; i++)
        arr[i] = i;

    for (i = 2; i <= max; i++)
    {
        for (j = 0; j < max; j++)
        {
            if (arr[j] == EMPTY)
                continue;

            if ((arr[j] != i) && !(arr[j] % i))
                arr[j] = EMPTY;
        }
    }

    int snum = 0;
    for (i = 0; i < max; i++)
    {
        if (arr[i] != EMPTY)
            snum++;
    }

#ifdef DEBUG
    printf ("Total simple numbers: %d\n", snum);
#endif
}

int main (int argc, char *argv[])
{
    double t;

    if (argc < 2)
    {
        printf ("\nArg error. Use first arg for set max value.\n");
        return 1;
    }

    max = atoi(argv[1]);
    if (max <= 0)
    {
        printf ("\nArg error. Max value must more than 0\n");
        return 1;
    }

    if (arr_init() == NULL)
        return 1;

    t = hpctimer_wtime();

    sieve_of_Eratosthenes ();

    t = hpctimer_wtime() - t;
    printf("Elapsed time (sec.): %.6f\n", t);

   if (arr)
       free (arr);

    return 0;
}