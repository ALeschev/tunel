#ifndef _PBYTE_core_H_
#define _PBYTE_core_H_

#include "pbyte_transport.h"

#define DEF_WORKER_COUNT 2
#define MAX_WORKER_COUNT 10

typedef struct {
	transport_mode_t mode;

	int io_threads;
	const char *addr;
	int port;
	int logger_prio;

	int worker_count;

	/* client only */
	identity_t identity;

	int (*message_handler)(void *app, pb_msg_t *message);
	int (*error_handler)(void *app, pb_msg_t *message);
	void (*logger)(int prio, char *format, ...);
	void (*thread_init)(void);
} pcore_params_t;

int pcore_start (void **core_ptr, pcore_params_t *pcore_params, void *application);
int pcore_stop(void *core);
int pcore_msg_send (void *core, pb_msg_t *pb_msg);

#endif