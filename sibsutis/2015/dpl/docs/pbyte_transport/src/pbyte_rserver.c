#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include <sys/time.h>

#include "zmq.h"
#include "mqueue.h"

#include "pbyte_core.h"
#include "pbyte_transport.h"
#include "pbyte_rserver.h"
#include "pbyte_msg_api.h"

#define SEC_1 1000000
#define SEC_10 10 * SEC_1
#define SEC_2 2 * SEC_1

static int CONNECTION_TIMEOUT = SEC_10;
static int ACK_TIMEOUT = SEC_2;

enum { eCONN_TIMEOUT, eCONNACK_TIMEOUT };

static void *rserver_thread_timer(void *arg);
static int rserver_proc_connection(pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_connreq (pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_conrsp (pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_conupd (pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_conupdack (pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_conclose (pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_transfer (pb_rserver_t *rserver, pb_msg_t *pb_msg);
static int rserver_proc_error (pb_rserver_t *rserver, int msg_type,
                          identity_t *identity, char *data, int size);

static int rserver_message_handler(void *server, pb_msg_t *message);
static int rserver_error_handler(void *server, pb_msg_t *message);

static int rserver_identity_rem(pb_rserver_t *rserver, char *identity, int idx);
static inline void rserver_identity_clean (identity_t *identity);
static identity_t *rserver_identity_check(pb_rserver_t *rserver, identity_t *identity);

void pb_print_buff (pb_logger_t *logger, int lvl, const char *descript, const char *data, int size);
void pb_msg_print(pb_logger_t *logger, int lvl, const char *desc, const pb_msg_t *pb_msg);
const char *pb_msg_type_to_str(msg_type_t type);

/*----------------------------------------------------------------------------*/

static int rserver_init(pb_rserver_t *rserver)
{
	int i;

	if (!rserver)
		return -1;

	for(i = 0; i < IDENTITY_MAX; i++)
		rserver_identity_clean (&rserver->identity_table.identity[i]);

	memset(&rserver->identity_table.control, 0, sizeof (rserver->identity_table.control));
	memset(rserver, 0, sizeof (pb_rserver_t));

	return 0;
}

/*----------------------------------------------------------------------------*/

static inline void rserver_thread_init(pb_rserver_t *rserver)
{
	if (rserver && rserver->thread_init)
		rserver->thread_init();
}

/*----------------------------------------------------------------------------*/

int rserver_stop(pb_rserver_t *rserver)
{
	int rc = 0;

	if (!rserver || !rserver->active)
		return 0;

	pb_log (rserver, PBYTE_WARN, "Stop detected");

	rserver->active = 0;

	pthread_join (rserver->thread_timer, NULL);

	pcore_stop(rserver->core);

	pb_log (rserver, PBYTE_INFO, "Stop success [%d]", rc);

	free (rserver);

	return rc;
}

/*----------------------------------------------------------------------------*/

int rserver_start(pb_rserver_t **rserver_ptr, pb_params_t *server_params)
{
	int rc;
	pb_rserver_t *rserver = NULL;
	pcore_params_t pcore_params = {0};

	if (!server_params)
	{
		rc = -1;
		goto err;
	}

	*rserver_ptr = (pb_rserver_t *)calloc(1, sizeof (pb_rserver_t));
	if (!*rserver_ptr)
	{
		rc = -1;
		goto err;
	}

	rserver = *rserver_ptr;

	rserver_init(rserver);


	if (server_params->msg_cb.server_msg_handler)
		rserver->message_handler = server_params->msg_cb.server_msg_handler;

	if (server_params->err_cb.server_error_handler)
		rserver->error_handler = server_params->err_cb.server_error_handler;

	if (server_params->logger)
	{
		strcpy (rserver->logger.prefix, "RSERVER:");
		pb_log_set_logger (&rserver->logger, server_params->logger);
		pb_log_set_prio (&rserver->logger, server_params->logger_prio);
	}

	if (server_params->thread_init)
	{
		rserver->thread_init = server_params->thread_init;
	} else {
		pb_log (rserver, PBYTE_ERR,
		        "Failed to start. "
		        "Thread init callback not set");
	}

	if (!server_params->addr ||
	    (server_params->port <= 0) ||
	    (server_params->io_threads <= 0))
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to start. "
		        "Invalid params");

		rc = -2;
		goto err;
	}

	if (server_params->ack_timeout && server_params->ack_timeout > 0)
		ACK_TIMEOUT = server_params->ack_timeout * SEC_1;

	if (server_params->conn_check_timeout && server_params->conn_check_timeout > 0)
		CONNECTION_TIMEOUT = server_params->conn_check_timeout * SEC_1;

	pcore_params.mode            = ePBYTE_SERVER;
	pcore_params.io_threads      = server_params->io_threads;
	pcore_params.addr            = server_params->addr;
	pcore_params.port            = server_params->port;
	pcore_params.logger_prio     = server_params->logger_prio;
	pcore_params.logger          = server_params->logger;
	pcore_params.thread_init     = server_params->thread_init;
	pcore_params.worker_count    = server_params->worker_count;

	pcore_params.message_handler = rserver_message_handler;
	pcore_params.error_handler   = rserver_error_handler;

	if (pcore_start(&rserver->core, &pcore_params, (void *)rserver) != 0)
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to start. "
		        "Failed to init pcore");

		rc = -2;
		goto err;
	}

	rserver->active = 1;

	rc = pthread_create(&rserver->thread_timer,
	                    NULL,
	                    &rserver_thread_timer,
	                    (void *)rserver);
	if(rc < 0)
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to start. "
		        "Can't create time thread: %s",
		        strerror(errno));

		rc = -3;
		goto err;
	}

	pb_log (rserver, PBYTE_INFO,
	        "Start success: %s:%d. CT %ds AT %ds\n",
	        server_params->addr,
	        server_params->port,
	        CONNECTION_TIMEOUT/SEC_1,
	        ACK_TIMEOUT/SEC_1);

	return 0;

err:
	switch(rc)
	{
		case -3:
			rserver_stop(rserver);
			break;
		case -2:
			if (rserver)
				free(rserver);
		case -1: break;
	}

	return rc;
}

