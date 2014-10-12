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
    int s1 = 0, s2 = 0;
    int size;
    double t;

    size = sizeof(*v) * n;

    if ( (v = malloc(size)) == NULL) {
        fprintf(stderr, "No enough memory\n");
        exit(EXIT_FAILURE);
    }

#if 1
    for (i = 0; i < n; i += 2)
    {
        v[i] = 1;
        v[i + 1] = 1;
    }

    t = hpctimer_wtime();
    for (i = 0; i < n; i += 2)
    {
        s1 += v[i];
        s2 += v[i + 1];
    }
    sum += s1 + s2;
    t = hpctimer_wtime() - t;
#else
    memset (v, 1, size);
    sum = size/sizeof(int);
#endif

    printf("Sum = %d\n", sum);
    printf("Elapsed time (sec.): %.6f\n", t);

    free(v);
    return 0;
}
