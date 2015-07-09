#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "logger.h"
#include "pbyte_transport.h"

void init_thread_test(void)
{
	log_debug("%s", __func__);
}

static int message_handler(pb_dialog_t *dialog, char *data, int size)
{
	log_info("recv new message");

	return 0;
}

int main(void)
{
	int res = -1, wait_count = 60;
	pb_transport_t pbyte_client;
	pb_params_t client_params;
	char asd[] = "cli test";

	memset(&pbyte_client, 0, sizeof (pbyte_client));
	memset(&client_params, 0, sizeof (client_params));

	client_params.mode = ePBYTE_CLIENT;
	client_params.io_threads = 1;
	client_params.addr = "5.128.41.70";
	client_params.port = 1230;
	client_params.logger_prio = 99;

	client_params.logger = &log_print;
	client_params.thread_init = &init_thread_test;

	client_params.msg_cb.client_msg_handler = &message_handler;

	res = pb_transport_start(&pbyte_client, &client_params);
	if (res != 0)
	{
		log_error("failed to start client [%d]", res);
		return -1;
	}

	while (wait_count > 0)
	{
		sleep(1);
		wait_count--;

		pb_transport_send(&pbyte_client, -1, NULL, asd, sizeof (asd));
	}

	pb_transport_stop(&pbyte_client);

	return 0;
}