/*----------------------------------------------------------------------------*/

static int rserver_message_handler(void *server, pb_msg_t *message)
{
	pb_rserver_t *rserver = NULL;

	if (!message || !server)
		return -1;

	rserver = (pb_rserver_t *)server;

	rserver_proc_connection(rserver, message);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_error_handler(void *server, pb_msg_t *message)
{
	if (!server || !message)
		return -1;

	rserver_proc_error ((pb_rserver_t *)server,
	               message->message_params.msg_type,
	               &message->identity,
	               message->data,
	               message->size);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_msg_send (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	int rc;
	identity_t *p_identity = NULL;

	if (!pb_msg || !rserver)
		goto err;

	/*
	 * msg_type != CONCLOSE ?
	 * check it. may be this check actually only for "dynamic identity" mode?
	 */
	if (pb_msg->message_params.msg_type != CONCLOSE)
	{
		p_identity = rserver_identity_check(rserver, &pb_msg->identity);
		if (!p_identity)
		{
			pb_log (rserver, PBYTE_ERR,
			        "Check identity failed. "
			        "Skip message");
			goto err;
		}

#if DYNAMIC_IDENTITY
		memcpy(&pb_msg->identity, p_identity, sizeof (identity_t));
#endif
	}

#ifdef TIME_DEBUG
		pb_clockgettime(&pb_msg->start);
#endif

	rc = pcore_msg_send (rserver->core, pb_msg);
	if (rc != 0)
	{
		pb_msg_print(&rserver->logger, PBYTE_ERR,
		             "Send failed.",
		             pb_msg);
		goto err;
	}

	return 0;

err:
	rserver_proc_error (rserver, pb_msg->message_params.msg_type,
	               &pb_msg->identity, pb_msg->data,
	               pb_msg->size);

	pb_msg_unpack_free (pb_msg);

	return -1;
}

/*----------------------------------------------------------------------------*/

static inline int
rserver_identity_timeout_set (pb_rserver_t *rserver,
                             identity_t *p_identity,
                             int t_num, int value)
{
	identity_control_t *p_control = NULL;

	if (!rserver || !p_identity)
		return -1;

	/* not a error, but the situation is unpleasant :\ */
	if (!p_identity->active)
		return 1;

	p_control = &rserver->identity_table.control[p_identity->idx];

	if (t_num == eCONN_TIMEOUT)
	{
		p_control->timeout = value;
	} else
	if (t_num == eCONNACK_TIMEOUT)
	{
		p_control->timeout_ack = value;
		p_control->wait_ack = 0; /* Hmm... */
	} else {
		pb_log (rserver, PBYTE_ERR,
		        "[%d] %s: Update control timeout failed.",
		        p_identity->identity);
		return -2;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/

static inline void
rserver_identity_control_check(pb_rserver_t *rserver, int delta_check)
{
	int i;
	pb_msg_t *pb_msg = NULL;
	identity_t *p_identity = NULL;
	identity_control_t *p_control = NULL;
	char identity_str[256] = {0};

	if (!rserver)
		return;

	for(i = 0; i < IDENTITY_MAX; i++)
	{
		p_identity = &rserver->identity_table.identity[i];
		p_control = &rserver->identity_table.control[i];

		if (!p_identity->active)
			continue;

		pb_identity_to_str(p_identity, 1, identity_str, sizeof (identity_str));

		if (p_control->wait_ack)
		{
			p_control->timeout_ack -= delta_check;
			if (p_control->timeout_ack <= 0)
			{
				pb_log (rserver, PBYTE_WARN,
				        "%s: ACK timeout expired. "
				        "No confirmation after %d seconds",
				       identity_str, SEC_2/SEC_1);

				rserver_identity_rem(rserver, NULL, i);
			}
		} else {
			p_control->timeout -= delta_check;

			if (p_control->timeout > 0)
				return;

			pb_log (rserver, PBYTE_WARN,
			        "%s: %d seconds of silence. "
			        "Send CONUPD",
			       identity_str, CONNECTION_TIMEOUT/SEC_1);

			if (pb_msg_init (&pb_msg, 0) != 0)
			{
				pb_log (rserver, PBYTE_ERR,
				        "%s: Send CONUPD failed",
				       identity_str);

				return;
			}

			pb_msg_set_identity (pb_msg, p_identity);
			pb_msg_set_types (pb_msg, CONUPD, 0);

			if (rserver_msg_send (rserver, pb_msg) == 0)
				p_control->wait_ack = 1;
		}
	}
}

/*----------------------------------------------------------------------------*/

static inline void rserver_identity_clean (identity_t *identity)
{
	memset (identity, 0, sizeof (identity_t));
	identity->idx = -1;
}

/*----------------------------------------------------------------------------*/

static identity_t *rserver_identity_get_by_idx(pb_rserver_t *rserver, int idx)
{
	if (!rserver || ((idx < 0) || (idx >= IDENTITY_MAX)))
		return NULL;

	if (rserver->identity_table.identity[idx].active)
		return &rserver->identity_table.identity[idx];

	return NULL;
}

/*----------------------------------------------------------------------------*/

static identity_t *rserver_identity_get(pb_rserver_t *rserver, identity_t *identity)
{
	int i;
	identity_t *p_identity = NULL;

	if (!rserver || !identity)
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to set identity. "
		        "Arg error");
		return NULL;
	}

	for(i = 0; i < IDENTITY_MAX; i++)
	{
		p_identity = &rserver->identity_table.identity[i];

		if (!p_identity->active)
			continue;

		if (p_identity->identity_len != identity->identity_len)
			continue;

		if (memcmp (p_identity->identity,
		            identity->identity,
		            identity->identity_len))
		{
			continue;
		}

		return p_identity;
	}

	pb_log (rserver, PBYTE_ERR,
	        "Failed to get identity. "
	        "Not found");

	return NULL;
}

/*----------------------------------------------------------------------------*/

static identity_t *rserver_identity_set(pb_rserver_t *rserver, identity_t *identity)
{
	int i;
	identity_t *p_identity = NULL;
	char identity_str[256] = {0};

	if (!rserver || !identity)
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to set identity. "
		        "Arg error");
		return NULL;
	}

	for(i = 0; i < IDENTITY_MAX; i++)
	{
		p_identity = &rserver->identity_table.identity[i];

		if (!p_identity->active)
			continue;

		if (p_identity->identity_len == identity->identity_len)
		{
			if (memcmp (p_identity->identity, identity->identity,
			        identity->identity_len))
			{
				continue;
			} else {
				return &rserver->identity_table.identity[i];
			}
		}
	}

	for(i = 0; i < IDENTITY_MAX; i++)
	{
		p_identity = &rserver->identity_table.identity[i];

		if (p_identity->active)
			continue;

		memcpy (p_identity->identity, identity->identity, identity->identity_len);

		p_identity->identity_len = identity->identity_len;
		p_identity->idx = i;
		p_identity->dialog.dialog_len = identity->dialog.dialog_len;

		strncpy (p_identity->dialog.dialog_id,
		         identity->dialog.dialog_id,
		         p_identity->dialog.dialog_len);

		p_identity->active = 1;

		rserver_identity_timeout_set(rserver, p_identity, eCONN_TIMEOUT, CONNECTION_TIMEOUT);
		rserver_identity_timeout_set(rserver, p_identity, eCONNACK_TIMEOUT, ACK_TIMEOUT);

		pb_identity_to_str(p_identity, 1, identity_str, sizeof(identity_str));

		pb_log (rserver, PBYTE_INFO,
		        "%s: Activated",
		        identity_str);

		return &rserver->identity_table.identity[i];
	}

	pb_log (rserver, PBYTE_WARN,
	        "Failed to set identity. "
	        "All writes used");

	return NULL;
}

