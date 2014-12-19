#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 1024

enum table_state { FREE, BUSY };

struct HashTable
{
        int key;
        char value[32];
        int state;
};

int generate_key(char *str)
{
    int i, value = 0;
    int len = strlen(str);

    for(i = 0; i < len; i++)
        value += (int)str[i];

    return value;
}

int generate_hash(int key, int i)
{
    return ((key % TABLE_SIZE) + i);
}

int main()
{
    int i, key, hash, writes;
    char s_input[32] = {0};
    struct HashTable table[TABLE_SIZE];

    FILE *f = fopen("input", "r");

    memset (table, 0, sizeof (struct HashTable) * TABLE_SIZE);

    while(!feof(f))
    {
        fscanf(f, "%s", s_input);

        key = generate_key(s_input);

        writes = 0;
        for (i = 0; i < TABLE_SIZE; i++)
        {
            // if (i != 0)
            //     printf ("offset %d\n", i);

            hash = generate_hash(key, i);

            if (table[hash].state == FREE)
            {
                table[hash].key = key;
                table[hash].state = BUSY;
                strcpy(table[hash].value, s_input);

                printf("'%s' <k 0x%04x> <h 0x%04x>\n",
                       table[hash].value, table[hash].key, hash);

                /* writes success */
                writes++;

                break;
            }
        }

        if (!writes)
        {
            printf ("ops! hash table completely filled. "
                    "'%s' miss\n", s_input);
        }
    }

    return 0;
}