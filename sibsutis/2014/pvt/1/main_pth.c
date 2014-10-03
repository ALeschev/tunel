#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

enum e_state { EMPTY = 0 };

int max = 0;
int *arr = NULL;

int done = 0;
int s_count = 0;

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
    int i;
    int step = *((int *)arg);
    int value = arr[step];

    for (i = step; i <= max; i++)
    {
        if (arr[i] == EMPTY)
            continue;

        if ((arr[i] != value) && !(arr[i] % value))
        {
            arr[i] = EMPTY;
            s_count++;
        }
    }

    done++;

    return NULL;
}

/*sieve of Eratosthenes*/
int *sieve_of_Eratosthenes()
{
    int i;
    int value;
    int *s_num = NULL;
    pthread_t *thread_id = NULL;

    thread_id = (pthread_t *)calloc(1, sizeof (pthread_t) * max);
    if (thread_id == NULL)
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

        pthread_create (&thread_id[i-3], NULL, pth_step, (void*)&i);
    }

    /* check this */
    while (done < (max - 3))
        usleep(100000);

    s_count = max - s_count - 1;

    printf ("Total simple numbers: %d\n", s_count);

#ifdef DEBUG
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
#endif

    free (thread_id);

    return s_num;
}

int main (int argc, char *argv[])
{
    int i;

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

#ifdef DEBUG
    int *s_num = NULL;
    s_num = sieve_of_Eratosthenes ();
    if (s_num == NULL)
    {
        printf ("\nFailed to get simple numbers :(\n");
        return 1;
    }

    for (i = 0; i < s_count; i++)
        printf ("%d ", s_num[i]);
    printf ("\n");

    /* use s_nums */

    if (s_num)
        free (s_num);
#else
    sieve_of_Eratosthenes ();
#endif

    if (arr)
        free (arr);

    return 0;
}