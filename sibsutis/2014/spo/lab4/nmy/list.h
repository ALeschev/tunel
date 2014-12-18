#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>

typedef struct entry
{
        int key;
        char value[32];
} entry_t;

typedef struct list
{
    entry_t entry;
    struct list *next;
} list_t;

void list_print(list_t *p_begin);
void list_clear(list_t **p_begin);
void list_insert(list_t **p_begin, entry_t *entry);
void list_add(list_t **p_begin, entry_t *entry);

#endif /*_LIST_H_*/