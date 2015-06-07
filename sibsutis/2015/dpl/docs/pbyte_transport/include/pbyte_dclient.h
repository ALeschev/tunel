#ifndef __PBYTE_DCLIENT_H__
#define __PBYTE_DCLIENT_H__

#include "pbyte_transport.h"
#include "pbyte_log.h"

enum dclient_states {
	eNOT_INITED = 0,
	eINITED,
	eDISCONNECTED,
	eCONNECTED,
	eCONNECTED_WAIT_ACK
};

typedef struct {
	int state;

	void *core;

	int reconnect_timeout;
	int connection_timeout;
	int ACK_timeout;

	pthread_t thread_timer;

	/* unique client identity */
	identity_t identity;

	pb_logger_t logger;

	int (*message_handler)(dialog_t *dialog, char *data, int size);
	int (*error_handler)(dialog_t *dialog, char *data, int size);

	void (*thread_init)(void);
} pb_dclient_t;

int dclient_start (pb_dclient_t **dclient_ptr, pb_params_t *server_params);
int dclient_stop (pb_dclient_t *dclient);
int dclient_send (pb_dclient_t *dclient, dialog_t *dialog, char *data, int size);

#endif