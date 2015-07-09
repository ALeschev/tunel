#ifndef __PBYTE_TRANSPORT_H__
#define __PBYTE_TRANSPORT_H__

#define MAX_ALPHANUMS 32
#define PBYTE_STR_LEN 255
#define PBYTE_IDENTITY_LEN 32
#define IDENTITY_MAX 32

// #define TIME_DEBUG
// #define QUEUE_DEBUG
// #define DYNAMIC_IDENTITY

/* protocol structures */

typedef enum {
	CONREQ = 0,
	CONRSP,
	CONUPD,
	CONUPDACK,
	CONCLOSE,
	TRANSFER
} pb_msg_type_t;

typedef enum {
	CONTROLCHANNEL = 1
} pb_data_type_t;

typedef struct {
	unsigned dialog_len;
	char dialog_id[MAX_ALPHANUMS + 1];
} pb_dialog_t;

typedef struct {
	int active;
	int idx;
	int identity_len;
	char identity[PBYTE_IDENTITY_LEN];
	pb_dialog_t dialog;
} pb_identity_t;

typedef struct {
	pb_msg_type_t msg_type;
	pb_data_type_t data_type;
} pb_msg_params_t;

typedef struct {
#ifdef TIME_DEBUG
	struct timeval start;
	struct timeval finish;
#endif
	pb_identity_t identity;
	pb_msg_params_t message_params;
	int size;
	char *data;
} pb_msg_t;

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

#if defined (__cplusplus)
extern "C" {
#endif

int pb_transport_start(pb_transport_t *transport,
                       pb_params_t *transport_params);

int pb_transport_stop(pb_transport_t *transport);

int pb_transport_send (pb_transport_t *transport,
                       int identity_idx,
                       pb_dialog_t *dialog,
                       char *data, int size);

int pb_transport_broadcast (pb_transport_t *transport,
                            pb_dialog_t *dialog,
                            char *data, int size);

#if defined (__cplusplus)
}
#endif

#endif
