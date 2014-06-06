#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#include "command_proc.h"

int main (int argc, char *argv[])
{
	int cmd_idx = -1;

	command_init();

	if (argc < 3 || argc > 3)
	{
		printf ("ERROR: arg error\nUsage: [command] \"[args]\"\n");
		return 1;
	}

	if ((cmd_idx = check_command (argv[1])) >= 0)
	{
		//int i;
		// char args[256] = {0};

		// for (i = 2; i < argc; i++)
		// 	strcat (args, argv[i]);

		command_t *p_command = get_command (cmd_idx);

		if (!p_command)
		{
			printf ("ERROR: can't get command [%d]", cmd_idx);
			return 1;
		}

		p_command->proc(p_command->argc, argv[2]);
	} else {
		printf ("ERROR: unsupported command\nUsage:\n");
		print_all_commands();
	}

	return 0;
}