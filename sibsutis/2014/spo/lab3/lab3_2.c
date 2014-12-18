#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1024
#define BUSY 0
#define FREE 1

struct HashTable
{
	int key;
	char id[32];
	int state;
} table[TABLE_SIZE];

int FindKey(char str[32])
{
    int i, value = 0;
    for(i = 0; i < strlen(str); i++)
    {
	value += (int)str[i];
    }
    return value;
}

int ComputeHash(int key)
{
    return key%TABLE_SIZE;
}


void InitTable()
{
    int i;
    for(i = 0; i < TABLE_SIZE; i++)
    {
	strcpy(table[i].id, "\0");
	table[i].key = 0;
	table[i].state = FREE;
    }
}

int main()
{
    int key, h, j;
    char temp[32];
    FILE *f = fopen("input", "r");
    InitTable();
    while(!feof(f))
    {
	fscanf(f, "%s", temp);
	key = FindKey(temp);
	h = ComputeHash(key);
	j = 1;
	while(table[h].state != FREE)
	{
	    h = (h * j) % TABLE_SIZE;
	    printf("ячейка занята\n");
	    j = (j + 1) % TABLE_SIZE;
	}
	table[h].key = key;
	table[h].state = BUSY;
	strcpy(table[h].id, temp);
	printf("ID = '%s'\t KEY = %d\tHASH = %d\n", table[h].id, table[h].key, h);
    }

    return 0;
}