/*----------------------------------------------------------------------------*/

static int rserver_identity_rem(pb_rserver_t *rserver, char *identity, int idx)
{
	int i;

	if (!rserver || (!identity && ((idx < 0) || (idx >= IDENTITY_MAX))))
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to remove identity. "
		        "Arg error");
		return -1;
	}

	if (!identity)
	{
		rserver_identity_clean (&rserver->identity_table.identity[idx]);
		return 0;
	}

	for(i = 0; i < IDENTITY_MAX; i++)
	{
		if (!rserver->identity_table.identity[i].active)
			continue;

		if (strcmp (rserver->identity_table.identity[i].identity, identity))
			continue;

		rserver_identity_clean (&rserver->identity_table.identity[i]);

		return 0;
	}

	return -1;
}

/*----------------------------------------------------------------------------*/

#if DYNAMIC_IDENTITY
static identity_t *rserver_identity_first_valid(pb_rserver_t *rserver)
{
	int i;

	if (!rserver)
		return NULL;

	for(i = 0; i < IDENTITY_MAX; i++)
	{
		if (rserver->identity_table.identity[i].active)
			return &rserver->identity_table.identity[i];
	}

	return NULL;
}
#endif

/*----------------------------------------------------------------------------*/

static identity_t *rserver_identity_check(pb_rserver_t *rserver, identity_t *identity)
{
	char identity_str[256] = {0};

	if (!rserver || !identity)
	{
		pb_log (rserver, PBYTE_ERR,
		        "Failed to check identity. "
		        "Arg error");

		return NULL;
	}

	pb_identity_to_str(identity, 1, identity_str, sizeof (identity_str));

	pb_log (rserver, PBYTE_INFO,
	        "Check %s: %sctive",
	        identity_str,
	        (identity->active)? "A":"Ina");

	if (identity->active)
		return identity;

#if DYNAMIC_IDENTITY
	return rserver_identity_first_valid(rserver);
#endif

	return NULL;
}

