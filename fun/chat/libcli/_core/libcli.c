
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <regex.h>
#include "libcli.h"

// vim:sw=4 ts=8

#ifdef __GNUC__
# define UNUSED(d) d __attribute__ ((unused))
#else
# define UNUSED(d) d
#endif

enum cli_states {
	STATE_LOGIN,
	STATE_PASSWORD,
	STATE_NORMAL,
	STATE_ENABLE_PASSWORD,
	STATE_ENABLE
};

struct unp {
	char *username;
	char *password;
	struct unp *next;
};

struct cli_filter_cmds
{
	char *cmd;
	char *help;
};

/* free and zero (to avoid double-free) */
#define free_z(p) if (p) { free(p); (p) = 0; }

int cli_match_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt);
int cli_range_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt);
int cli_count_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt);
int cli_match_filter(struct cli_def *cli, char *string, void *data);
int cli_range_filter(struct cli_def *cli, char *string, void *data);
int cli_count_filter(struct cli_def *cli, char *string, void *data);

static struct cli_filter_cmds filter_cmds[] =
{
	{ "begin",   "Begin with lines that match" },
	{ "between", "Between lines that match" },
	{ "count",   "Count of lines"   },
	{ "exclude", "Exclude lines that match" },
	{ "include", "Include lines that match" },
	{ "grep",	"Include lines that match regex (options: -v, -i, -e)" },
	{ "egrep",   "Include lines that match extended regex" },
	{ NULL, NULL}
};

char *cli_command_name(struct cli_def *cli, struct cli_command *command)
{
	char *name = cli->commandname;
	char *o;

	if (name) free(name);
	name = (char *)calloc(1, 1);

	while (command)
	{
		o = name;
		if (asprintf(&name, "%s %s", command->command, o) <= 0)
		{

		}
		command = command->parent;
		free(o);
	}
	cli->commandname = name;

	return name;
}

void cli_set_auth_callback(struct cli_def *cli, int (*auth_callback)(char *, char *))
{
	cli->auth_callback = auth_callback;
}

void cli_set_enable_callback(struct cli_def *cli, int (*enable_callback)(char *))
{
	cli->enable_callback = enable_callback;
}

void cli_allow_user(struct cli_def *cli, char *username, char *password)
{
	struct unp *u, *n;
	n = (struct unp *)malloc(sizeof(struct unp));
	n->username = strdup(username);
	n->password = strdup(password);
	n->next = NULL;

	if (!cli->users)
		cli->users = n;
	else
	{
		for (u = cli->users; u && u->next; u = u->next);
		if (u) u->next = n;
	}
}

void cli_allow_enable(struct cli_def *cli, char *password)
{
	free_z(cli->enable_password);
	cli->enable_password = strdup(password);
}

void cli_deny_user(struct cli_def *cli, char *username)
{
	struct unp *u, *p = NULL;
	if (!cli->users) return;
	for (u = cli->users; u; u = u->next)
	{
		if (strcmp(username, u->username) == 0)
		{
			if (p)
				p->next = u->next;
			else
				cli->users = u->next;
			free(u->username);
			free(u->password);
			free(u);
			break;
		}
		p = u;
	}
}

void cli_set_banner(struct cli_def *cli, char *banner)
{
	free_z(cli->banner);
	if (banner && *banner)
		cli->banner = strdup(banner);
}

void cli_set_hostname(struct cli_def *cli, char *hostname)
{
	free_z(cli->hostname);
	if (hostname && *hostname)
		cli->hostname = strdup(hostname);
}

void cli_set_promptchar(struct cli_def *cli, char *promptchar)
{
	free_z(cli->promptchar);
	cli->promptchar = strdup(promptchar);
}

int cli_set_privilege(struct cli_def *cli, int priv)
{
	char privp[] = "# ";
	char notprivp[] = "> ";
	int old = cli->privilege;
	cli->privilege = priv;

	if (priv != old)
		cli_set_promptchar(cli, (priv == PRIVILEGE_PRIVILEGED) ? privp : notprivp);

	return old;
}

void cli_set_modestring(struct cli_def *cli, char *modestring)
{
	free_z(cli->modestring);
	if (modestring)
		cli->modestring = strdup(modestring);
}

int cli_set_configmode(struct cli_def *cli, int mode, char *config_desc)
{
	int old = cli->mode;
	cli->mode = mode;

	if (mode != old)
	{
		if (!cli->mode)
		{
			// Not config mode
			cli_set_modestring(cli, NULL);
		}
		else if (config_desc && *config_desc)
		{
			char string[64];
			snprintf(string, sizeof(string), "(config-%s)", config_desc);
			cli_set_modestring(cli, string);
		}
		else
		{
			cli_set_modestring(cli, "(config)");
		}
	}

	return old;
}

int cli_build_shortest(struct cli_def *cli, struct cli_command *commands)
{
	struct cli_command *c, *p;

	for (c = commands; c; c = c->next)
	{
		for (c->unique_len = 1; c->unique_len <= strlen(c->command); c->unique_len++)
		{
			for (p = commands; p; p = p->next)
			{
				if (c == p) continue;
				if ((c->mode == p->mode || c->mode == MODE_ANY || p->mode == MODE_ANY) &&
					c->privilege == p->privilege &&
					strncmp(p->command, c->command, c->unique_len) == 0)
						break; //foundmatch
			}
			if (!p) break; //check foundmatch
		}
		if (c->children) cli_build_shortest(cli, c->children);
	}
	return CLI_OK;
}

