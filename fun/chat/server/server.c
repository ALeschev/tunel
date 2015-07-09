/*
 * LAV by fun
 */

#include <stdio.h>

#include "logger.h"
#include "client_base.h"
#include "message.h"
#include "transport.h"
#include "pbyte_transport.h"

/*
c_client_t *c_client_add(int fd, char *nickname, size_t len);
int c_client_rem(c_client_t *client);
c_client_t *c_client_get_by_id(uint16_t id);
c_client_t *c_client_get_by_fd(int fd);
c_client_t *c_client_get_by_nick(char *nickname, size_t len);
c_client_t *c_client_get_by_ip(s_addr_t *ip_addr);
*/

/*
typedef enum {
	ePBYTE_SERVER = 0,
	ePBYTE_CLIENT,
} pb_transport_mode_t;

typedef struct {
	pb_transport_mode_t mode;
	int io_threads;
	const char *addr;
	int port;
	int logger_prio;

	int ack_timeout;
	int conn_check_timeout;

	int worker_count;

	union {
		int (*server_msg_handler)(int identity_idx, pb_dialog_t *dialog, char *data, int size);
		int (*client_msg_handler)(pb_dialog_t *dialog, char *data, int size);
	} msg_cb;

	union {
		int (*server_error_handler)(int identity_idx, pb_dialog_t *dialog, char *data, int size);
		int (*client_error_handler)(pb_dialog_t *dialog, char *data, int size);
	} err_cb;

	void (*logger)(int prio, char *format, ...);
	void (*thread_init)(void);
} pb_params_t;

typedef struct {
	pb_transport_mode_t mode;

	union {
		void *rserver;
		void *dclient;
	} choise;
} pb_transport_t;
*/

void init_thread_test(void)
{
	log_debug("%s", __func__);
}

int main(void)
{
	int res;
	int i;
	char nickname[MAX_STR_LEN] = {0};
	c_client_t *test_client;
	c_client_base_t client_base = {NULL, 0};

	pb_transport_t transport;
	pb_params_t params;

	log_info("test");

	memset(&transport, 0, sizeof(transport));
	memset(&params, 0, sizeof(params));

	params.mode = ePBYTE_SERVER;
	params.io_threads = 1;
	params.addr = "192.168.23.51";
	params.port = 1234;
	params.logger_prio = 99;
	params.ack_timeout = 5;
	params.conn_check_timeout = 30;
	params.worker_count = 1;

	// params.msg_cb.server_msg_handler
	// params.err_cb.server_error_handler

	params.logger = &log_print;
	params.thread_init = &init_thread_test;

	pb_transport_start(&transport, &params);

	res = c_base_init(&client_base, MAX_CLIENTS);
	if (res != 0)
	{
		log_error("failed to init client base [%d]", res);
		return;
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

	c_base_close(&client_base);

	pb_transport_stop(&transport);

	return 0;
}