/*----------------------------------------------------------------------------*/

static void *rserver_thread_timer(void *arg)
{
	pb_rserver_t *rserver = NULL;
	int delta_check = SEC_1;

	if (!arg)
		return NULL;

	rserver = (pb_rserver_t *)arg;

	rserver_thread_init(rserver);

	pb_log (rserver, PBYTE_INFO, "Timer thread: Inited");

	while (rserver->active)
	{
		usleep(delta_check);

		rserver_identity_control_check(rserver, delta_check);
	}

	pb_log (rserver, PBYTE_INFO, "Timer thread: Normal exit");

	return NULL;
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_connection(pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	int rc = -1;
	int msg_type = -1;
	char identity_str[256] = {0};

	if (!rserver || !pb_msg)
	{
		pb_log (rserver, PBYTE_ERR,
		        "Connection process failed. "
		        "Arg error");
		return -1;
	}

	msg_type = pb_msg->message_params.msg_type;

	pb_identity_to_str(&pb_msg->identity, 1, identity_str, sizeof (identity_str));

	pb_log (rserver, PBYTE_INFO,
	        "%s: Process '%s'",
	        identity_str,
	        pb_msg_type_to_str(msg_type));

	switch(msg_type)
	{
		case CONREQ:
			rc = rserver_proc_connreq(rserver, pb_msg);
		break;

		case CONRSP:
			rc = rserver_proc_conrsp(rserver, pb_msg);
		break;

		case CONUPD:
			rc = rserver_proc_conupd(rserver, pb_msg);
		break;

		case CONUPDACK:
			rc = rserver_proc_conupdack(rserver, pb_msg);
		break;

		case CONCLOSE:
			rc = rserver_proc_conclose(rserver, pb_msg);
		break;

		case TRANSFER:
			rc = rserver_proc_transfer(rserver, pb_msg);
		break;

		default:
			pb_log (rserver, PBYTE_ERR,
			        "Unknown message type: %s",
			        msg_type);
	}

	pb_log (rserver, PBYTE_INFO,
	        "%s: Process '%s' - %s",
	        identity_str,
	        pb_msg_type_to_str(msg_type),
	        (rc == 0)? "Success":"Failed");

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_connreq (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	int conn_res = CONRSP;
	identity_t *identity = NULL;
	pb_msg_t *pb_conrsp = NULL;
	char identity_str[256] = {0};

	pb_identity_to_str(&pb_msg->identity, 1, identity_str, sizeof (identity_str));

	if (pb_msg_init (&pb_conrsp, 0) != 0)
	{
		pb_log (rserver, PBYTE_WARN,
		        "%s: Process CONREQ failed",
		        identity_str);

		return -1;
	}

	if ((identity = rserver_identity_set(rserver, &pb_msg->identity)) == NULL)
	{
		conn_res = CONCLOSE;

		pb_log (rserver, PBYTE_WARN,
		        "%s: Process CONREQ failed",
		        identity_str);
	} else {
		memcpy (&pb_msg->identity, identity, sizeof (identity_t));
	}

	pb_msg_set_identity (pb_conrsp, &pb_msg->identity);

	pb_msg_set_types (pb_conrsp, conn_res, 0);

	rserver_msg_send (rserver, pb_conrsp);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_conrsp (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	char identity_str[256] = {0};

	pb_identity_to_str(&pb_msg->identity, 1, identity_str, sizeof (identity_str));

#if 0
	identity_t *identity;

	if ((identity = rserver_identity_get(rserver, &pb_msg->identity)) == NULL)
	{
		pb_log (rserver, PBYTE_ERR,
		        "%s: Process CONRSP failed",
		        pb_identity_to_str(&pb_msg->identity, 1));

		if (rserver_identity_rem(rserver, pb_msg->identity.identity, -1) < 0)
			return -1;

		return 0;
	}

	rserver_identity_timeout_set(rserver, identity, eCONN_TIMEOUT, SEC_10);
#else
	pb_log (rserver, PBYTE_WARN,
	        "%s: Process CONRSP. CONRSP? Im a server!",
	        identity_str);
#endif

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_conupd (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	int conn_res = CONUPDACK;
	pb_msg_t *pb_conrsp = NULL;
	identity_t *identity = NULL;
	char identity_str[256] = {0};

	pb_identity_to_str(&pb_msg->identity, 1, identity_str, sizeof (identity_str));

	if (pb_msg_init (&pb_conrsp, 0) != 0)
	{
		pb_log (rserver, PBYTE_WARN,
		        "%s: Process CONUPD failed",
		        identity_str);

		return -1;
	}

	identity = rserver_identity_get(rserver, &pb_msg->identity);
	if (!identity)
	{
		conn_res = CONCLOSE;
	} else {
		rserver_identity_timeout_set(rserver, identity, eCONN_TIMEOUT, CONNECTION_TIMEOUT);
		pb_msg->identity.idx = identity->idx;
		pb_msg->identity.active = identity->active;
	}

	pb_msg_set_identity (pb_conrsp, &pb_msg->identity);;

	pb_msg_set_types (pb_conrsp, conn_res, 0);

	rserver_msg_send (rserver, pb_conrsp);

	return (conn_res != CONUPDACK);
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_conupdack (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	identity_t *identity = NULL;

	if ((identity = rserver_identity_get(rserver, &pb_msg->identity)) == NULL)
	{
		if (rserver_identity_rem(rserver, pb_msg->identity.identity, -1) < 0)
			return -1;

		return 0;
	}

	rserver_identity_timeout_set(rserver, identity, eCONN_TIMEOUT, CONNECTION_TIMEOUT);
	rserver_identity_timeout_set(rserver, identity, eCONNACK_TIMEOUT, ACK_TIMEOUT);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_conclose (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	return rserver_identity_rem(rserver, pb_msg->identity.identity, -1);
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_transfer (pb_rserver_t *rserver, pb_msg_t *pb_msg)
{
	pb_msg_t *pb_conrsp = NULL;
	identity_t *identity = NULL;

	if (!rserver->message_handler)
		return 0;

	identity = rserver_identity_get(rserver, &pb_msg->identity);
	if (!identity)
	{
		if (pb_msg_init (&pb_conrsp, 0) != 0)
			return -1;

		pb_msg_set_types (pb_conrsp, CONCLOSE, 0);

		rserver_msg_send (rserver, pb_conrsp);

		return 1;
	} else {
		rserver_identity_timeout_set(rserver, identity, eCONN_TIMEOUT, CONNECTION_TIMEOUT);
		pb_msg->identity.idx = identity->idx;
		pb_msg->identity.active = identity->active;
	}

	pb_msg_set_dialog (identity, pb_msg->identity.dialog.dialog_id,
	                                pb_msg->identity.dialog.dialog_len);

#ifdef TIME_DEBUG
	pb_print_msg_time(&rserver->logger, "RServer -> MSR.", pb_msg);

	struct timeval start, finish, diff;

	pb_clockgettime(&start);
#endif

	rserver->message_handler(identity, pb_msg->data, pb_msg->size);

#ifdef TIME_DEBUG
	pb_clockgettime(&finish);
	timersub(&finish, &start, &diff);

	pb_trace (&rserver->logger,
	          ((diff.tv_sec > 1) && diff.tv_usec)? PBYTE_WARN:PBYTE_INFO,
	          "MSR callback time (dialog '%s'): %ld.%06ld sec",
	          pb_msg->identity.dialog.dialog_id, diff.tv_sec, diff.tv_usec);

#endif

	return 0;
}

/*----------------------------------------------------------------------------*/

static int rserver_proc_error (pb_rserver_t *rserver, int msg_type,
                               identity_t *identity,
                               char *data, int size)
{
	if (!rserver)
		return -1;

	if (!rserver->error_handler)
		return 0;

	if (msg_type != TRANSFER)
		return 0;

	return rserver->error_handler(identity, data, size);
}

/*----------------------------------------------------------------------------*/

int rserver_send (pb_rserver_t *rserver, identity_t *identity, char *data, int size)
{
	identity_t *p_identity = NULL;
	pb_msg_t *pb_conrsp = NULL;
	char identity_str[256] = {0};

	pb_identity_to_str(identity, 1, identity_str, sizeof (identity_str));

	if (!rserver || !identity || !data || (size <= 0))
	{
		pb_log (rserver, PBYTE_ERR,
		        "Message send failed. "
		        "Arg error: <%p> <%p> [%d]",
		        identity, data, size);

		return rserver_proc_error (rserver, TRANSFER, identity, data, size);
	}

	pb_log (rserver, PBYTE_INFO,
	        "New message for %s: %s",
	        identity_str,
	        identity->dialog.dialog_id);

	p_identity = rserver_identity_get_by_idx(rserver, identity->idx);
	if (!p_identity)
	{
		pb_log (rserver, PBYTE_WARN,
		        "%s: Send message failed. "
		        "Identity is invalid",
		        identity_str);

		return -2;
	} else {
		identity->idx = p_identity->idx;
		identity->active = p_identity->active;
		// rserver_identity_timeout_set(identity->idx, eCONN_TIMEOUT, SEC_10);
	}

	if (pb_msg_init (&pb_conrsp, size) != 0)
	{
		pb_identity_to_str(identity, 1, identity_str, sizeof (identity_str));
		pb_log (rserver, PBYTE_WARN,
		        "%s: Send message failed",
		        identity_str);
		return -3;
	}

	pb_msg_put_data(pb_conrsp, data, size);

	pb_msg_set_identity (pb_conrsp, identity);

	pb_msg_set_dialog (&pb_conrsp->identity,
	                   identity->dialog.dialog_id,
	                   identity->dialog.dialog_len);

	pb_msg_set_types (pb_conrsp, TRANSFER, CONTROLCHANNEL);

	rserver_msg_send (rserver, pb_conrsp);

	return 0;
}
