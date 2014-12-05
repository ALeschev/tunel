#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include "hpctimer.h"

enum th_state { DISABLE = 0, ENABLE };
enum e_state { EMPTY = 0 };

int max = 0;
int *arr = NULL;

struct thread_data
{
    int th_num;
    int start;
    int end;
};

int *arr_init()
{
    if (arr)
        return arr;

    arr = (int *) calloc (1, sizeof (int) * max);
    if (arr == NULL)
        printf ("[%s][%d] Failed to allocated memory\n", __func__, __LINE__);

    return arr;
}

void *pth_step(void *arg)
{
    int i, j;
    // int value;
    struct thread_data *data = (struct thread_data *)arg;

#ifdef DEBUG
    printf("Thread %d started:\n\tstart index: %d\n\tend index: %d\n\n",
           data->th_num, data->start, data->end);
#endif

    for (i = 2; i <= max; i++)
    {
        for (j = data->start; j <= data->end; j++)
        {
            if (arr[j] == EMPTY)
                continue;

            if ((arr[j] != i) && !(arr[j] % i))
                arr[j] = EMPTY;
        }
    }

#ifdef DEBUG
    printf("Thread %d stopped\n", data->th_num);
#endif

    return NULL;
}

void sieve_of_Eratosthenes()
{
    int i;
    pthread_t th_id[2];
    struct thread_data th_data[2];

    for (i = 0; i < max; i++)
        arr[i] = i;

    th_data[0].th_num = 0;
    th_data[0].start = 0;
    th_data[0].end = max/2;
    th_data[1].th_num = 1;
    th_data[1].start = max/2 + 1;
    th_data[1].end = max-1;

    pthread_create (&th_id[0], NULL, pth_step, (void *)&th_data[0]);
    pthread_create (&th_id[1], NULL, pth_step, (void *)&th_data[1]);

    pthread_join (th_id[0], NULL);
    pthread_join (th_id[1], NULL);

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