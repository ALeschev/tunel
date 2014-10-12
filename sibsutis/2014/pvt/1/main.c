#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

enum e_state { EMPTY = 0 };

int *sieve_of_Eratosthenes(int max)
{
    int i, j;
    int value;
    int s_count = 0;

    int *s_num = NULL;
    int *arr = NULL;

    arr = (int *) calloc (1, sizeof (int) * max);
    if (arr == NULL)
    {
        printf ("[%s][%d] Failed to allocated memory\n", __func__, __LINE__);
        return NULL;
    }

    for (i = 1; i < max; i++)
        arr[i - 1] = i;

    /*
        i = 3.
        cause: "first elem always equal 1, just skip it"
    */
    for (i = 3; i <= max; i++)
    {
        value = arr[i - 2];

        if (value == EMPTY)
            continue;

        for (j = i; j <= max; j++)
        {
            int step = j;

            if (arr[step] == EMPTY)
                continue;

            if ((arr[step] != value) && !(arr[step] % value))
            {
                arr[step] = EMPTY;
                s_count++;
            }
        }
    }

    s_count = max - s_count - 1;

    printf ("Total simple numbers: %d\n", s_count);

    // int k;
    // for (k = 0; k < max; k++)
    //     printf ("%d ", arr[k]);
    // printf ("\n");

    s_num = (int *) calloc (1, sizeof (int) * s_count);
    if (s_num == NULL)
    {
        printf ("[%s][%d] Failed to allocated memory\n", __func__, __LINE__);
        return NULL;
    }

    for (i = 0; i < max; i++)
    {
        if (arr[i] != EMPTY)
            s_num[i] = arr[i];
    }

    free (arr);

    return s_num;
}

int main (int argc, char *argv[])
{
    int input = 0;

    if (argc < 2)
    {
        printf ("\nArg error. Use first arg for set max value.\n");
        return 1;
    }

    input = atoi(argv[1]);
    if (input <= 0)
    {
        printf ("\nArg error. Max value must be > 0\n");
        return 1;
    }

    int *s_num = NULL;

    s_num = sieve_of_Eratosthenes (input);
    if (s_num == NULL)
    {
        printf ("\nFailed to get simple numbers :(\n");
        return 1;
    }

    /* use s_nums */
    if (s_num)
        free (s_num);

    return 0;
}