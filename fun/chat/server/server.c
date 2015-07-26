/*
 * LAV by fun
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "logger.h"
#include "client_base.h"
#include "pbyte_transport.h"
#include "libcli.h"

#define CSR_IP   "127.0.0.1"
#define CSR_PORT 1231

#define KEY_ARROW_UP    1
#define KEY_ARROW_DOWN  2
#define KEY_ARROW_LEFT  3
#define KEY_ARROW_RIGHT 4
#define KEY_ESC         5

#define get_array_size(array) sizeof(array)/sizeof(array[0])

static int need_exit = 0;
static pb_transport_t transport;
static c_client_base_t client_base = {NULL, 0};
static struct cli_def *cli;
static int consol_trace_enable = 0;

void init_thread_test(void)
{
	log_debug("%s", __func__);
}

static int message_handler(int identity_idx, pb_dialog_t *dialog, char *data, int size)
{
	log_info("recv new message from %d: '%s'", identity_idx, data? data:"NULL");

	pb_transport_send(&transport, identity_idx, dialog, data, size);

	return 0;
}

void c_puts(char *str)
{
	cli_print(cli, "%s", str);
}

int trace_enable(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if (argc)
	{
		if (!strcmp(argv[0], "start"))
		{
			log_register_logger(c_puts);
			consol_trace_enable = 1;
			log_info("Start tracing to console");
			return CLI_OK;
		}
		else if (!strcmp(argv[0], "stop"))
		{
			log_deregister_logger();
			consol_trace_enable = 0;
			log_info("Stop tracing to console");
			return CLI_OK;
		}
	}

	log_error("Argument error. Usage: start/stop");

	return CLI_ERROR;
}

int bcase_send(struct cli_def *cli, char *command, char *argv[], int argc)
{
	if (argc == 1)
	{
		pb_transport_broadcast(&transport, NULL, argv[0], strlen (argv[0]));
		return CLI_OK;
	}

	log_error("Argument error. Usage: %s <message>", command);

	return CLI_ERROR;
}

int show_clients(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int trace_enable = 0;

	if (!consol_trace_enable)
	{
		log_register_logger(c_puts);
		trace_enable++;
	}

	c_base_show(&client_base, 1);

	if (trace_enable)
	{
		log_deregister_logger();
	}

	return CLI_OK;
}

int show_connections(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int trace_enable = 0;

	if (!consol_trace_enable)
	{
		log_register_logger(c_puts);
		trace_enable++;
	}

	pb_transport_show_connections(&transport);

	if (trace_enable)
	{
		log_deregister_logger();
	}

	return CLI_OK;
}

int message_send(struct cli_def *cli, char *command, char *argv[], int argc)
{
	c_client_t *client;

	if (argc == 2)
	{

		client = c_client_get_by_id(&client_base, atoi(argv[0]));
		if (client)
		{
			pb_transport_send (&transport, client->trid, NULL, argv[1], strlen(argv[1]));
			return CLI_OK;
		}
		else
		{
			log_error("Failed to send message: "
			          "client with id %d not found",
			          atoi(argv[0]));
			return CLI_ERROR;
		}
	}

	log_error("Argument error. Usage: %s <client id> <message>", command);

	return CLI_ERROR;
}

int stop_server(struct cli_def *cli, char *command, char *argv[], int argc)
{
	pb_transport_stop(&transport);
	c_base_close(&client_base);
	cli_done(cli);

	exit(0);

	return CLI_OK; /* all fine c: */
}

int restart_server(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int res;
	int trace_enable = 0;
	pb_params_t params;

	if (!consol_trace_enable)
	{
		log_register_logger(c_puts);
		trace_enable++;
	}

	pb_transport_stop(&transport);
	c_base_close(&client_base);

	memset(&params, 0, sizeof (params));
	memset(&transport, 0, sizeof (transport));

	params.mode = ePBYTE_SERVER;
	params.io_threads = 1;
	params.addr = "127.0.0.1";
	params.port = 1230;
	params.logger_prio = eLOG_WARN;
	params.ack_timeout = 1;
	params.conn_check_timeout = 5;
	params.worker_count = 1;

	params.msg_cb.server_msg_handler = &message_handler;
	// params.err_cb.server_error_handler

	params.logger = &log_print;
	params.thread_init = &init_thread_test;

	res = pb_transport_start(&transport, &params);
	if (res != 0)
	{
		log_error("failed to start server [%d]", res);
		stop_server(cli, command, argv, argc);
		return CLI_ERROR;
	}

	res = c_base_init(&client_base, MAX_CLIENTS);
	if (res != 0)
	{
		log_error("failed to init client base [%d]", res);
		stop_server(cli, command, argv, argc);
		return CLI_ERROR;
	}

	if (trace_enable)
		log_deregister_logger();

	return CLI_OK;
}

