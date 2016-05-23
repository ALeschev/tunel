#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "lz78_pack.h"

#define LZ78_MAX_DICT_STR_LEN 16

typedef struct lz78_elem
{
	int position;
	int count;
	char string[LZ78_MAX_DICT_STR_LEN];

	struct lz78_elem *next;
} lz78_elem_t;

typedef struct
{
	lz78_elem_t *head;
	int count;
} lz78_dict_t;

static int lz78_dict_add_elem(lz78_dict_t *dict, lz78_elem_t *elem)
{
	lz78_elem_t *p_elem;

	if (!dict->head)
	{
		dict->head = (lz78_elem_t *)calloc(1, sizeof (lz78_elem_t));
		if (!dict->head)
		{
			lz78_trace("failed to allocate memory: %s", strerror(errno));
			return -1;
		}

		memset(dict->head, 0, sizeof (lz78_elem_t));

		p_elem = dict->head;
	}
	else
	{
		p_elem = dict->head;

		while (p_elem->next != NULL)
			p_elem = p_elem->next;

		p_elem->next = (lz78_elem_t *)calloc(1, sizeof (lz78_elem_t));
		if (!p_elem->next)
		{
			lz78_trace("failed to allocate memory: %s", strerror(errno));
			return -2;
		}

		memset(p_elem->next, 0, sizeof (lz78_elem_t));

		p_elem = p_elem->next;
	}

	p_elem->position = elem->position;
	p_elem->count = elem->count;
	memcpy(p_elem->string, elem->string, LZ78_MAX_DICT_STR_LEN);

	return 0;
}

int lz78_pack(FILE *input, FILE *output)
{
	lz78_dict_t dictionary;

	if (!input || !output)
	{
		lz78_trace("bad descriptor: in <%p> out <%p>",
		           input, output);
		return -1;
	}

	memset(&dictionary, 0, sizeof(dictionary));

	return 0;
}
