/*
 * loop.c:
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hpctimer.h"

enum { n = 64 * 1024 * 1024 };

int main()
{
    int *v, i, sum = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    int s5 = 0, s6 = 0, s7 = 0, s8 = 0;
    int s9 = 0, s10 = 0, s11 = 0, s12 = 0;
    int s13 = 0, s14 = 0, s15 = 0, s16 = 0;
    int size;
    double t;

    size = sizeof(*v) * n;

    if ( (v = malloc(size)) == NULL) {
        fprintf(stderr, "No enough memory\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n; i ++)
        v[i] = 1;

    t = hpctimer_wtime();
    for (i = 0; i < n; i += 8)
    {
        s1 += v[i];
        s2 += v[i + 1];
        s3 += v[i + 2];
        s4 += v[i + 3];
        s5 += v[i + 4];
        s6 += v[i + 5];
        s7 += v[i + 6];
        s8 += v[i + 7];
        s9 += v[i + 8];
        s10 += v[i + 9];
        s11 += v[i + 10];
        s12 += v[i + 11];
        s13 += v[i + 12];
        s14 += v[i + 13];
        s15 += v[i + 14];
        s16 += v[i + 15];
    }
    sum += s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 +
           s9 + s10 + s11 + s12 + s13 + s14 + s15 + s16;
    t = hpctimer_wtime() - t;

    printf("Sum = %d\n", sum);
    printf("Elapsed time (sec.): %.6f\n", t);

    free(v);
    return 0;
}
