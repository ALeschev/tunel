#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hpctimer.h"

#define TN 30

#define PAGE_SIZE 4 * 1024

int main(void)
{
	int i, j, kk, k, jj;
	double A[TN][TN] = {{0.0}};
	double B[TN][TN] = {{5.2345452}};
	double t;


    t = hpctimer_getwtime();

	for (kk=0; k < TN; k++)
		for (jj=0; jj < TN; jj++)
			for (i=0; i < TN; i++)
				for (k=kk; k < TN; k++)
				{
					for (j=jj; j < TN; j++)
						A[i][j] += B[k][j];
			}

	t = hpctimer_getwtime() - t;

	printf("Elapsed time (sec.): %.6f\n", t);

	return 0;
}