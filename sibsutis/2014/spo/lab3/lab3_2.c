#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hash_entry
{
        int key;
        char value[32];
        int count;
} hash_entry_t;

typedef struct hash_list
{
    hash_entry_t entry;
    struct hash_list *next;
} hash_list_t;
 
void list_add(hash_list_t **p_begin, hash_entry_t *entry)
{
    hash_list_t *p = NULL;
    hash_list_t *pEnd = NULL;

    if(*p_begin == NULL) /* начинаем новый список */
    {
        *p_begin = malloc(sizeof(**p_begin));
        memcpy(&(*p_begin)->entry, entry, sizeof ((*p_begin)->entry));
        (*p_begin)->entry.count++;
        (*p_begin)->next = NULL;
    } else { /* добавляем в конец */
        pEnd = *p_begin;

        while(pEnd->next != NULL)
            pEnd = pEnd->next;

        p = malloc(sizeof(*p));
        memcpy(&p->entry, entry, sizeof (p->entry));
        p->next = NULL;
        pEnd->next = p;
        p->entry.count++;
    }
}
 
void list_print(hash_list_t *p_begin)
{
    hash_entry_t *p_entry;

    while(p_begin != NULL)
    {
        p_entry = &p_begin->entry;

        printf("'%s' <k %d>\n",
               p_entry->value, p_entry->key);

        p_begin = p_begin->next;
    }
}

void list_clear(hash_list_t **p_begin)
{
    hash_list_t *p = NULL;

    while(*p_begin != NULL)
    {
        p = (*p_begin)->next;
        free(*p_begin);
        *p_begin = p;
    }
}
 
void list_insert(hash_list_t **p_begin, hash_entry_t *entry)
{
    hash_list_t *p = NULL;
    hash_list_t *i = NULL;
    unsigned char flag = 0;

    if (*p_begin == NULL)
    {
        list_add(p_begin, entry);
        return;
    }

    if(entry->key <= (*p_begin)->entry.key) /* вставить перед первым */
    {
        p = malloc(sizeof(*p));
        memcpy(&p->entry, entry, sizeof (p->entry));
        p->next = (*p_begin);
        *p_begin = p;
        p->entry.count++;
    } else {
        i = (*p_begin);

        while((flag == 0) && (i->next != NULL))
        {
            if((i->entry.key < entry->key) && (i->next->entry.key >= entry->key))
            {
                flag = 1;
                break;
            }

            i = i->next;
        }

        if(flag == 0) /* вставить в конец */
        {
            p = malloc(sizeof(*p));
            memcpy(&p->entry, entry, sizeof (p->entry));
            p->next = NULL;
            i->next = p;
            p->entry.count++;
        } else { /* позиция в середине найдена */
            p = malloc(sizeof(*p));
            memcpy(&p->entry, entry, sizeof (p->entry));
            p->next = i->next;
            i->next = p;
            p->entry.count++;
        }
    }
}

int generate_key(char *str)
{
    int i, value = 0;
    int len = strlen(str);

    for(i = 0; i < len; i++)
        value += (int)str[i];

    return value;
}

int main(void)
{
    hash_list_t *hash_list = NULL;
    hash_entry_t entry;
    char s_input[32] = {0};

    FILE *f = fopen("input", "r");

    while(!feof(f))
    {
        fscanf(f, "%s", s_input);

        entry.key = generate_key(s_input);
        strcpy(entry.value, s_input);

        list_insert(&hash_list, &entry);
    }

    // for(i = 0; i<5; i++) list_add(&hash_list,i);

    // Print(hash_list);

    // printf("%s","Enter element:");

    // scanf("%i",&i);

    // list_insert(&hash_list,i);

    list_print(hash_list);

    list_clear(&hash_list);

    // getchar();

    return 0;
}