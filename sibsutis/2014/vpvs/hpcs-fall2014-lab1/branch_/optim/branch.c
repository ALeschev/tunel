/*
 * branch.c:
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "hpctimer.h"

enum { 
    n = 100000,
    nreps = 20 
};

double x[n], y[n], z[n];

void blend_map(double *dest, double *a, double *b, int size, int blend)
{
    int i = 0;
    double k = 0.0;

    if (blend == 0)
    {
        for (i = 0; i < size; i += 2)
        {
            dest[i] = b[i];
            dest[i + 1] = b[i + 1];
        }
    } else
    if (blend == 255)
    {
        for (i = 0; i < size; i += 2)
        {
            dest[i] = a[i];
            dest[i + 1] = a[i + 1];
        }
    } else {
        k = (255 - blend) / 256.0;
        for (i = 0; i < size; i += 2)
        {
            dest[i] = a[i] * blend + b[i] * k;
            dest[i + 1] = a[i + 1] * blend + b[i + 1] * k;
        }
    }
}

int main(int argc, char *argv[])
{
    double tfirst, t;
    int i;
    int repeat_count = 1;
    int current_repeat = 0;
    double average = 0.0;

    if (argc >= 2)
    {
        repeat_count = atoi(argv[1]);
    }

    /* First run: warmup */
    tfirst = hpctimer_wtime();
    blend_map(z, x, y, n, 0);
    tfirst = hpctimer_wtime() - tfirst;

        srand(time(NULL));

    while (current_repeat < repeat_count)
    {
        /* Measures */
        t = hpctimer_wtime();
        for (i = 0; i < nreps; i++) {
            blend_map(z, x, y, n, rand());
        }
        t = (hpctimer_wtime() - t) / nreps;

        //printf("First run (sec.): %.6f\n", tfirst);
        //printf("Mean of %d runs (sec.): %.6f\n", nreps, t);
        average += t;

        current_repeat++;
    }

    average /= repeat_count;

    printf("%.6f\n", average);

    return 0;
}
