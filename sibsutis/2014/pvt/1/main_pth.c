#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

enum th_state { DISABLE = 0, ENABLE };
enum e_state { EMPTY = 0 };

int max = 0;
int *arr = NULL;

int s_count = 0;

struct thread
{
    int state;
    pthread_t thread_id;
    /* data */
};

int *arr_init()
{
    if (arr)
        return arr;

    arr = (int *) calloc (1, sizeof (int) * max);
    if (arr == NULL)
    {
        printf ("[%s][%d] Failed to allocated memory\n", __func__, __LINE__);
        return NULL;
    }

    return arr;
}

void *pth_step(void *arg)
{
    int i, j;
    int step = *((int *)arg);
    int value = arr[step];

    for (i = 0; i < max; i++)
    {
        if (arr[i] != EMPTY)
            printf ("%d ", arr[i]);
    }

    for (i = step; i <= max; i+=2)
    {
        value = arr[i - 2];

        if (value == EMPTY)
            continue;

        printf ("\ni = %d\n", i);

        for (j = i; j <= max; j+=2)
        {
            int step = j;

            if (arr[step] == EMPTY)
                continue;

            if ((arr[step] != value) && !(arr[step] % value))
            {
                printf("arr[%d][%d] value[%d]: %d empty\n",step, arr[step], value, arr[step]);
                arr[step] = EMPTY;
                s_count++;
            }
        }
    }

    return NULL;
}

/*sieve of Eratosthenes*/
void sieve_of_Eratosthenes()
{
    int i;//, j;
    struct thread threads[2];

    for (i = 1; i < max; i++)
        arr[i - 1] = i;

    for (i = 0; i < max; i++)
    {
        if (arr[i] != EMPTY)
            printf ("%d ", arr[i]);
    }

    /*
        i = 3.
        cause: "first elem always equal 1, just skip it"
    */

    i = 4;
    pthread_create (&threads[0].thread_id, NULL, pth_step, (void *)&i);

    // j = 3;
    // pthread_create (&threads[1].thread_id, NULL, pth_step, (void *)&j);

    pthread_join (threads[0].thread_id, NULL);
    // pthread_join (threads[1].thread_id, NULL);

    s_count = max - s_count - 1;

    printf ("Total simple numbers: %d\n", s_count);

for (i = 0; i < max; i++)
    {
        if (arr[i] != EMPTY)
            printf ("%d ", arr[i]);
    }    
}

int main (int argc, char *argv[])
{
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

    sieve_of_Eratosthenes ();

    if (arr)
        free (arr);

    return 0;
}