int set_trace_level(struct cli_def *cli, char *command, char *argv[], int argc)
{
	int i, offset = 0;
	char level_str[512] = {0};

	if (argc == 1)
	{
		for (i = 0; i < eLOG_MAX, offset < strlen(level_str); i++)
		{
			if (!strcmp(e_log_level_str[i], argv[0]))
			{
				pb_transport_server_set_log_prio(&transport, i);
				return CLI_OK;
			}
		}
	}

	for (i = 0; i < eLOG_MAX, offset < strlen(level_str); i++)
	{
		strcat(level_str[offset], e_log_level_str[i]);
		offset += strlen(e_log_level_str[i]);
	}

	log_error("Argument error. Usage: %s %s", command, level_str);

	return CLI_ERROR;
}

int main(void)
{
	int wait_count = 120;
	int res;
	int i, key;
	int size;
	char buff[1024] = {0};
	char nickname[MAX_STR_LEN] = {0};
	c_client_t *test_client;
	pb_params_t params;

	memset(&params, 0, sizeof (params));
	memset(&transport, 0, sizeof (transport));

	log_info("test");

	params.mode = ePBYTE_SERVER;
	params.io_threads = 1;
	params.addr = "127.0.0.1";
	params.port = 1230;
	params.logger_prio = eLOG_WARN;
	params.ack_timeout = 1;
	params.conn_check_timeout = 5;
	params.worker_count = 1;

	params.msg_cb.server_msg_handler = &message_handler;
	// params.err_cb.server_error_handler

	params.logger = &log_print;
	params.thread_init = &init_thread_test;

	res = pb_transport_start(&transport, &params);
	if (res != 0)
	{
		log_error("failed to start server [%d]", res);
		return -1;
	}

	res = c_base_init(&client_base, MAX_CLIENTS);
	if (res != 0)
	{
		log_error("failed to init client base [%d]", res);
		return -2;
	}

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		sprintf (nickname, "test_nick%d", i);
		test_client = c_client_add(&client_base, i, nickname, strlen(nickname));
		if (!test_client)
		{
			log_error("failed to add client: %d %s", i, nickname);
			continue;
		}
	}

#if 1 /* replace this to consol.c */
	struct cli_command *c;
	int s, x;
	struct sockaddr_in servaddr;
	int on = 1;

	cli = cli_init();
	cli_set_banner(cli, "CSR. Command Line Interface. (C)2014-2015 Insic Ltd.");
	cli_set_hostname(cli, "CSR");
	cli_allow_user(cli, "a", "a");

	cli_register_command(cli, NULL, "trace", trace_enable, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "Enable/disable tracing to console");

	cli_register_command(cli, NULL, "bcast", bcase_send, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "Send broadcast message to all clients");

	cli_register_command(cli, NULL, "clients", show_clients, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "Show connected clients");

	cli_register_command(cli, NULL, "connections", show_connections, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "Show transport connections");

	cli_register_command(cli, NULL, "send", message_send, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "Send message to client");

	cli_register_command(cli, NULL, "shutdown", stop_server, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "Shutdown server");

	cli_register_command(cli, NULL, "restart", restart_server, PRIVILEGE_UNPRIVILEGED,
		MODE_EXEC, "restart server");

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		log_error("socket: %s", strerror(errno));
		return 1;
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(CSR_IP);
	servaddr.sin_port = htons(CSR_PORT);
	if (bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		log_error("bind: %s", strerror(errno));
		return 1;
	}

	if (listen(s, 50) < 0)
	{
		log_error("listen: %s", strerror(errno));
		return 1;
	}

	log_info("Listening on port %d", CSR_PORT);
	while ((x = accept(s, NULL, 0)))
	{
		cli_loop(cli, x);
		close(x);
	}
	cli_done(cli);
#endif
	// return 0;

	// while((key = castom_getchar(buff, 1023)) >= 0) /* while(1) ? :) */
	// {
	// 	printf ("> ");

	// 	switch(key)
	// 	{
	// 		case KEY_ARROW_UP   : printf ("KEY_ARROW_UP   \n"); continue;
	// 		case KEY_ARROW_DOWN : printf ("KEY_ARROW_DOWN \n"); continue;
	// 		case KEY_ARROW_LEFT : printf ("KEY_ARROW_LEFT \n"); continue;
	// 		case KEY_ARROW_RIGHT: printf ("KEY_ARROW_RIGHT\n"); continue;
	// 		case KEY_ESC        : printf ("KEY_ESC        \n"); continue;
	// 	}

	// 	size = strlen(buff);

	// 	while(size && (buff[size - 1] == '\n'))
	// 		size--;

	// 	buff[size++] = 0;

	// 	proc_command_message(buff);

	// 	// pb_transport_broadcast(&transport, NULL, buff, strlen (buff));

	// 	memset(buff, 0, sizeof (buff));
	// 	fflush(stdin);

	// 	if (need_exit)
	// 		break;
	// }

	c_base_close(&client_base);

	// pb_transport_stop(&transport);

	return 0;
}