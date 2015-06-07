#ifndef __PBYTE_TRANSPORT_H__
#define __PBYTE_TRANSPORT_H__

#define MAX_ALPHANUMS 32
#define PBYTE_STR_LEN 255
#define PBYTE_IDENTITY_LEN 32
#define IDENTITY_MAX 2

#define TIME_DEBUG
#define QUEUE_DEBUG
// #define DYNAMIC_IDENTITY

/* protocol structures */

typedef enum {
	CONREQ = 0,
	CONRSP,
	CONUPD,
	CONUPDACK,
	CONCLOSE,
	TRANSFER
} msg_type_t;

typedef enum {
	CONTROLCHANNEL = 1
} data_type_t;

typedef struct {
	int dialog_len;
	char dialog_id[MAX_ALPHANUMS + 1];
} dialog_t;

typedef struct {
	int active;
	int idx;
	int identity_len;
	char identity[PBYTE_IDENTITY_LEN];
	dialog_t dialog;
} identity_t;

typedef struct {
	msg_type_t msg_type;
	data_type_t data_type;
} msg_params_t;

typedef struct {
#ifdef TIME_DEBUG
	struct timeval start;
	struct timeval finish;
#endif
	identity_t identity;
	msg_params_t message_params;
	int size;
	char *data;
} pb_msg_t;

typedef enum {
	ePBYTE_SERVER = 0,
	ePBYTE_CLIENT,
} transport_mode_t;

typedef struct {
	transport_mode_t mode;
	int io_threads;
	const char *addr;
	int port;
	int logger_prio;

	int ack_timeout;
	int conn_check_timeout;

	int worker_count;

	union {
		int (*server_msg_handler)(identity_t *identity, char *data, int size);
		int (*client_msg_handler)(dialog_t *dialog, char *data, int size);
	} msg_cb;

	union {
		int (*server_error_handler)(identity_t *identity, char *data, int size);
		int (*client_error_handler)(dialog_t *dialog, char *data, int size);
	} err_cb;

	void (*logger)(int prio, char *format, ...);
	void (*thread_init)(void);
} pb_params_t;

typedef struct {
	transport_mode_t mode;

	union {
		void *rserver;
		void *dclient;
	} choise;
} pbyte_transport_t;

#if defined (__cplusplus)
extern "C" {
#endif

int pb_transport_start(pbyte_transport_t *transport,
                       pb_params_t *transport_params);

int pb_transport_stop(pbyte_transport_t *transport);

int pb_transport_send (pbyte_transport_t *transport,
                       identity_t *identity,
                       dialog_t *dialog,
                       char *data, int size);

#if defined (__cplusplus)
}
#endif

#endif
