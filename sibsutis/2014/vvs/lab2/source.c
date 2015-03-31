#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#include "hpctimer.h"

void buildmatrix(float *c, int n, float c1, float c2, float c3);
void printmatrix(float *c, int n);
void braun(const float *c, int n, float e);
//void braun_long(float c[17][17], int n, float e);

int main(){
	FILE *f;
	float c[3] = { 1.0f, 2.0f, 3.0f };
	float *matrix;
	int n;

	f = fopen("res.res","w");

	for(n = 10; n <= 16; n++ )
	{
		matrix = malloc(sizeof(float) * (n + 1) * (n + 1));
		if (!matrix)
		{
			printf ("Failed to allocate: %s\n", strerror(errno));
			return 1;
		}

		fprintf(f, "%d ", n);

		buildmatrix(matrix, n, c[0], c[1], c[2]);

		braun(matrix, n, 0.01);

		fprintf(f, "\n");

		free(matrix);
	}

	fclose(f);

	return 0;
}

void buildmatrix(float *c, int n, float c1, float c2, float c3){
	int i, j;
	for (i = 0; i <= n; i++){
		for (j = 0; j <= n; j++){
			if (i >= j){
				*(c + i * (n + 1) + j) = j * c1 + (i - j) * c2;
			}
			else{
				*(c + i * (n + 1) + j) = j * c1 + (j - i) * c3-j;
			}
		}	
	}
}

void printmatrix(float *c, int n){
	int i, j;
	for (i = 0; i <= n; i++){
		for (j = 0; j <= n; j++)
			printf("%6.2f",*(c + i * (n + 1) + j));
	printf("\n");
	}
}

void braun(const float *c, int n, float e){
	float X[n + 1], Y[n + 1];
	float V, cond=1;
	int i, j, l=0;
	int counterx[n + 1], countery[n + 1];
	int I, J;
	float t;
	float maxmin, minmax;
	t = hpctimer_wtime();
	for (i=0;i<n+1;i++)
	{
	X[i] = FLT_MAX;
	Y[i] = 0;
	counterx[i] = 0;
	countery[i] = 0; 
	}
	for (i = 0; i <= n; i++){
		for (j = 0; j < n; j++){
			if (*(c + i * (n + 1) + j) < X[i]){
				X[i] = *(c + i * (n + 1) + j);
			}
			if (*(c + i + (n + 1) * j) > Y[i]){
				Y[i] = *(c + i + (n + 1) * j);
			}
		}
	}
	while (cond > e){
		l++;
		maxmin = 0;
		minmax = INT_MAX;
		for (i = 0; i <= n; i++){
			if (X[i] > maxmin){
				maxmin = X[i];
				I = i;					
			}
			if (Y[i] < minmax){
				minmax = Y[i];
				J = i;
			}		
		}
		counterx[I]++;
		countery[J]++;
		for (i = 0; i <= n; i++){
			X[i] += (*(c + J + (n + 1) * i));
			Y[i] += (*(c + I * (n + 1) + i));
			//X[i] /= l;
			//Y[i] /= l;
		}
		cond = /*(1 / l) * */fabs(maxmin - minmax) / l;
	}
	t=hpctimer_wtime()-t;
	fprintf(f, "%f ",t);
	//printf("%f %f\n",maxmin/l,minmax/l);
	printf("Вероятности выборов стратегий ВЦ:\n{");
	for(i=0; i <= n; i++){
		X[i] = (float)counterx[i] / l;
		Y[i] = (float)countery[i] / l;
		printf("%f, ",X[i]);
	}
	printf("}\n");
	printf("Вероятности выборов стратегий клиента:\n{");
	for(i=0; i <= n; i++){
		printf("%f, ",Y[i]);
	}
	printf("}\n");
	V = (float)(maxmin - minmax) / (2.0 * l);
	printf("Количество итераций: %d, цена игры: %1.4f\n", l,V);
}

/*void braun_long(float c[11][11], int n, float e){
	int X[n + 1], Y[n + 1];
	float V,cond=1;
	int i, j, l=0;
	int I, J, maxmin, minmax;
	for (i = 0; i <=n; i++){
			for(j = 0; j<=n; j++){
				if (c[i][j] < X[i]){
					X[i] = c[i][j];
				}
			}		
		}
		for (i = 0; i <=n; i++){
			for(j = 0; j<=n; j++){
				if (c[j][i] < Y[i]){
					Y[i] = c[j][i];
				}
			}		
		}
	while(cond < e){
		l++;
		X[i] = INT_MAX;
		Y[i] = 0;
		maxmin = 0;
		minmax = INT_MAX;
		
		for(i = 0; i<=n; i++){
			if (X[i] > maxmin){
				maxmin = X[i] / l;
				I = i;					
			}
			if (Y[i] < minmax){
				minmax = Y[i] / l;
				J = i;
			}		
		}
		for(i = 0; i <= n; i++){
			X[i] += c[i][J];
			Y[i] += c[I][i];
		}
	}
}*/