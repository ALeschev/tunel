#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum
{
	eCFG_OP_DEBUG,
	eCFG_OP_AVERAGE_TTL,
	eCFG_OP_MIN_REPROD_NEIGH,
	eCFG_OP_MAX_REPROD_NEIGH,
	eCFG_OP_MIN_REPROD_TTL,
	eCFG_OP_MAX_WAT_GROUP,

	eCFG_OP_MAX
} cfg_op_type;

typedef struct
{
	char name[64];
	cfg_op_type type;
	int value;
} option_t;

static option_t config[eCFG_OP_MAX] = 
{
	{"DEBUG",            eCFG_OP_DEBUG,            1},
	{"AVERAGE_TTL",      eCFG_OP_AVERAGE_TTL,      70},
	{"MIN_REPROD_NEIGH", eCFG_OP_MIN_REPROD_NEIGH, 1},
	{"MAX_REPROD_NEIGH", eCFG_OP_MAX_REPROD_NEIGH, 4},
	{"MIN_REPROD_TTL",   eCFG_OP_MIN_REPROD_TTL,   70/3},
	{"MAX_WAT_GROUP",    eCFG_OP_MAX_WAT_GROUP,    3}
};

void cfg_print(option_t *config)
{
	int i;

	for (i = 0; i < eCFG_OP_MAX; i++)
		printf("%s:%d\n", config[i].name, config[i].value);
}

int cfg_entry_exist(char *entry)
{
	int i;

	for (i = 0; i < eCFG_OP_MAX; i++)
	{
		if (!strcmp(config[i].name, entry))
			return config[i].type;
	}

	return -1;
}

int cfg_read_options(FILE *cfg)
{
	int i, entry_line = 1, entry_type;
	char cfg_line[128], op[64], val[16];
	char *entry;

	while(fscanf(cfg, "%s", cfg_line) != EOF)
	{
		if (strlen(cfg_line) > 1)
		{
			if (cfg_line[0] == '#')
				continue;
		}
		else
		{
			printf("error on %d line\n", entry_line);
			return -1;
		}

		entry = strtok (cfg_line,":");
		while (entry != NULL)
		{
			if ((entry_type = cfg_entry_exist(entry)) >= 0)
			{
				entry = strtok (NULL, ":");
				if (entry)
					config[entry_type].value = atoi(entry);
			}
			else
			{
				printf("invalid option '%s' on %d line\n", entry, entry_line);
				return -1;
			}

			entry = NULL;
		}

		entry_line++;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *cfg;

	if (argc < 2)
	{
		printf("set config file\n");
		return -1;
	}

	cfg = fopen(argv[1], "r");
	if (!cfg)
	{
		printf("failed to open '%s' config file\n", argv[1]);
		return -1;
	}

	if (cfg_read_options(cfg))
		return -1;

	fclose(cfg);

	return 0;
};