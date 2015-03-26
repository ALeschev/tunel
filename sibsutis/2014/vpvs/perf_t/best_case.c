#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hpctimer.h"

#define TN 64 * 1024

#define PAGE_SIZE 4 * 1024

#define T_SIZE PAGE_SIZE/3

struct Foo
{
	char arr[T_SIZE];
	char arr1[T_SIZE];
	char arr2[T_SIZE];
	char arr3[T_SIZE];
	char arr4[T_SIZE];
	char arr5[T_SIZE];
};

int main(void)
{
	int i, size;
	struct Foo *Bar;
	struct Foo *Bar1;

	double t;

	size = sizeof (struct Foo) * TN;

    if ( (Bar = malloc(size)) == NULL) {
        fprintf(stderr, "No enough memory\n");
        exit(EXIT_FAILURE);
    }

    if ( (Bar1 = malloc(size)) == NULL) {
        fprintf(stderr, "No enough memory\n");
        exit(EXIT_FAILURE);
    }

	memset (Bar, 0 ,sizeof (struct Foo));
	memset (Bar1, 0 ,sizeof (struct Foo));

    t = hpctimer_getwtime();

    int j;
	for (i = 0; i < TN; i++)
	{
		for (j = 0; j < T_SIZE; j++)
		{
			Bar1[i].arr[j] = Bar[i].arr[j];
			Bar1[i].arr1[j] = Bar[i].arr1[j];
			Bar1[i].arr2[j] = Bar[i].arr2[j];
			Bar1[i].arr3[j] = Bar[i].arr3[j];
			Bar1[i].arr4[j] = Bar[i].arr4[j];
			Bar1[i].arr5[j] = Bar[i].arr5[j];
		}
	}

	t = hpctimer_getwtime() - t;

	printf("T_SIZE: %d\nElapsed time (sec.): %.6f\n", T_SIZE, t);

	free(Bar);
	free(Bar1);

	return 0;
}