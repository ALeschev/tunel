#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "lz78.h"
#include "lz78_dictionary.h"

int lz78_dict_add_elem(lz78_dict_t *dict, lz78_elem_t *elem)
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
		p_elem->position++;
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

		p_elem->next->position = p_elem->position + 1;
		p_elem = p_elem->next;
	}

	p_elem->rep = elem->rep;
	memcpy(p_elem->string, elem->string, LZ78_MAX_DICT_STR_LEN);

	dict->count++;

	return 0;
}

lz78_elem_t *lz78_dict_find(lz78_dict_t *dict, const char *str)
{
	lz78_elem_t *p_elem = dict->head;
	char tot_str[LZ78_MAX_DICT_STR_LEN * 2];

	while (p_elem != NULL)
	{
		sprintf(tot_str, "%s%s", p_elem->rep? p_elem->rep->string:"", p_elem->string);

		if (!strcmp(tot_str, str))
			break;

		p_elem = p_elem->next;
	}

	return p_elem;
}

lz78_elem_t *lz78_dict_get(lz78_dict_t *dict, int pos)
{
	lz78_elem_t *p_elem = dict->head;

	while (p_elem != NULL)
	{
		if (p_elem->position == pos)
			break;

		p_elem = p_elem->next;
	}

	return p_elem;
}

void lz78_dict_print(lz78_dict_t *dict)
{
	lz78_elem_t *p_elem = dict->head;

	lz78_trace("Dictionary:");
	lz78_trace("--------------");

	while (p_elem != NULL)
	{
		lz78_trace("%d: %s%s", p_elem->position,
		           p_elem->rep? p_elem->rep->string:"", p_elem->string);
		p_elem = p_elem->next;
	}

	lz78_trace("--------------");
}
