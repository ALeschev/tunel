#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define TIME_MAX 24

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
int numOfRepair;

void Init(char *name);
double Cacl_Survivability(double timeVar);
void SaveToFile(char *name, double *data[]);
double Cacl_Employment(double timeVar);
double Get_N();
double Get_M();
void rangeInit ();
void rangeAdd (double val);
double rangeGet (double p);

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

	sprintf (input_file, "i_%s.in", argv[1]);

	printf ("load '%s'\n", input_file);

	Init(input_file);

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
		{
			dataArray[t/2][i] = (atoi(argv[2]) == 0) ? Cacl_Survivability(t) : Cacl_Employment(t);
			printf (".");
		}

		printf ("\n");
	}

	char output_file[256] = {0};
	sprintf (output_file, "o_%s.%s.data", argv[1], argv[2]);

	SaveToFile(output_file, dataArray);
}

void Init(char *name)
{
	int size;
	char *pch = NULL;
//	char *confLines = NULL;
	int confLines[10] = {0};
	FILE *input_fd = NULL;

	if (!name)
	{
		printf ("[%s] \"name\" is <%p>\n", __func__, name);
		return;
	}

	input_fd = fopen (name, "r");
	if (!input_fd)
	{
		printf ("[%s] can't open \"%s\"\n", __func__, name);
		return;
	}

	int i = 0;
	while (fscanf (input_fd, "%d", &confLines[i]) && i != 10)
	{
		printf ("%s\n", confLines[i]);
		i++;
	}

//	size = fseek(input_fd, 0, SEEK_END); 
//	fseek(input_fd, 0, SEEK_SET); 
//	confLines = (char *)calloc(size, sizeof(char)); 
//	if (!confLines)
//	{
//		printf ("[%s] can't allocate memory\n", __func__);
//		return;
//	}
//	fread(confLines, size, sizeof(char), input_fd);

	printf ("confLines:\n%s\n", confLines);

	fclose (input_fd);

	int confElements[4] = {0};
	memcpy (confElements, confLines, sizeof (int) * 4);

//	pch = strtok (confLines, " ");
//	while ((pch != NULL) && (i < 4))
//	{
//		confElements[i++] = atoi (pch);
//		printf ("confElement %d\n", confElements);
//		pch = strtok (NULL, " ");
//	}

	numOfPC = confElements[0];
	startPoint = confElements[1];
	failRate = pow(10, confElements[2]);
	repairRate = confElements[3];
	numOfRepair = confElements[4];

	indexOfDifference = atoi(confLines);

	rangeInit();

	pch = strtok (confLines, " ");
	while (pch != NULL)
	{
		rangeAdd (atoi(pch));
		pch = strtok (NULL, " ");
	}

	// foreach (var rangeElement in confLines[2].Split(' '))
	// { rangeOfDifference.Add(double.Parse(rangeElement)); }
	/* --- Obtain range of difference --- */
}

double Cacl_Survivability( double timeVar)
{
	double retVal;
	double curN = numOfPC;

	retVal = (repairRate/(failRate + repairRate)) +
	         ((startPoint*failRate - (curN - startPoint)*repairRate)/(curN*(failRate + repairRate))) *
	         exp(-(failRate + repairRate)*timeVar);

	return retVal;
}

double Cacl_Employment(double timeVar)
{
	double retVal;
	double curN = numOfPC;

	retVal = (curN*failRate / (numOfRepair * (failRate + repairRate))) -
	         ((startPoint * failRate - (curN - startPoint) * repairRate) / (numOfRepair * (failRate + repairRate))) *
	         exp(-(failRate + repairRate) * timeVar);

	return retVal;
}

void SaveToFile(char *name, double *data[])
{
	FILE *output_fd = NULL;
	if (!name)
	{
		printf ("[%s] \"name\" is <%p>\n", __func__, name);
		return;
	}

	output_fd = fopen (name, "w");
	if (!output_fd)
	{
		printf ("[%s] can't open \"%s\" <%p>\n", __func__, name, output_fd);
		return;
	}

	printf ("[%s] Start writing to disk...\n", __func__);

	int j, t;
	char out_str[512] = {0};
	char tmpStr[32] = {0};
	for (t = 0; t < TIME_MAX; t+=2)
	{
		sprintf(tmpStr, "%d ", t);
		strcat (out_str, tmpStr);

		for (j = 0; j < rangeOfDifference.count; j++)
		{
			sprintf(tmpStr,"%.2f ", data[t/2][j]);
			strcat (out_str, tmpStr);
		}

		fwrite (out_str, sizeof (out_str), sizeof (char), output_fd);
		memset (out_str, 0, sizeof (out_str));
		memset (tmpStr, 0, sizeof (tmpStr));
	}

	fclose (output_fd);

	printf ("Done\n");
	getchar();
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
	rangeOfDifference.count = 0;
}

void rangeAdd (double val)
{
	if (rangeOfDifference.diff == NULL)
		return;

	Difference *p_diff = rangeOfDifference.diff;
	while (p_diff->next)
		p_diff = p_diff->next;

	if (rangeOfDifference.diff == p_diff)
	{
		p_diff->difference = val;
		rangeOfDifference.count++;
		return;
	}

	p_diff->next = (struct _difference *)malloc(sizeof (Difference));
	p_diff = p_diff->next;
	p_diff->difference = val;

	rangeOfDifference.count++;
}

double rangeGet (double p)
{
	if (rangeOfDifference.diff == NULL)
		return;

	int i = 0;
	Difference *p_diff = rangeOfDifference.diff;
	while (p_diff && (i != p))
	{
		i++;
		p_diff = p_diff->next;
	}

	return p_diff->difference;
}

