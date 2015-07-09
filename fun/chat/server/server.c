/*
 * LAV by fun
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"
#include "client_base.h"
#include "pbyte_transport.h"

static pb_transport_t transport;

void init_thread_test(void)
{
	log_debug("%s", __func__);
}

static int message_handler(int identity_idx, pb_dialog_t *dialog, char *data, int size)
{
	log_info("recv new message from %d", identity_idx);

	pb_transport_send(&transport, identity_idx, dialog, data, size);

	return 0;
}

int main(void)
{
	int wait_count = 120;
	int res;
	int i;
	char nickname[MAX_STR_LEN] = {0};
	c_client_t *test_client;
	c_client_base_t client_base = {NULL, 0};
	pb_params_t params;

	memset(&params, 0, sizeof (params));
	memset(&transport, 0, sizeof (transport));

	log_info("test");

	memset(&transport, 0, sizeof(transport));
	memset(&params, 0, sizeof(params));

	params.mode = ePBYTE_SERVER;
	params.io_threads = 1;
	params.addr = "192.168.1.34";
	params.port = 1234;
	params.logger_prio = 99;
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

	while (wait_count > 0)
	{
		sleep(1);
		wait_count--;
	}

	c_base_close(&client_base);

	pb_transport_stop(&transport);

	return 0;
}