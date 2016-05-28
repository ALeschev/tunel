#ifndef __LZ78_DICTIONRY_H__
#define __LZ78_DICTIONRY_H__

#include <stdio.h>

#define LZ78_MAX_DICT_STR_LEN 16

typedef struct lz78_elem
{
	int position;
	char string[LZ78_MAX_DICT_STR_LEN];

	struct lz78_elem *rep;
	struct lz78_elem *next;
} lz78_elem_t;

typedef struct
{
	lz78_elem_t *head;
	int count;
} lz78_dict_t;


int lz78_dict_add_elem(lz78_dict_t *dict, lz78_elem_t *elem);
lz78_elem_t *lz78_dict_find(lz78_dict_t *dict, const char *str);
lz78_elem_t *lz78_dict_get(lz78_dict_t *dict, int pos);
void lz78_dict_print(lz78_dict_t *dict);

#endif /* __DICTIONRY_H__ */