struct cli_command *cli_register_command(struct cli_def *cli,
	struct cli_command *parent, char *command,
	int (*callback)(struct cli_def *cli, char *, char **, int),
	int privilege, int mode, char *help)
{
	struct cli_command *c, *p;

	if (!command) return NULL;
	if (!(c = (struct cli_command *)calloc(sizeof(struct cli_command), 1))) return NULL;

	c->callback = callback;
	c->next = NULL;
	c->command = strdup(command);
	c->parent = parent;
	c->privilege = privilege;
	c->mode = mode;
	if (help) c->help = strdup(help);
	c->help_ex = NULL;

	if (parent)
	{
		if (!parent->children)
		{
			parent->children = c;
		}
		else
		{
			for (p = parent->children; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}
	else
	{
		if (!cli->commands)
		{
			cli->commands = c;
		}
		else
		{
			for (p = cli->commands; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}

	cli_build_shortest(cli, (parent) ? parent : cli->commands);
	return c;
}

struct cli_command *cli_register_command_ex(struct cli_def *cli,
	struct cli_command *parent, char *command,
	int (*callback)(struct cli_def *cli, char *, char **, int),
	int privilege, int mode, char *help, char *help_ex)
{
	struct cli_command *c, *p;

	if (!command) return NULL;
	if (!(c = (struct cli_command *)calloc(sizeof(struct cli_command), 1))) return NULL;

	c->callback = callback;
	c->next = NULL;
	c->command = strdup(command);
	c->parent = parent;
	c->privilege = privilege;
	c->mode = mode;
	if (help) c->help = strdup(help);
	if (help_ex) c->help_ex = strdup(help_ex);

	if (parent)
	{
		if (!parent->children)
		{
			parent->children = c;
		}
		else
		{
			for (p = parent->children; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}
	else
	{
		if (!cli->commands)
		{
			cli->commands = c;
		}
		else
		{
			for (p = cli->commands; p && p->next; p = p->next);
			if (p) p->next = c;
		}
	}

	cli_build_shortest(cli, (parent) ? parent : cli->commands);
	return c;
}

static void cli_free_command(struct cli_command *cmd)
{
	struct cli_command *c,*p;

	for (c = cmd->children; c;)
	{
		p = c->next;
		cli_free_command(c);
		c = p;
	}

	free(cmd->command);
	if (cmd->help) free(cmd->help);
	free(cmd);
}

int cli_unregister_command(struct cli_def *cli, char *command)
{
	struct cli_command *c, *p = NULL;

	if (!command) return -1;
	if (!cli->commands) return CLI_OK;

	for (c = cli->commands; c; c = c->next)
	{
		if (strcmp(c->command, command) == 0)
		{
			if (p)
				p->next = c->next;
			else
				cli->commands = c->next;

			cli_free_command(c);
			return CLI_OK;
		}
		p = c;
	}

	return CLI_OK;
}

int cli_show_help(struct cli_def *cli, struct cli_command *c)
{
	struct cli_command *p;

	for (p = c; p; p = p->next)
	{
		if (p->command && p->callback && cli->privilege >= p->privilege &&
			(p->mode == cli->mode || p->mode == MODE_ANY))
		{
			cli_error(cli, "  %-20s %s", cli_command_name(cli, p), p->help ? : "");
		}

		if (p->children)
			cli_show_help(cli, p->children);
	}

	return CLI_OK;
}

int cli_int_enable(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	if (cli->privilege == PRIVILEGE_PRIVILEGED)
		return CLI_OK;

	if (!cli->enable_password && !cli->enable_callback)
	{
		/* no password required, set privilege immediately */
		cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
		cli_set_configmode(cli, MODE_EXEC, NULL);
	}
	else
	{
		/* require password entry */
		cli->state = STATE_ENABLE_PASSWORD;
	}

	return CLI_OK;
}

int cli_int_disable(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_EXEC, NULL);
	return CLI_OK;
}

int cli_int_help(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_error(cli, "\nCommands available:");
	cli_show_help(cli, cli->commands);
	return CLI_OK;
}

int cli_int_history(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	int i;

	cli_error(cli, "\nCommand history:");
	for (i = 0; i < MAX_HISTORY; i++)
	{
		if (cli->history[i])
			cli_error(cli, "%3d. %s", i, cli->history[i]);
	}

	return CLI_OK;
}

int cli_int_quit(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_EXEC, NULL);
	return CLI_QUIT;
}

int cli_int_exit(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if (cli->mode == MODE_EXEC)
		return cli_int_quit(cli, command, argv, argc);

	if (cli->mode > MODE_CONFIG)
		cli_set_configmode(cli, MODE_CONFIG, NULL);
	else
		cli_set_configmode(cli, MODE_EXEC, NULL);

	cli->service = NULL;
	return CLI_OK;
}

int cli_int_configure_terminal(struct cli_def *cli, UNUSED(char *command), UNUSED(char *argv[]), UNUSED(int argc))
{
	cli_set_configmode(cli, MODE_CONFIG, NULL);
	return CLI_OK;
}

struct cli_def *cli_init()
{
	struct cli_def *cli;
	struct cli_command *c;

	if (!(cli = (struct cli_def *)calloc(sizeof(struct cli_def), 1)))
		return 0;

	cli->buf_size = 32768;//1024;
	cli->buffer = (char *)calloc(cli->buf_size, 1);

	cli_register_command(cli, 0, "help", cli_int_help, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show available commands");
	cli_register_command(cli, 0, "quit", cli_int_quit, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Disconnect");
	//cli_register_command(cli, 0, "logout", cli_int_quit, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Disconnect");
	//cli_register_command(cli, 0, "exit", cli_int_exit, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Exit from current mode");
	cli_register_command(cli, 0, "history", cli_int_history, PRIVILEGE_UNPRIVILEGED, MODE_ANY, "Show a list of previously run commands");
	//cli_register_command(cli, 0, "enable", cli_int_enable, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Turn on privileged commands");
	//cli_register_command(cli, 0, "disable", cli_int_disable, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Turn off privileged commands");

	c = cli_register_command(cli, 0, "configure", 0, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Enter configuration mode");
	cli_register_command(cli, c, "terminal", cli_int_configure_terminal, PRIVILEGE_PRIVILEGED, MODE_EXEC, "Configure from the terminal");

	cli->privilege = cli->mode = -1;
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_EXEC, 0);
	return cli;
}

void cli_unregister_all(struct cli_def *cli, struct cli_command *command)
{
	struct cli_command *c, *p = NULL;

	if (!command) command = cli->commands;
	if (!command) return;

	for (c = command; c; )
	{
		p = c->next;

		// Unregister all child commands
		if (c->children)
			cli_unregister_all(cli, c->children);

		if (c->command) free(c->command);
		if (c->help) free(c->help);
		free(c);

		c = p;
	}
}

int cli_done(struct cli_def *cli)
{
	struct unp *u = cli->users, *n;

	if (!cli) return CLI_OK;
	cli_free_history(cli);

	// Free all users
	while (u)
	{
		if (u->username) free(u->username);
		if (u->password) free(u->password);
		n = u->next;
		free(u);
		u = n;
	}

	/* free all commands */
	cli_unregister_all(cli, 0);

	free_z(cli->commandname);
	free_z(cli->modestring);
	free_z(cli->banner);
	free_z(cli->promptchar);
	free_z(cli->hostname);
	free_z(cli->buffer);
	free_z(cli);

	return CLI_OK;
}

static int cli_add_history(struct cli_def *cli, char *cmd)
{
	int i;
	for (i = 0; i < MAX_HISTORY; i++)
	{
		if (!cli->history[i])
		{
			if (i == 0 || strcasecmp(cli->history[i-1], cmd))
			cli->history[i] = strdup(cmd);
			return CLI_OK;
		}
	}
	// No space found, drop one off the beginning of the list
	free(cli->history[0]);
	for (i = 0; i < MAX_HISTORY-1; i++)
		cli->history[i] = cli->history[i+1];
	cli->history[MAX_HISTORY - 1] = strdup(cmd);
	return CLI_OK;
}

void cli_free_history(struct cli_def *cli)
{
	int i;
	for (i = 0; i < MAX_HISTORY; i++)
	{
		if (cli->history[i])
			free_z(cli->history[i]);
	}
}

static int cli_parse_line(char *line, char *words[], int max_words)
{
	int nwords = 0;
	char *p = line;
	char *word_start = line;
	int inquote = 0;

	while (nwords < max_words - 1)
	{
		if (!*p || *p == inquote || (word_start && !inquote && (isspace(*p) || *p == '|')))
		{
			if (word_start)
			{
				int len = p - word_start;

				memcpy(words[nwords] = (char *)malloc(len + 1), word_start, len);
				words[nwords++][len] = 0;
			}

			if (!*p)
				break;

			if (inquote)
				p++; /* skip over trailing quote */

			inquote = 0;
			word_start = 0;
		}
		else if (*p == '"' || *p == '\'')
		{
			inquote = *p++;
			word_start = p;
		}
		else
		{
			if (!word_start)
			{
				if (*p == '|')
					words[nwords++] = strdup("|");
				else if (!isspace(*p))
					word_start = p;
			}

			p++;
		}
	}

	return nwords;
}

static char *join_words(int argc, char **argv)
{
	char *p;
	int len = 0;
	int i;

	for (i = 0; i < argc; i++)
	{
		if (i)
			len += 1;

		len += strlen(argv[i]);
	}

	p = (char *)malloc(len + 1);
	p[0] = 0;

	for (i = 0; i < argc; i++)
	{
		if (i)
			strcat(p, " ");

		strcat(p, argv[i]);
	}

	return p;
}

static int cli_find_command(struct cli_def *cli, struct cli_command *commands, int num_words, char *words[], int start_word, int filters[])
{
	struct cli_command *c, *again = NULL;
	int c_words = num_words;
	int help_flag = 0;

	if (filters[0])
		c_words = filters[0];

	// Deal with ? for help
	if (!words[start_word])
		return CLI_ERROR;

	if((num_words > 1) && ((start_word + 1) < num_words) && (words[start_word + 1][strlen(words[start_word + 1]) - 1] == '?'))
			help_flag = 1;

	if (words[start_word][strlen(words[start_word]) - 1] == '?' || help_flag)
	{
		int l = strlen(words[start_word])-1;

		if (commands->parent && commands->parent->callback)
		{
				if(help_flag)
						cli_error(cli, "%-20s %s", cli_command_name(cli, commands->parent),
								commands->parent->help_ex ? : (commands->parent->help ? : ""));
				else
						cli_error(cli, "%-20s %s", cli_command_name(cli, commands->parent),
								commands->parent->help ? : "");
		}

		for (c = commands; c; c = c->next)
		{
			if (strncasecmp(c->command, words[start_word], l) == 0
				&& (c->callback || c->children)
				&& cli->privilege >= c->privilege
				&& (c->mode == cli->mode || c->mode == MODE_ANY))
				{
						if(help_flag)
								cli_error(cli, "  %-20s %s", c->command, c->help_ex ? : (c->help ? : ""));
						else
								cli_error(cli, "  %-20s %s", c->command, c->help ? : "");
				}
		}

		return CLI_OK;
	}

	for (c = commands; c; c = c->next)
	{
		if (cli->privilege < c->privilege)
			continue;

		if (strncasecmp(c->command, words[start_word], c->unique_len))
			continue;

		if (strncasecmp(c->command, words[start_word], strlen(words[start_word])))
			continue;

		AGAIN:
		if (c->mode == cli->mode || c->mode == MODE_ANY)
		{
			int rc = CLI_OK;
			int f;
			struct cli_filter **filt = &cli->filters;

			// Found a word!
			if (!c->children)
			{
				// Last word
				if (!c->callback)
				{
					cli_error(cli, "No callback for \"%s\"", cli_command_name(cli, c));
					return CLI_ERROR;
				}
			}
			else
			{
				if (start_word == c_words - 1)
				{
					if (c->callback)
						goto CORRECT_CHECKS;

					cli_error(cli, "Incomplete command");
					return CLI_ERROR;
				}
				rc = cli_find_command(cli, c->children, num_words, words, start_word + 1, filters);
				if (rc == CLI_ERROR_ARG)
				{
					if (c->callback)
					{
						rc = CLI_OK;
						goto CORRECT_CHECKS;
					}
					else
					{
						cli_error(cli, "Invalid %s \"%s\"", commands->parent ? "argument" : "command", words[start_word]);
					}
				}
				return rc;
			}

			if (!c->callback)
			{
				cli_error(cli, "Internal server error processing \"%s\"", cli_command_name(cli, c));
				return CLI_ERROR;
			}

			CORRECT_CHECKS:
			for (f = 0; rc == CLI_OK && filters[f]; f++)
			{
				int n = num_words;
				char **argv;
				int argc;
				int len;

				if (filters[f+1])
				n = filters[f+1];

				if (filters[f] == n - 1)
				{
					cli_error(cli, "Missing filter");
					return CLI_ERROR;
				}

				argv = words + filters[f] + 1;
				argc = n - (filters[f] + 1);
				len = strlen(argv[0]);
				if (argv[argc - 1][strlen(argv[argc - 1]) - 1] == '?')
				{
					if (argc == 1)
					{
						int i;

						for(i = 0; filter_cmds[i].cmd; i++)
						{
							cli_error(cli, "  %-20s %s", filter_cmds[i].cmd, filter_cmds[i].help );
						}
					}
					else
					{
						if (argv[0][0] != 'c') // count
							cli_error(cli, "  WORD");

						if (argc > 2 || argv[0][0] == 'c') // count
							cli_error(cli, "  <cr>");
					}

					return CLI_OK;
				}

				if (argv[0][0] == 'b' && len < 3) // [beg]in, [bet]ween
				{
					cli_error(cli, "Ambiguous filter \"%s\" (begin, between)", argv[0]);
					return CLI_ERROR;
				}
				*filt = (struct cli_filter *)calloc(sizeof(struct cli_filter), 1);

				if (!strncmp("include", argv[0], len) ||
					!strncmp("exclude", argv[0], len) ||
					!strncmp("grep", argv[0], len) ||
					!strncmp("egrep", argv[0], len))
						rc = cli_match_filter_init(cli, argc, argv, *filt);
				else if (!strncmp("begin", argv[0], len) ||
					!strncmp("between", argv[0], len))
						rc = cli_range_filter_init(cli, argc, argv, *filt);
				else if (!strncmp("count", argv[0], len))
					rc = cli_count_filter_init(cli, argc, argv, *filt);
				else
				{
					cli_error(cli, "Invalid filter \"%s\"", argv[0]);
					rc = CLI_ERROR;
				}

				if (rc == CLI_OK)
				{
					filt = &(*filt)->next;
				}
				else
				{
					free(*filt);
					*filt = 0;
				}
			}

			if (rc == CLI_OK)
				rc = c->callback(cli, cli_command_name(cli, c), words + 1, c_words - 1);

			while (cli->filters)
			{
				struct cli_filter *filt = cli->filters;

				// call one last time to clean up
				filt->filter(cli, NULL, filt->data);
				cli->filters = filt->next;
				free(filt);
			}

			return rc;
		}
		else if (cli->mode > MODE_CONFIG && c->mode == MODE_CONFIG)
		{
			// command matched but from another mode,
			// remeber it if we fail to find correct command
			again = c;
		}
	}

	// drop out of config submode if we have matched command on MODE_CONFIG
	if (again)
	{
		c = again;
		cli_set_configmode(cli, MODE_CONFIG, NULL);
		goto AGAIN;
	}

	if (start_word == 0)
		cli_error(cli, "Invalid %s \"%s\"", commands->parent ? "argument" : "command", words[start_word]);
	return CLI_ERROR_ARG;
}

int cli_run_command(struct cli_def *cli, char *command)
{
	int r;
	unsigned int num_words, i, f;
	char *words[128] = {0};
	int filters[128] = {0};

	if (!command) return CLI_ERROR;
	while (isspace(*command))
		command++;

	if (!*command) return CLI_OK;

	num_words = cli_parse_line(command, words, sizeof(words) / sizeof(words[0]));
	for (i = f = 0; i < num_words && f < sizeof(filters) / sizeof(filters[0]) - 1; i++)
	{
		if (words[i][0] == '|')
		filters[f++] = i;
	}

	filters[f] = 0;

	if (num_words)
		r = cli_find_command(cli, cli->commands, num_words, words, 0, filters);
	else
		r = CLI_ERROR;

	for (i = 0; i < num_words; i++)
		free(words[i]);

	if (r == CLI_QUIT)
		return r;

	return CLI_OK;
}

static int cli_get_completions(struct cli_def *cli, char *command, char **completions, int max_completions)
{
	struct cli_command *c;
	int num_words, i, k=0;
	char *words[128] = {0};
	int filter = 0;

	if (!command) return 0;
	while (isspace(*command))
		command++;

	if (!*command) return 0;

	num_words = cli_parse_line(command, words, sizeof(words)/sizeof(words[0]));
	for (i = 0; i < num_words; i++)
	{
		if (words[i][0] == '|')
		filter = i;
	}

	if (!num_words) return 0;

	if ( command[strlen(command)-1] == ' ') num_words++;

	if (filter) // complete filters
	{
		int all = 0;
		unsigned int len;

		if (num_words - filter > 2) return 0;	// filter already completed
		if (num_words - filter == 1) all = 1;

		len = strlen(words[num_words-1]);

		for (i = 0; filter_cmds[i].cmd && k < max_completions; i++)
			if (all || ( len != strlen(filter_cmds[i].cmd)
				&& !strncmp(filter_cmds[i].cmd, words[num_words - 1], len)))
					completions[k++] = filter_cmds[i].cmd;

		completions[k] = NULL;
		return k;
	}

	for (c = cli->commands, i=0; c && i < num_words && k < max_completions && words[i];)
	{
		if (cli->privilege < c->privilege)
			goto NEXT_COMMAND;

		if (c->mode != cli->mode && c->mode != MODE_ANY)
			goto NEXT_COMMAND;

		if (strncasecmp(c->command, words[i], strlen(words[i])))
			goto NEXT_COMMAND;

		if (num_words - i == 1)
			completions[k++] = c->command;

		if (strncasecmp(c->command, words[i], c->unique_len))
			goto NEXT_COMMAND;

		// matched
		if (num_words - i > 0)
		{
			c = c->children;
			i++;
			continue;
		}

		if (strlen(words[i]) == strlen(c->command))
		{
			for(c = c->children, k=0; c; c = c->next)
			{
				if (cli->privilege < c->privilege)
					continue;
				if (c->mode != cli->mode && c->mode != MODE_ANY)
					continue;
				completions[k++] = c->command;
			}
		}
		else
		{
			completions[0] = c->command;
			k = 1;
		}
		return k;
		NEXT_COMMAND:
		c = c->next;
	}
	return k;
}

static void cli_clear_line(int sockfd, char *cmd, int l, int cursor)
{
	int i, n;
	if (cursor < l) for (i = 0; i < (l - cursor); i++) n = write(sockfd, " ", 1);
	for (i = 0; i < l; i++) cmd[i] = '\b';
	for (; i < l * 2; i++) cmd[i] = ' ';
	for (; i < l * 3; i++) cmd[i] = '\b';
	n = write(sockfd, cmd, i);
	memset(cmd, 0, i);
	l = cursor = 0;

	if (!n)
	{

	}
}

void cli_reprompt(struct cli_def *cli)
{
	if (!cli) return;
	cli->showprompt = 1;
}

void cli_regular(struct cli_def *cli, int (*callback)(struct cli_def *cli))
{
	if (!cli) return;
	cli->regular_callback = callback;
}

#define DES_PREFIX "{crypt}"		/* to distinguish clear text from DES crypted */
#define MD5_PREFIX "$1$"

static int pass_matches(char *pass, char *try_pass)
{
	int des;

	if ((des = !strncasecmp(pass, DES_PREFIX, sizeof(DES_PREFIX)-1)))
		pass += sizeof(DES_PREFIX)-1;

	if (des || !strncmp(pass, MD5_PREFIX, sizeof(MD5_PREFIX)-1))
		try_pass = crypt(try_pass, pass);

	return !strcmp(pass, try_pass);
}

#define CTRL(c) (c - '@')

static int show_prompt(struct cli_def *cli, int sockfd)
{
	int len = 0;

	if (cli->hostname)
		len += write(sockfd, cli->hostname, strlen(cli->hostname));

	if (cli->modestring)
		len += write(sockfd, cli->modestring, strlen(cli->modestring));

	return len + write(sockfd, cli->promptchar, strlen(cli->promptchar));
}

int cli_loop(struct cli_def *cli, int sockfd)
{
		return cli_loop_ex(cli, sockfd, -1, -1);
}

int cli_loop_ex(struct cli_def *cli, int sockfd, int listenfd, int listenfd2)
{
	unsigned char c;
	int maxfd = -1;
	int n, l, oldl = 0, is_telnet_option = 0, skip = 0, esc = 0;
	int cursor = 0, insertmode = 1;
	char *cmd = NULL, *oldcmd = 0;
	char *username = NULL, *password = NULL;
	char *negotiate =
		"\xFF\xFB\x03"
		"\xFF\xFB\x01"
		"\xFF\xFD\x03"
		"\xFF\xFD\x01";

	cli->state = STATE_LOGIN;//STATE_PASSWORD;/*STATE_LOGIN;*/

	cli_free_history(cli);
	n = write(sockfd, negotiate, strlen(negotiate));

	if ((cmd = (char *)malloc(4096)) == NULL)
		return CLI_ERROR;

	if (!(cli->client = fdopen(sockfd, "w+")))
		return CLI_ERROR;

	setbuf(cli->client, NULL);
	if (cli->banner)
		cli_error(cli, "%s", cli->banner);

	/* start off in unprivileged mode */
	cli_set_privilege(cli, PRIVILEGE_UNPRIVILEGED);
	cli_set_configmode(cli, MODE_EXEC, NULL);

	/* no auth required? */
	if (!cli->users && !cli->auth_callback)
		cli->state = STATE_NORMAL;

	while (1)
	{
		signed int in_history = 0;
		int lastchar = 0;
		struct timeval tm;

		cli->showprompt = 1;

		if (oldcmd)
		{
			l = cursor = oldl;
			oldcmd[l] = 0;
			cli->showprompt = 1;
			oldcmd = NULL;
			oldl = 0;
		}
		else
		{
			memset(cmd, 0, 4096);
			l = 0;
			cursor = 0;
		}

		tm.tv_sec = 1;
		tm.tv_usec = 0;

		while (1)
		{
			int sr;
			fd_set r;
			if (cli->showprompt)
			{
				//if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
				//	write(sockfd, "\r\n", 2);

				switch (cli->state)
				{
					case STATE_LOGIN:
						n = write(sockfd, "Username: ", strlen("Username: "));
						break;

					case STATE_PASSWORD:
						n = write(sockfd, "Password: ", strlen("Password: "));
						break;

					case STATE_NORMAL:
					case STATE_ENABLE:
						show_prompt(cli, sockfd);
						n = write(sockfd, cmd, l);
						if (cursor < l)
						{
							int n = l - cursor;
							while (n--)
								n = write(sockfd, "\b", 1);
						}
						break;

					case STATE_ENABLE_PASSWORD:
						n = write(sockfd, "Password: ", strlen("Password: "));
						break;

				}

				cli->showprompt = 0;
			}

			FD_ZERO(&r);
			FD_SET(sockfd, &r);
			if(listenfd > 0) FD_SET(listenfd, &r);
			if(listenfd2 > 0) FD_SET(listenfd2, &r);

			maxfd = sockfd;
			if(listenfd > maxfd) maxfd = listenfd;
			if(listenfd2 > maxfd) maxfd = listenfd2;

			if ((sr = select(maxfd + 1, &r, NULL, NULL, &tm)) < 0)
			{
				/* select error */
				if (errno == EINTR)
					continue;

				perror("read");
				l = -1;
				break;
			}

			if((listenfd > 0) && FD_ISSET(listenfd, &r) )
			{
					/* another session requested */
					/* break this one */
					l = -1;
					break;
			}

			if((listenfd2 > 0) && FD_ISSET(listenfd2, &r) )
			{
					/* another session requested */
					/* break this one */
					l = -1;
					break;
			}

			if (sr == 0)
			{
				/* timeout every second */
				if (cli->regular_callback && cli->regular_callback(cli) != CLI_OK)
					break;

				tm.tv_sec = 1;
				tm.tv_usec = 0;
				continue;
			}

			if ((n = read(sockfd, &c, 1)) < 0)
			{
				if (errno == EINTR)
					continue;

				perror("read");
				l = -1;
				break;
			}

			if (n == 0)
			{
				l = -1;
				break;
			}

			if (skip)
			{
				skip--;
				continue;
			}

			if (c == 255 && !is_telnet_option)
			{
				is_telnet_option++;
				continue;
			}

			if (is_telnet_option)
			{
				if (c >= 251 && c <= 254)
				{
					is_telnet_option = c;
					continue;
				}

				if (c != 255)
				{
					is_telnet_option = 0;
					continue;
				}

				is_telnet_option = 0;
			}

			/* handle ANSI arrows */
			if (esc)
			{
				if (esc == '[')
				{
					/* remap to readline control codes */
					switch (c)
					{
						case 'A': /* Up */
							c = CTRL('P');
							break;

						case 'B': /* Down */
							c = CTRL('N');
							break;

						case 'C': /* Right */
							c = CTRL('F');
							break;

						case 'D': /* Left */
							c = CTRL('B');
							break;

						default:
							c = 0;
					}

					esc = 0;
				}
				else
				{
					esc = (c == '[') ? c : 0;
					continue;
				}
			}

			if (c == 0) continue;
			if (c == '\n') continue;

			if (c == '\r')
			{
				if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
					n = write(sockfd, "\r\n", 2);
				break;
			}

			if (c == 27)
			{
				esc = 1;
				continue;
			}

			if (c == CTRL('C'))
			{
				n = write(sockfd, "\a", 1);
				continue;
			}

			/* back word, backspace/delete */
			if (c == CTRL('W') || c == CTRL('H') || c == 0x7f)
			{
				int back = 0;

				if (c == CTRL('W')) /* word */
				{
					int nc = cursor;

					if (l == 0 || cursor == 0)
						continue;

					while (nc && cmd[nc - 1] == ' ')
					{
						nc--;
						back++;
					}

					while (nc && cmd[nc - 1] != ' ')
					{
						nc--;
						back++;
					}
				}
				else /* char */
				{
					if (l == 0 || cursor == 0)
					{
						n = write(sockfd, "\a", 1);
						continue;
					}

					back = 1;
				}

				if (back)
				{
					while (back--)
					{
						if (l == cursor)
						{
							cmd[--cursor] = 0;
							if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
								n = write(sockfd, "\b \b", 3);
						}
						else
						{
							int i;
							cursor--;
							if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
							{
								for (i = cursor; i <= l; i++) cmd[i] = cmd[i+1];
								n = write(sockfd, "\b", 1);
								n = write(sockfd, cmd + cursor, strlen(cmd + cursor));
								n = write(sockfd, " ", 1);
								for (i = 0; i <= (int)strlen(cmd + cursor); i++)
									n = write(sockfd, "\b", 1);
							}
						}
						l--;
					}

					continue;
				}
			}

			/* redraw */
			if (c == CTRL('L'))
			{
				int i;
				int cursorback = l - cursor;

				if (cli->state == STATE_PASSWORD || cli->state == STATE_ENABLE_PASSWORD)
					continue;

				n = write(sockfd, "\r\n", 2);
				show_prompt(cli, sockfd);
				n = write(sockfd, cmd, l);

				for (i = 0; i < cursorback; i++)
					n = write(sockfd, "\b", 1);

				continue;
			}

			/* clear line */
			if (c == CTRL('U'))
			{
				if (cli->state == STATE_PASSWORD || cli->state == STATE_ENABLE_PASSWORD)
					memset(cmd, 0, l);
				else
					cli_clear_line(sockfd, cmd, l, cursor);

				l = cursor = 0;
				continue;
			}

			/* kill to EOL */
			if (c == CTRL('K'))
			{
				if (cursor == l)
					continue;

				if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
				{
					int c;
					for (c = cursor; c < l; c++)
						n = write(sockfd, " ", 1);

					for (c = cursor; c < l; c++)
						n = write(sockfd, "\b", 1);
				}

				memset(cmd + cursor, 0, l - cursor);
				l = cursor;
				continue;
			}

			/* EOT */
			if (c == CTRL('D'))
			{
				if (cli->state == STATE_PASSWORD || cli->state == STATE_ENABLE_PASSWORD)
					break;

				if (l)
					continue;

				strcpy(cmd, "quit");
				l = cursor = strlen(cmd);
				n = write(sockfd, "quit\r\n", l + 2);
				break;
			}

			/* disable */
			if (c == CTRL('Z'))
			{
				if (cli->mode != MODE_EXEC)
				{
					cli_clear_line(sockfd, cmd, l, cursor);
					cli_set_configmode(cli, MODE_EXEC, NULL);
					cli->showprompt = 1;
				}

				continue;
			}

			/* TAB completion */
			if (c == CTRL('I'))
			{
				char *completions[128];
				int num_completions = 0;

				if (cli->state == STATE_PASSWORD || cli->state == STATE_ENABLE_PASSWORD)
					continue;

				if (cursor != l) continue;

				if (l > 0)
					num_completions = cli_get_completions(cli, cmd, completions, 128);

				if (num_completions == 0)
				{
					n = write(sockfd, "\a", 1);
				}
				else if (num_completions == 1)
				{
					// Single completion
					for (; l > 0; l--, cursor--)
					{
						if (cmd[l-1] == ' ' || cmd[l-1] == '|')
							break;
						n = write(sockfd, "\b", 1);
					}
					strcpy((cmd + l), completions[0]);
					l += strlen(completions[0]);
					cmd[l++] = ' ';
					cursor = l;
					n = write(sockfd, completions[0], strlen(completions[0]));
					n = write(sockfd, " ", 1);
				}
				else if (lastchar == CTRL('I'))
				{
					// double tab
					int i;
					n = write(sockfd, "\r\n", 2);
					for (i = 0; i < num_completions; i++)
					{
						n = write(sockfd, completions[i], strlen(completions[i]));
						if (i % 4 == 3)
							n = write(sockfd, "\r\n", 2);
						else
							n = write(sockfd, "	 ", 1);
					}
					if (i % 4) n = write(sockfd, "\r\n", 2);
						cli->showprompt = 1;
				}
				else
				{
					// More than one completion
					lastchar = c;
					n = write(sockfd, "\a", 1);
				}
				continue;
			}

			/* history */
			if (c == CTRL('P') || c == CTRL('N'))
			{
				int history_found = 0;

				if (cli->state == STATE_PASSWORD || cli->state == STATE_ENABLE_PASSWORD)
					continue;

				if (c == CTRL('P')) // Up
				{
					in_history--;
					if (in_history < 0)
					{
						for (in_history = MAX_HISTORY-1; in_history >= 0; in_history--)
						{
							if (cli->history[in_history])
							{
								history_found = 1;
								break;
							}
						}
					}
					else
					{
						if (cli->history[in_history]) history_found = 1;
					}
				}
				else // Down
				{
					in_history++;
					if (in_history >= MAX_HISTORY || !cli->history[in_history])
					{
						int i = 0;
						for (i = 0; i < MAX_HISTORY; i++)
						{
							if (cli->history[i])
							{
								in_history = i;
								history_found = 1;
								break;
							}
						}
					}
					else
					{
						if (cli->history[in_history]) history_found = 1;
					}
				}
				if (history_found && cli->history[in_history])
				{
					// Show history item
					cli_clear_line(sockfd, cmd, l, cursor);
					memset(cmd, 0, 4096);
					strncpy(cmd, cli->history[in_history], 4095);
					l = cursor = strlen(cmd);
					n = write(sockfd, cmd, l);
				}

				continue;
			}

			/* left/right cursor motion */
			if (c == CTRL('B') || c == CTRL('F'))
			{
				if (c == CTRL('B')) /* Left */
				{
					if (cursor)
					{
						if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
							n = write(sockfd, "\b", 1);

						cursor--;
					}
				}
				else /* Right */
				{
					if (cursor < l)
					{
						if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
							n = write(sockfd, &cmd[cursor], 1);

						cursor++;
					}
				}

				continue;
			}

			/* start of line */
			if (c == CTRL('A'))
			{
				if (cursor)
				{
					if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
					{
						n = write(sockfd, "\r", 1);
						show_prompt(cli, sockfd);
					}

					cursor = 0;
				}

				continue;
			}

			/* end of line */
			if (c == CTRL('E'))
			{
				if (cursor < l)
				{
					if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
						n = write(sockfd, &cmd[cursor], l - cursor);

					cursor = l;
				}

				continue;
			}

			/* normal character typed */
			if (cursor == l)
			{
				 /* append to end of line */
				cmd[cursor] = c;
				if (l < 4095)
				{
					l++;
					cursor++;
				}
				else
				{
					n = write(sockfd, "\a", 1);
					continue;
				}
			}
			else
			{
				// Middle of text
				if (insertmode)
				{
					int i;
					// Move everything one character to the right
					if (l >= 4094) l--;
					for (i = l; i >= cursor; i--)
						cmd[i + 1] = cmd[i];
					// Write what we've just added
					cmd[cursor] = c;

					n = write(sockfd, &cmd[cursor], l - cursor + 1);
					for (i = 0; i < (l - cursor + 1); i++)
						n = write(sockfd, "\b", 1);
					l++;
				}
				else
				{
					cmd[cursor] = c;
				}
				cursor++;
			}

			if (cli->state != STATE_PASSWORD && cli->state != STATE_ENABLE_PASSWORD)
			{
				if (c == '?' && cursor == l)
				{
					n = write(sockfd, "\r\n", 2);
					oldcmd = cmd;
					oldl = cursor = l - 1;
					break;
				}
				n = write(sockfd, &c, 1);
			}

			oldcmd = 0;
			oldl = 0;
			lastchar = c;
		}

		if (l < 0) break;
		if (!strcasecmp(cmd, "quit")) break;

		if (cli->state == STATE_LOGIN)
		{
			if (l == 0) continue;

			/* require login */
			free_z(username);
			username = strdup(cmd);
			cli->state = STATE_PASSWORD;
			cli->showprompt = 1;
		}
		else if (cli->state == STATE_PASSWORD)
		{
			/* require password */
			int allowed = 0;

			free_z(password);
			password = strdup(cmd);
			if (cli->auth_callback)
			{
				if (cli->auth_callback(username, password) == CLI_OK)
					allowed++;
			}

			if (!allowed)
			{
				struct unp *u;
				for (u = cli->users; u; u = u->next)
				{
					if (!strcmp(u->username, username) && pass_matches(u->password, password))
					{
						allowed++;
						break;
					}
				}
			}

			if (allowed)
			{
				cli_error(cli, "\nSuccessful login.\n\n");
				cli->state = STATE_NORMAL;
			}
			else
			{
				cli_error(cli, "\n\nAccess denied");
				free_z(username);
				free_z(password);
				cli->state = STATE_LOGIN;//STATE_PASSWORD;/*STATE_LOGIN;*/
			}

			cli->showprompt = 1;
		}
		else if (cli->state == STATE_ENABLE_PASSWORD)
		{
			int allowed = 0;
			if (cli->enable_password)
			{
				/* check stored static enable password */
				if (pass_matches(cli->enable_password, cmd))
					allowed++;
			}

			if (!allowed && cli->enable_callback)
			{
				/* check callback */
				if (cli->enable_callback(cmd))
					allowed++;
			}

			if (allowed)
			{
				cli->state = STATE_ENABLE;
				cli_set_privilege(cli, PRIVILEGE_PRIVILEGED);
			}
			else
			{
				cli_error(cli, "\n\nAccess denied");
				cli->state = STATE_NORMAL;
			}
		}
		else
		{
			if (l == 0) continue;
			if (cmd[l - 1] != '?' && strcasecmp(cmd, "history") != 0)
				cli_add_history(cli, cmd);

			if (cli_run_command(cli, cmd) == CLI_QUIT)
				break;
		}
	}

	cli_free_history(cli);
	free_z(username);
	free_z(password);
	free_z(cmd);

	fclose(cli->client);
	cli->client = 0;
	return CLI_OK;
}

int cli_file(struct cli_def *cli, FILE *fh, int privilege, int mode)
{
	int oldpriv = cli_set_privilege(cli, privilege);
	int oldmode = cli_set_configmode(cli, mode, NULL);
	char buf[4096];

	while (1)
	{
		char *p;
		char *cmd;
		char *end;

		if (fgets(buf, sizeof(buf), fh) == NULL)
			break; /* end of file */

		if ((p = strpbrk(buf, "#\r\n")))
			*p = 0;

		cmd = buf;
		while (isspace(*cmd))
			cmd++;

		if (!*cmd)
			continue;

		for (p = end = cmd; *p; p++)
			if (!isspace(*p))
				end = p;

		*++end = 0;
		if (strcasecmp(cmd, "quit") == 0)
			break;

		if (cli_run_command(cli, cmd) == CLI_QUIT)
			break;
	}

	cli_set_privilege(cli, oldpriv);
	cli_set_configmode(cli, oldmode, NULL /* didn't save desc */);

	return CLI_OK;
}

static void _print(struct cli_def *cli, int print_mode, char *format, va_list ap)
{
	static char *buffer;
	static int size, len;
	char *p;
	int n;

	if (!cli) return; // sanity check

	buffer = cli->buffer;
	size = cli->buf_size;
	len = strlen(buffer);

	while ((n = vsnprintf(buffer+len, size-len, format, ap)) >= size-len)
	{
				if (!(buffer = (char *)realloc(buffer, size += 1024)))
					return;
		
				cli->buffer = buffer;
				cli->buf_size = size;
	}

	if (n < 0) // vaprintf failed
		return;
		
//sprintf(str, "Show slot %02d\n", slot);
		
	p = buffer;
	do
	{
				char *next = strchr(p, '\n');
		/*		if(next) 
				{
						next++;
						if(*next == '\r') next++;
				}
		*/		
				struct cli_filter *f = (print_mode&PRINT_FILTERED) ? cli->filters : 0;
				int print = 1;
		
				if (next)
					*next++ = 0;
				else if (print_mode&PRINT_BUFFERED)
					break;
		
		
				while (print && f)
				{
					print = (f->filter(cli, p, f->data) == CLI_OK);
					f = f->next;
				}
				if (print)
				{
					if (cli->print_callback)
						cli->print_callback(cli, p);
					else if (cli->client)
					{		
							fprintf(cli->client, "%s\r\n", p);
						}
		
				}
		
				if(next && !strlen(next))
				{
						p = NULL;
						break;
				}else{
						p = next;
				}
				
	} while (p);

	if (p && *p)
	{
				if (p != buffer)
				bcopy(p, buffer, strlen(p));
	}
	else *buffer = 0;
}

void cli_bufprint(struct cli_def *cli, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_print(cli, PRINT_BUFFERED|PRINT_FILTERED, format, ap);
	va_end(ap);
}

void cli_vabufprint(struct cli_def *cli, char *format, va_list ap)
{
	_print(cli, PRINT_BUFFERED, format, ap);
}

void cli_print(struct cli_def *cli, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_print(cli, PRINT_FILTERED, format, ap);
	va_end(ap);
}

void cli_error(struct cli_def *cli, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_print(cli, PRINT_PLAIN, format, ap);
	va_end(ap);
}

struct cli_match_filter_state
{
	int flags;
#define MATCH_REGEX				1
#define MATCH_INVERT				2
	union {
		char *string;
		regex_t re;
	} match;
};

int cli_match_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt)
{
	struct cli_match_filter_state *state;
	int rflags;
	int i;
	char *p;

	if (argc < 2)
	{
		if (cli->client)
			fprintf(cli->client, "Match filter requires an argument\r\n");

		return CLI_ERROR;
	}

	filt->filter = cli_match_filter;
	filt->data = state = (cli_match_filter_state*)calloc(sizeof(struct cli_match_filter_state), 1);

	if (argv[0][0] == 'i' || // include/exclude
		(argv[0][0] == 'e' && argv[0][1] == 'x'))
	{
		if (argv[0][0] == 'e')
			state->flags = MATCH_INVERT;

		state->match.string = join_words(argc-1, argv+1);
		return CLI_OK;
	}

	state->flags = MATCH_REGEX;

	// grep/egrep
	rflags = REG_NOSUB;
	if (argv[0][0] == 'e') // egrep
		rflags |= REG_EXTENDED;

	i = 1;
	while (i < argc - 1 && argv[i][0] == '-' && argv[i][1])
	{
		int last = 0;
		p = &argv[i][1];

		if (strspn(p, "vie") != strlen(p))
			break;

		while (*p)
		{
			switch (*p++)
			{
				case 'v':
					state->flags |= MATCH_INVERT;
					break;

				case 'i':
					rflags |= REG_ICASE;
					break;

				case 'e':
					last++;
					break;
			}
		}

		i++;
		if (last)
			break;
	}

	p = join_words(argc-i, argv+i);
	if ((i = regcomp(&state->match.re, p, rflags)))
	{
		if (cli->client)
			fprintf(cli->client, "Invalid pattern \"%s\"\r\n", p);

		free_z(p);
		return CLI_ERROR;
	}

	free_z(p);
	return CLI_OK;
}

int cli_match_filter(UNUSED(struct cli_def *cli), char *string, void *data)
{
	struct cli_match_filter_state *state = (struct cli_match_filter_state*)data;
	int r = CLI_ERROR;

	if (!string) // clean up
	{
		if (state->flags & MATCH_REGEX)
			regfree(&state->match.re);
		else
			free(state->match.string);

		free(state);
		return CLI_OK;
	}

	if (state->flags & MATCH_REGEX)
	{
		if (!regexec(&state->match.re, string, 0, NULL, 0))
			r = CLI_OK;
	}
	else
	{
		if (strstr(string, state->match.string))
			r = CLI_OK;
	}

	if (state->flags & MATCH_INVERT)
	{
		if (r == CLI_OK)
			r = CLI_ERROR;
		else
			r = CLI_OK;
	}

	return r;
}

struct cli_range_filter_state {
	int matched;
	char *from;
	char *to;
};

int cli_range_filter_init(struct cli_def *cli, int argc, char **argv, struct cli_filter *filt)
{
	struct cli_range_filter_state *state;
	char *from = 0;
	char *to = 0;

	if (!strncmp(argv[0], "bet", 3)) // between
	{
		if (argc < 3)
		{
			if (cli->client)
				fprintf(cli->client, "Between filter requires 2 arguments\r\n");

			return CLI_ERROR;
		}

		from = strdup(argv[1]);
		to = join_words(argc-2, argv+2);
	}
	else // begin
	{
		if (argc < 2)
		{
			if (cli->client)
				fprintf(cli->client, "Begin filter requires an argument\r\n");

			return CLI_ERROR;
		}

		from = join_words(argc-1, argv+1);
	}

	filt->filter = cli_range_filter;
	filt->data = state = (struct cli_range_filter_state*)calloc(sizeof(struct cli_range_filter_state), 1);

	state->from = from;
	state->to = to;

	return CLI_OK;
}

int cli_range_filter(UNUSED(struct cli_def *cli), char *string, void *data)
{
	struct cli_range_filter_state *state = (struct cli_range_filter_state*)data;
	int r = CLI_ERROR;

	if (!string) // clean up
	{
		free_z(state->from);
		free_z(state->to);
		free_z(state);
		return CLI_OK;
	}

	if (!state->matched)
	state->matched = !!strstr(string, state->from);

	if (state->matched)
	{
		r = CLI_OK;
		if (state->to && strstr(string, state->to))
			state->matched = 0;
	}

	return r;
}

int cli_count_filter_init(struct cli_def *cli, int argc, UNUSED(char **argv), struct cli_filter *filt)
{
	if (argc > 1)
	{
		if (cli->client)
			fprintf(cli->client, "Count filter does not take arguments\r\n");

		return CLI_ERROR;
	}

	filt->filter = cli_count_filter;
	filt->data = calloc(sizeof(int), 1);

	return CLI_OK;
}

int cli_count_filter(struct cli_def *cli, char *string, void *data)
{
	int *count = (int *)data;

	if (!string) // clean up
	{
		// print count
		if (cli->client)
			fprintf(cli->client, "%d\r\n", *count);

		free(count);
		return CLI_OK;
	}

	while (isspace(*string))
		string++;

	if (*string)
		(*count)++;  // only count non-blank lines

	return CLI_ERROR; // no output
}

void cli_print_callback(struct cli_def *cli, void (*callback)(struct cli_def *, char *))
{
	cli->print_callback = callback;
}

