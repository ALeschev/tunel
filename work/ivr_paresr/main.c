#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#define MAX_COMMAND_LEN 64
#define MAX_ARG_CNT 2
#define MAX_ARG_LEN 32

typedef struct {
	char command[MAX_COMMAND_LEN];
	uint8_t argc;

	void (*proc)(uint8_t argc, char *argv);
} command_t;

void command_proc (uint8_t argc, char *argv)
{
	char *args[MAX_ARG_LEN/2];
	int i;

	char *str1, *token;
	char *saveptr1;

	for (i = 0, str1 = argv; i < argc; i++, str1 = NULL)
	{
		token = strtok_r(str1, " ", &saveptr1);

		if (token == NULL)
			break;

		args[i] = token;
	}

	if (argc != i)
	{
		printf ("arg error\n");
		return;
	}

	for (i = 0; i < argc; i++)
	{
		if (args[i] && strlen(args[i]))
			printf ("%d: %s\n", i, args[i]);
	}
}

int main (void)
{
	command_t t_com;
	char argv[MAX_ARG_LEN];

	strcpy (t_com.command, "hello");
	t_com.argc = 2;
	strcpy (argv, "hello world");
	t_com.proc = &command_proc;

	t_com.proc(t_com.argc, argv);

	return 0;
}