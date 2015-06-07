#ifndef __PBYTE_RSERVER_H__
#define __PBYTE_RSERVER_H__

#include "pbyte_transport.h"
#include "pbyte_log.h"

typedef struct {
	int wait_ack;
	int timeout;
	int timeout_ack;
} identity_control_t;

typedef struct {
	identity_t identity[IDENTITY_MAX];
	identity_control_t control[IDENTITY_MAX];
} identity_table_t;

typedef struct {
	int active;

	void *core;

	/* identity control thread */
	pthread_t thread_timer;
	identity_table_t identity_table;

	pb_logger_t logger;

	int (*message_handler)(identity_t *identity, char *data, int size);
	int (*error_handler)(identity_t *identity, char *data, int size);

	void (*thread_init)(void);
} pb_rserver_t;

int rserver_start(pb_rserver_t **rserver_ptr, pb_params_t *server_params);
int rserver_stop(pb_rserver_t *rserver);
int rserver_send (pb_rserver_t *rserver, identity_t *identity, char *data, int size);

#endif