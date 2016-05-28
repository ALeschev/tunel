#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "lz78_dictionary.h"
#include "lz78_pack_unpack.h"

static void lz78_write_encode(FILE *output, lz78_dict_t *dict)
{
	lz78_elem_t *p_elem = dict->head;
	int offset = 0;
	char tot_str[LZ78_MAX_DICT_STR_LEN * 2] = {0};

	while (p_elem != NULL)
	{
		offset += sprintf(&tot_str[offset], "%d%s",
		                  p_elem->rep? p_elem->rep->position:0,
		                  p_elem->string);

		fprintf(output, "%d%s", p_elem->rep? p_elem->rep->position:0,
		        p_elem->string);

		p_elem = p_elem->next;
	}

	// lz78_trace("%s", tot_str);
}

static void lz78_write_decode(FILE *output, lz78_dict_t *dict)
{
	lz78_elem_t *p_elem = dict->head;
	lz78_elem_t *rep_elem = NULL;
	int offset = 0, i, rep_offset = 0;
	char tot_str[LZ78_MAX_DICT_STR_LEN * 2] = {0};
	char rep_str[LZ78_MAX_DICT_STR_LEN * 2] = {0};

	while (p_elem != NULL)
	{
		rep_elem = p_elem->rep;

		while(rep_elem)
		{
			rep_offset += sprintf(&rep_str[rep_offset], "%s", rep_elem->string);
			rep_elem = rep_elem->rep;
		}

		for (i = 0; i < strlen(rep_str); i++)
			tot_str[offset++] = rep_str[strlen(rep_str) - i - 1];

		offset += sprintf(&tot_str[offset], "%s", p_elem->string);

		memset(rep_str, 0, sizeof(rep_str));
		rep_offset =0;

		p_elem = p_elem->next;
	}

	fputs(tot_str, output);
	// lz78_trace("%s", tot_str);
}

int lz78_pack(const char *input_file, const char *output_file)
{
	FILE *i_fd, *o_fd;
	lz78_dict_t dictionary;
	lz78_elem_t new_elem;
	lz78_elem_t *rep_elem;
	char key[LZ78_MAX_DICT_STR_LEN * 2];

	i_fd = fopen(input_file, "r");
	if (!i_fd)
	{
		lz78_trace("failed to open '%s'", input_file);
		return -1;
	}

	o_fd = fopen(output_file, "w");
	if (!o_fd)
	{
		lz78_trace("failed to create '%s'", output_file);
		fclose(i_fd);
		return -1;
	}

	memset(&dictionary, 0, sizeof(dictionary));
	memset(&new_elem, 0, sizeof(new_elem));

	while(1)
	{
		if (fgets(new_elem.string, 2, i_fd) == NULL)
			break;

		sprintf(key, "%s%s", new_elem.rep? new_elem.rep->string:"", new_elem.string);

		rep_elem = lz78_dict_find(&dictionary, key);
		if (rep_elem)
		{
			new_elem.rep = rep_elem;
			continue;
		}

		lz78_dict_add_elem(&dictionary, &new_elem);

		memset(&new_elem, 0, sizeof(new_elem));
	}

	lz78_dict_print(&dictionary);

	lz78_write_encode(o_fd, &dictionary);

	fclose(i_fd);
	fclose(o_fd);

	return 0;
}

int lz78_unpack(const char *input_file, const char *output_file)
{
	FILE *i_fd, *o_fd;
	lz78_dict_t dictionary;
	lz78_elem_t new_elem;
	lz78_elem_t *rep_elem;

	i_fd = fopen(input_file, "r");
	if (!i_fd)
	{
		lz78_trace("failed to open '%s'", input_file);
		return -1;
	}

	o_fd = fopen(output_file, "w");
	if (!o_fd)
	{
		lz78_trace("failed to create '%s'", output_file);
		fclose(i_fd);
		return -1;
	}

	memset(&dictionary, 0, sizeof(dictionary));
	memset(&new_elem, 0, sizeof(new_elem));

	while(1)
	{
		if (fgets(new_elem.string, 3, i_fd) == NULL)
			break;

		rep_elem = lz78_dict_get(&dictionary, atoi(new_elem.string));
		if (rep_elem)
			new_elem.rep = rep_elem;

		memcpy(new_elem.string, &new_elem.string[1], strlen(new_elem.string)-1);
		new_elem.string[strlen(new_elem.string)-1] = '\0';

		lz78_dict_add_elem(&dictionary, &new_elem);

		memset(&new_elem, 0, sizeof(new_elem));
	}

	lz78_dict_print(&dictionary);

	lz78_write_decode(o_fd, &dictionary);

	fclose(i_fd);
	fclose(o_fd);

	return 0;
}