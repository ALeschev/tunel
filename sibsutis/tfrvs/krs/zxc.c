#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INPUT_PATH "idata"
#define OUTPUT_PATH "odata"
#define TIME_MAX 24
#define ERROR 1
#define SUCCESS 0

typedef struct _difference
{
	double difference;
	struct _difference *next;
} Difference;

typedef struct _rangeOfDifferenceList
{
	Difference *diff;
	int count;
} rangeOfDifferenceList;

rangeOfDifferenceList rangeOfDifference;
int indexOfDifference;

int startPoint;
int numOfPC;
double failRate;
double repairRate;
double numOfRepair;

int Init(char *name);
double Cacl_Survivability(double timeVar);
int SaveToFile(char *name, double *data[]);
double Cacl_Employment(double timeVar);
double Get_N();
double Get_M();
void rangeInit ();
void rangeAdd (double val);
double rangeGet (int p);

int main (int argc, char *argv[])
{
	if(argc < 2)
	{
		printf ("Argument error\n");
		return -1;
	}

	int i, t;
	char input_file[256] = {0};
	double *dataArray[TIME_MAX / 2];
	int employment = atoi(argv[2]);

	sprintf (input_file, "%s/i_%s.in", INPUT_PATH, argv[1]);

	printf ("input '%s'\n", input_file);

	if (Init(input_file) == ERROR)
		return ERROR;

	for (i = 0; i < TIME_MAX / 2; i++)
		dataArray[i] = (double *)malloc(sizeof (double) * rangeOfDifference.count);

	for (i = 0; i < rangeOfDifference.count; i++)
	{
		switch (indexOfDifference)
		{
			case 0:
				printf ("Unrecognize value\n");
			break;

			case 1:
				startPoint = (int)rangeGet(i);
			break;

			case 2:
				failRate = pow(10, (int)rangeGet(i));
			break;

			case 3:
				repairRate = (int)rangeGet(i);
			break;

			case 4:
				numOfRepair = (int)rangeGet(i);
			break;
		}

		for (t = 0; t < TIME_MAX; t += 2)
			dataArray[t/2][i] = (employment) ? Cacl_Employment(t) : Cacl_Survivability(t);
	}

	char output_file[256] = {0};
	sprintf (output_file, "%s/o_%s.%s.data", OUTPUT_PATH, argv[1], argv[2]);

	if (SaveToFile(output_file, dataArray) == ERROR)
		return ERROR;

	return SUCCESS;
}

int Init(char *name)
{
	int size;
	char *pch = NULL;
	int confLines[10] = {0};
	FILE *input_fd = NULL;
	struct stat st = {0};

	if (!name)
	{
		printf ("[%s] \"name\" is <%p>\n", __func__, name);
		return ERROR;
	}

	if (stat(INPUT_PATH, &st) == -1)
		return ERROR;

	input_fd = fopen (name, "r");
	if (!input_fd)
	{
		printf ("[%s] can't open \"%s\"\n", __func__, name);
		return ERROR;
	}

	int i = 0;
	while (fscanf (input_fd, "%d", &confLines[i]) && i != 10)
		i++;

	fclose (input_fd);

	int confElements[10] = {0};
	memcpy (confElements, confLines, sizeof (int) * 10);

	i = 0;
	numOfPC = confElements[i++];
	startPoint = confElements[i++];
	failRate = pow(10, confElements[i++]);
	repairRate = confElements[i++];
	numOfRepair = confElements[i++];

	indexOfDifference = confLines[i++];

	rangeInit();

	while (i < 10)
		rangeAdd (confLines[i++]);

	return SUCCESS;
}

double Cacl_Survivability( double timeVar)
{
	double retVal;
	double curN = numOfPC;

	retVal = (repairRate / (failRate + repairRate)) +
	         ((startPoint * failRate - (curN - startPoint) * repairRate) / (curN * (failRate + repairRate))) *
	         exp(-(failRate + repairRate) * timeVar);

	return retVal;
}

double Cacl_Employment(double timeVar)
{
	double retVal;
	double curN = numOfPC;

	retVal = (curN * failRate / (numOfRepair * (failRate + repairRate))) -
	         ((startPoint * failRate - (curN - startPoint) * repairRate) / (numOfRepair * (failRate + repairRate))) *
	         exp(-(failRate + repairRate) * timeVar);

	return retVal;
}

int SaveToFile(char *name, double *data[])
{
	struct stat st = {0};
	FILE *output_fd = NULL;
	if (!name)
	{
		printf ("[%s] \"name\" is <%p>\n", __func__, name);
		return ERROR;
	}

	printf ("output: %s\n", name);

	if (stat(OUTPUT_PATH, &st) == -1)
		mkdir(OUTPUT_PATH, 0775);

	output_fd = fopen (name, "w");
	if (!output_fd)
	{
		printf ("[%s] can't open \"%s\" <%p>\n", __func__, name, output_fd);
		return ERROR;
	}

	printf ("[%s] Start writing to disk...\n", __func__);

	int j, t;
	char out_str[2048] = {0};
	char tmpStr[32] = {0};
	for (t = 0; t < TIME_MAX; t+=2)
	{
		sprintf(tmpStr, "%d ", t);
		strcat (out_str, tmpStr);

		for (j = 0; j < rangeOfDifference.count; j++)
		{
			sprintf(tmpStr,"%f ", data[t/2][j]);
			strcat (out_str, tmpStr);
		}

		strcat (out_str, "\n");
	}

	fwrite (out_str, sizeof (char) * strlen (out_str), sizeof (char), output_fd);

	fclose (output_fd);

	printf ("-----------------------------------\n");

	return SUCCESS;
}

double Get_N()
{
	return repairRate / (failRate + repairRate);
}

double Get_M()
{
	return (Get_N() * failRate) / ceil(numOfRepair * (failRate + repairRate));
}

void rangeInit ()
{
	rangeOfDifference.diff = (struct _difference *)malloc(sizeof (Difference));
	rangeOfDifference.diff->next = NULL;
	rangeOfDifference.count = 0;
}

void rangeAdd (double val)
{
	if (rangeOfDifference.diff == NULL)
		return;

	Difference *p_diff = rangeOfDifference.diff;

	while (p_diff->next)
		p_diff = p_diff->next;

	p_diff->difference = val;
	rangeOfDifference.count++;

	Difference *new_elem = (struct _difference *)malloc(sizeof (Difference));
	p_diff->next = new_elem;
	p_diff = p_diff->next;
	p_diff->difference = 0;
	p_diff->next = NULL;
}

double rangeGet (int p)
{
	if (rangeOfDifference.diff == NULL)
		return -1;

	int i = 0;
	Difference *p_diff = rangeOfDifference.diff;
	while (p_diff && (i != p))
	{
		i++;
		p_diff = p_diff->next;
	}

	return p_diff->difference;
}