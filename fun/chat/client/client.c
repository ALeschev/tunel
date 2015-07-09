#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "logger.h"
#include "pbyte_transport.h"

void init_thread_test(void)
{
	log_debug("%s", __func__);
}

static int message_handler(pb_dialog_t *dialog, char *data, int size)
{
	log_info("recv new message: '%s'", data? data:"NULL");

	return 0;
}

int main(void)
{
	int res = -1, wait_count = 60;
	int size;
	pb_transport_t pbyte_client;
	pb_params_t client_params;
	char buff[1024] = {0};

	memset(&pbyte_client, 0, sizeof (pbyte_client));
	memset(&client_params, 0, sizeof (client_params));

	client_params.mode = ePBYTE_CLIENT;
	client_params.io_threads = 1;
	client_params.addr = "127.0.0.1";
	client_params.port = 1230;
	client_params.logger_prio = eLOG_ERR;

	client_params.logger = &log_print;
	client_params.thread_init = &init_thread_test;

	client_params.msg_cb.client_msg_handler = &message_handler;

	res = pb_transport_start(&pbyte_client, &client_params);
	if (res != 0)
	{
		log_error("failed to start client [%d]", res);
		return -1;
	}

	while(fgets(buff, 1023, stdin)) /* while(1) ? :) */
	{
		printf ("> ");

		size = strlen(buff);

		if ((size == 1) && (buff[0] == '\n'))
		{
			continue;
		}
		// scanf("%1023s", buff);
		// sleep(1);
		// wait_count--;

		while(size && (buff[size-1] == '\n'))
			size--;

		buff[size++] = 0;

		if (!strcmp(buff, "close"))
		{
			break;
		}

		pb_transport_send(&pbyte_client, -1, NULL, buff, strlen (buff));

		memset(buff, 0, sizeof (buff));
		fflush(stdin);
	}

	pb_transport_stop(&pbyte_client);

	return 0;
}