#ifndef USER_FUNCT
#define USER_FUNCT

#include "cli/common.h"

int check_auth(char *username, char *password)
{
    if (strcasecmp(username, LOGIN) != 0)
	return 0;
    if (strcasecmp(password, PASS) != 0)
	return 0;
    return 1;
}

int check_enable(char *password)
{
    return !strcasecmp(password, "topsecret");
}

void pc(struct cli_def *cli, char *string)
{
    cli_print(cli, "%s\n", string);
}

void cli_dump(struct cli_def *cli, struct cli_command *commands, int lvl)
{
        struct cli_command *c;

        if (commands)
                c = commands;
        else
                c = cli->commands;

        for ( ; c; c = c->next) {
                cli_print(cli, "%s", c->command);
                if (c->children)
                        cli_dump(cli, c->children, lvl+4);
        }
}

#define MAX_PARAM_LEN 512

#define COMMAND_HANDLER(name) int cmd_ ## name(struct cli_def *cli, char *command, char *argv[], int argc) {


#define V5_COMMAND_HANDLER(name) int cmd_ ## name(struct cli_def *cli, char *command, char *argv[], int argc) { \
    char param[MAX_PARAM_LEN];\
    memset(param, '\0', MAX_PARAM_LEN);\
    int a;\
    for (a = 0; (a < argc) && (strlen(param)+5 < MAX_PARAM_LEN); a++) { \
	if(strlen(argv[a])+5 >= (MAX_PARAM_LEN - strlen(param))) \
	{ \
		if((MAX_PARAM_LEN - strlen(param)) > 5) \
			sprintf(&param[strlen(param)], "%.*s", (MAX_PARAM_LEN - strlen(param) - 5), argv[a] );\
	}else{ \
		strcat(param, argv[a]); \
	} \
        if (a < argc - 1) strcat(param, " "); \
    }

#define END_HANDLER(name) return 1; }


COMMAND_HANDLER(reload)
    cli_print(cli, "Reload requested by console.");
    sync();
    system("reboot");
END_HANDLER(reload)

COMMAND_HANDLER(show_dump)
    cli_dump(cli, NULL, 0);
END_HANDLER(show_dump)

#endif
