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
#include "pbyte_dclient.h"
#include "pbyte_msg_api.h"

#define MSEC_1 1000

#define SEC_1 1000 * MSEC_1
#define SEC_2 2 * SEC_1
#define SEC_10 10 * SEC_1
#define SEC_5 5 * SEC_1

#define CONNECTION_TIMEOUT SEC_10
#define ACK_TIMEOUT SEC_2
#define RECONNECT_TIMEOUT SEC_5

static void *dclient_thread_timer(void *arg);
static int dclient_proc_connection(pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_connreq (pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_conrsp (pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_conupd (pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_conupdack (pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_conclose (pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_transfer (pb_dclient_t *dclient, pb_msg_t *pb_msg);
static int dclient_proc_error (pb_dclient_t *dclient, int msg_type,
                               pb_dialog_t *dialog, char *data, int size);

static int dclient_message_handler(void *server, pb_msg_t *message);
static int dclient_error_handler(void *server, pb_msg_t *message);

void pb_print_buff (pb_logger_t *logger, int lvl, const char *descript, const char *data, int size);
void pb_msg_print(pb_logger_t *logger, int lvl, const char *desc, const pb_msg_t *pb_msg);
const char *pb_pb_msg_type_to_str(pb_msg_type_t type);

static int dclient_msg_send (pb_dclient_t *dclient, pb_msg_t *pb_msg);

/*----------------------------------------------------------------------------*/

static char *pb_dclient_statate_str (int state)
{
	switch (state)
	{
		case eNOT_INITED: return "not_inited";
		case eINITED: return "inited";
		case eDISCONNECTED: return "disconnected";
		case eCONNECTED: return "connected";
		case eCONNECTED_WAIT_ACK: return "connected.wait ACK";
	}

	return "Undef";
}

/*----------------------------------------------------------------------------*/

static char *dclient_info_str (pb_dclient_t *dclient)
{
	static char client_info[256] = {0};
	char identity_str[256] = {0};

	if (!dclient)
		return "NULL";

	pb_pb_identity_to_str(&dclient->identity, 0, identity_str, sizeof (identity_str));

	snprintf(client_info, sizeof (client_info), "%s [%s]",
	         identity_str,
	         pb_dclient_statate_str(dclient->state));

	return client_info;
}

/*----------------------------------------------------------------------------*/

static void dclient_statate_set (pb_dclient_t *dclient, int state)
{
	if (!dclient)
		return;

	if (dclient->state == state)
		return;

	pb_log (dclient, PBYTE_INFO, "%s: State changed: '%s' -> '%s'",
	        dclient_info_str(dclient),
	        pb_dclient_statate_str(dclient->state),
	        pb_dclient_statate_str(state));

	dclient->state = state;
}

/*----------------------------------------------------------------------------*/

static int dclient_init (pb_dclient_t *dclient)
{
	if (!dclient)
		return -1;

	memset(dclient, 0, sizeof (pb_dclient_t));

	dclient_statate_set(dclient, eINITED);

	return 0;
}

/*----------------------------------------------------------------------------*/

static inline void dclient_thread_init (pb_dclient_t *dclient)
{
	if (dclient && dclient->thread_init)
		dclient->thread_init();
}

/*----------------------------------------------------------------------------*/

static int dclient_identity_generate(pb_identity_t *identity)
{
	time_t t;
	struct timeval tp;
	struct tm *pt;

	if (!identity)
		return -1;

	gettimeofday(&tp, NULL);
	t = (time_t)tp.tv_sec;
	pt = localtime(&t);

	sprintf(identity->identity, " %02d%02d%02d%06lu",
	        pt->tm_hour, pt->tm_min, pt->tm_sec, tp.tv_usec);

	identity->identity_len = strlen(identity->identity);

	return 0;
}

/*----------------------------------------------------------------------------*/

static inline void
dclient_connection_timeout_update (pb_dclient_t *dclient)
{
	if (!dclient)
		return;

	if (dclient->state == eCONNECTED_WAIT_ACK)
		dclient_statate_set (dclient, eCONNECTED);

	dclient->connection_timeout = CONNECTION_TIMEOUT;
	dclient->ACK_timeout = ACK_TIMEOUT;
}

/*----------------------------------------------------------------------------*/

static inline void
dclient_reconnect_timeout_update (pb_dclient_t *dclient)
{
	if (!dclient)
		return;

	dclient->reconnect_timeout = RECONNECT_TIMEOUT;
}

/*----------------------------------------------------------------------------*/

int dclient_stop (pb_dclient_t *dclient)
{
	int rc = 0;
	pb_msg_t *pb_msg = NULL;

	if (!dclient || dclient->state == eNOT_INITED)
		return 0;

	pb_log (dclient, PBYTE_WARN,
	        "%s: Stop detected",
	        dclient_info_str(dclient));

	if (dclient->state == eCONNECTED)
	{
		if (pb_msg_init (&pb_msg, 0) != 0)
		{
			pb_log (dclient, PBYTE_WARN,
			          "%s: Send message failed",
			          dclient_info_str(dclient));
			return -3;
		}

		pb_msg_set_identity (pb_msg, &dclient->identity);

		pb_msg_set_types (pb_msg, CONCLOSE, CONTROLCHANNEL);

		dclient_msg_send (dclient, pb_msg);
	}

	dclient_statate_set (dclient, eDISCONNECTED);
	dclient_statate_set (dclient, eNOT_INITED);

	pthread_join (dclient->thread_timer, NULL);

	pcore_stop (dclient->core);

	pb_log (dclient, PBYTE_INFO,
	        "%s: Stop success [%d]",
	        dclient_info_str(dclient),
	        rc);

	free (dclient);

	return rc;
}

/*----------------------------------------------------------------------------*/

int dclient_start (pb_dclient_t **dclient_ptr, pb_params_t *client_params)
{
	int rc;
	pb_dclient_t *dclient = NULL;
	pcore_params_t pcore_params = {0};

	if (!client_params)
	{
		rc = -1;
		goto err;
	}

	*dclient_ptr = (pb_dclient_t *)calloc(1, sizeof (pb_dclient_t));
	if (!*dclient_ptr)
	{
		rc = -1;
		goto err;
	}

	dclient = *dclient_ptr;

	dclient_init (dclient);

	if (client_params->msg_cb.client_msg_handler)
		dclient->message_handler = client_params->msg_cb.client_msg_handler;

	if (client_params->err_cb.client_error_handler)
		dclient->error_handler = client_params->err_cb.client_error_handler;

	if (client_params->logger)
	{
		strcpy (dclient->logger.prefix, "DCLIENT:");
		pb_log_set_logger (&dclient->logger, client_params->logger);
		pb_log_set_prio (&dclient->logger, client_params->logger_prio);
	}

	if (client_params->thread_init)
	{
		dclient->thread_init = client_params->thread_init;
	} else {
		pb_log (dclient, PBYTE_ERR,
		        "%s: Failed to start. "
		        "Thread init callback not set",
		        dclient_info_str(dclient));
	}

	if (!client_params->addr ||
	    (client_params->port <= 0) ||
	    (client_params->io_threads <= 0))
	{
		pb_log (dclient, PBYTE_ERR,
		        "%s: Failed to start. "
		        "Invalid params",
		        dclient_info_str(dclient));

		rc = -2;
		goto err;
	}

	dclient_identity_generate (&dclient->identity);

	pcore_params.mode            = ePBYTE_CLIENT;
	pcore_params.io_threads      = client_params->io_threads;
	pcore_params.addr            = client_params->addr;
	pcore_params.port            = client_params->port;
	pcore_params.logger_prio     = client_params->logger_prio;
	pcore_params.logger          = client_params->logger;
	pcore_params.thread_init     = client_params->thread_init;

	pcore_params.message_handler = dclient_message_handler;
	pcore_params.error_handler   = dclient_error_handler;

	memcpy (&pcore_params.identity, &dclient->identity, sizeof (pcore_params.identity));

	if (pcore_start (&dclient->core, &pcore_params, (void *)dclient) != 0)
	{
		pb_log (dclient, PBYTE_ERR,
		        "%s: Failed to start. "
		        "Failed to init pcore",
		        dclient_info_str(dclient));

		rc = -2;
		goto err;
	}

	dclient_statate_set (dclient, eDISCONNECTED);

	rc = pthread_create(&dclient->thread_timer,
	                    NULL,
	                    &dclient_thread_timer,
	                    (void *)dclient);
	if(rc < 0)
	{
		pb_log (dclient, PBYTE_ERR,
		        "%s: Failed to start. Can't create time thread: %s",
		        dclient_info_str(dclient),
		        strerror(errno));

		rc = -3;
		goto err;
	}

	// dclient_reconnect_timeout_update (dclient);
	dclient_connection_timeout_update (dclient);

	pb_log (dclient, PBYTE_INFO,
	        "%s: Start success",
	        dclient_info_str(dclient));

	return 0;

err:
	switch(rc)
	{
		case -3:
			dclient_stop(dclient);
		case -2:
			if (dclient)
				free (dclient);
		case -1: break;
	}

	return rc;
}

/*----------------------------------------------------------------------------*/

static int dclient_message_handler (void *client, pb_msg_t *message)
{
	pb_dclient_t *dclient = NULL;

	if (!message || !client)
		return -1;

	dclient = (pb_dclient_t *)client;

	dclient_proc_connection(dclient, message);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_error_handler (void *client, pb_msg_t *message)
{
	if (!client || !message)
		return -1;

	dclient_proc_error ((pb_dclient_t *)client,
	                    message->message_params.msg_type,
	                    &message->identity.dialog,
	                    message->data, message->size);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_msg_send (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	int rc = 0;

	if (!pb_msg || !dclient)
		goto err;

	rc = pcore_msg_send (dclient->core, pb_msg);
	if (rc != 0)
	{
		pb_msg_print(&dclient->logger, PBYTE_ERR,
		             "Send failed",
		             pb_msg);
		goto err;
	}

	return 0;

err:
	if (pb_msg)
		dclient_proc_error (dclient, pb_msg->message_params.msg_type,
		                    &pb_msg->identity.dialog, pb_msg->data,
		                    pb_msg->size);

	if (rc == 0)
		pb_msg_unpack_free (&pb_msg);

	return -1;
}

/*----------------------------------------------------------------------------*/

static void dclient_reconnect_check (pb_dclient_t *dclient, int delta_check)
{
	pb_msg_t *pb_msg = NULL;

	if (dclient->state != eDISCONNECTED)
		return;

	if (dclient->reconnect_timeout > 0)
	{
		dclient->reconnect_timeout -= delta_check;
		return;
	}

	pb_log (dclient, PBYTE_INFO,
	        "%s: Reconnect timeout expired. "
	        "Try send CONREQ",
	        dclient_info_str(dclient));

	if (pb_msg_init (&pb_msg, 0) != 0)
	{
		pb_log (dclient, PBYTE_WARN,
		        "%s: Send CONREQ failed",
		        dclient_info_str(dclient));

		return;
	}

	pb_msg_set_identity (pb_msg, &dclient->identity);

	pb_msg_set_types (pb_msg, CONREQ, 0);

	dclient_msg_send (dclient, pb_msg);

	dclient_reconnect_timeout_update (dclient);
}

/*----------------------------------------------------------------------------*/

static void dclient_connection_check (pb_dclient_t *dclient, int delta_check)
{
	pb_msg_t *pb_msg = NULL;

	if (dclient->state < eCONNECTED)
		return;

	if (dclient->state == eCONNECTED_WAIT_ACK)
	{
		dclient->ACK_timeout -= delta_check;
		if (dclient->ACK_timeout <= 0)
		{
			pb_log (dclient, PBYTE_WARN,
			        "%s: ACK timeout expired. "
			        "No confirmation after %d seconds",
			        dclient_info_str(dclient),
			        ACK_TIMEOUT / SEC_1);

			dclient_statate_set(dclient, eDISCONNECTED);
		}
	} else {
		dclient->connection_timeout -= delta_check;
		if (dclient->connection_timeout <= 0)
		{
			pb_log (dclient, PBYTE_WARN,
			        "%s: %d seconds of silence. "
			        "Send CONUPD",
			        dclient_info_str(dclient),
			        CONNECTION_TIMEOUT / SEC_1);

			if (pb_msg_init (&pb_msg, 0) != 0)
			{
				pb_log (dclient, PBYTE_WARN,
				        "%s: Send CONUPD failed",
				        dclient_info_str(dclient));

				return;
			}

			pb_msg_set_identity (pb_msg, &dclient->identity);

			pb_msg_set_types (pb_msg, CONUPD, 0);

			dclient_msg_send (dclient, pb_msg);

			dclient_statate_set(dclient, eCONNECTED_WAIT_ACK);
		}
	}
}

/*----------------------------------------------------------------------------*/

void *dclient_thread_timer (void *arg)
{
	pb_dclient_t *dclient = NULL;
	int delta_check = SEC_1;

	if (!arg)
		return NULL;

	dclient = (pb_dclient_t *)arg;

	dclient_thread_init (dclient);

	pb_log (dclient, PBYTE_INFO, "Timer thread: Inited");

	while (dclient->state != eNOT_INITED)
	{
		dclient_reconnect_check (dclient, delta_check);
		dclient_connection_check (dclient, delta_check);

		usleep (delta_check);
	}

	pb_log (dclient, PBYTE_INFO,
	        "Timer thread: Normal exit",
	        dclient_info_str(dclient));

	return NULL;
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_connection (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	int rc = -1;
	int msg_type;
	char identity_str[256] = {0};

	if (!dclient || !pb_msg)
	{
		pb_log (dclient, PBYTE_ERR,
		        "Connection process failed. "
		        "Arg error: <%p> <%p>",
		        dclient, pb_msg);
		return -1;
	}

	msg_type = pb_msg->message_params.msg_type;

	pb_log (dclient, PBYTE_INFO,
	        "%s: Process '%s'",
	        dclient_info_str(dclient),
	        pb_pb_msg_type_to_str(msg_type));

	switch(msg_type)
	{
		case CONREQ:
			rc = dclient_proc_connreq(dclient, pb_msg);
		break;

		case CONRSP:
			rc = dclient_proc_conrsp(dclient, pb_msg);
		break;

		case CONUPD:
			rc = dclient_proc_conupd(dclient, pb_msg);
		break;

		case CONUPDACK:
			rc = dclient_proc_conupdack(dclient, pb_msg);
		break;

		case CONCLOSE:
			rc = dclient_proc_conclose(dclient, pb_msg);
		break;

		case TRANSFER:
			rc = dclient_proc_transfer(dclient, pb_msg);
		break;

		default:
				pb_log (dclient, PBYTE_ERR,
				        "Unknown message type: %s",
				        msg_type);
	}

	pb_pb_identity_to_str(&dclient->identity, 1, identity_str, sizeof (identity_str));

	pb_log (dclient, PBYTE_INFO,
	        "%s: Process '%s' - %s",
	        identity_str,
	        pb_pb_msg_type_to_str(msg_type),
	        (rc == 0)? "Success":"Failed");

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_connreq (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	pb_log (dclient, PBYTE_WARN,
	        "%s: Process CONREQ. CONREQ? Im a client!",
	        dclient_info_str(dclient));

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_conrsp (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	dclient_statate_set (dclient, eCONNECTED);

	dclient_connection_timeout_update (dclient);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_conupd (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	int conn_res = CONUPDACK;
	pb_msg_t *pb_msg_upd = NULL;

	if (pb_msg_init (&pb_msg_upd, 0) != 0)
		return -1;

	if (dclient->state != eCONNECTED)
		conn_res = CONCLOSE;

	pb_msg_set_identity (pb_msg_upd, &dclient->identity);

	pb_msg_set_types (pb_msg_upd, conn_res, 0);

	dclient_msg_send (dclient, pb_msg_upd);

	dclient_connection_timeout_update (dclient);

	return (conn_res != CONUPDACK);
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_conupdack (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	if (dclient->state != eCONNECTED_WAIT_ACK)
		return -1;

	dclient_connection_timeout_update (dclient);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_conclose (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	if (dclient->state != eCONNECTED)
		return -1;

	dclient_stop(dclient);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int dclient_proc_transfer (pb_dclient_t *dclient, pb_msg_t *pb_msg)
{
	if (!dclient || !dclient->message_handler)
		return 0;

	if (dclient->state != eCONNECTED)
	{
		pb_log (dclient, PBYTE_ERR,
		        "%s: Process TRANSFER failed. "
		        "Bad state",
		        dclient_info_str(dclient));
		return -1;
	}

	dclient_connection_timeout_update (dclient);

	// pb_msg_set_dialog (&dclient->identity,
	//                    pb_msg->identity.dialog.dialog_id,
	//                    pb_msg->identity.dialog.dialog_len);

	dclient->message_handler(&pb_msg->identity.dialog, pb_msg->data, pb_msg->size);

	return 0;
}

static int dclient_proc_error (pb_dclient_t *dclient, int msg_type,
                               pb_dialog_t *dialog, char *data, int size)
{
	if (!dclient)
		return -1;

	if (!dclient->error_handler)
		return 0;

	if (msg_type != TRANSFER)
		return 0;

	return dclient->error_handler(dialog, data, size);
}

int dclient_send (pb_dclient_t *dclient, pb_dialog_t *dialog, char *data, int size)
{
	pb_msg_t *pb_msg = NULL;

	if (!dclient || /*!dialog ||*/ !data || (size <= 0))
	{
		pb_log (dclient, PBYTE_ERR,
		        "Message send failed. "
		        // "Arg error: <%p> <%p> [%d]",
		        "Arg error: <%p> [%d]",
		        data, /*dialog, */size);

		if (dclient)
			dclient_proc_error (dclient, TRANSFER, dialog, data, size);

		return -1;
	}

	if (dialog)
	{
		pb_log (dclient, PBYTE_INFO,
		        "%s: New message. Dialog '%.*s'",
		        dclient_info_str(dclient),
		        (int)dialog->dialog_len, dialog->dialog_id);
	}
	else
	{
		pb_log (dclient, PBYTE_WARN,
		        "%s: New message. Dialog '%p'",
		        dclient_info_str(dclient), dialog);
	}

	if (dclient->state <= eDISCONNECTED)
	{
		pb_log (dclient, PBYTE_ERR,
		        "%s: Message send failed. Bad state",
		        dclient_info_str(dclient));
		return -2;
	}

	if (dialog)
	{
		pb_msg_set_dialog (&dclient->identity,
		                   dialog->dialog_id,
		                   dialog->dialog_len);
	}

	if (pb_msg_init (&pb_msg, size) != 0)
	{
		pb_log (dclient, PBYTE_WARN,
		          "%s: Send message failed",
		          dclient_info_str(dclient));
		return -3;
	}

	pb_msg_put_data(pb_msg, data, size);

	pb_msg_set_identity (pb_msg, &dclient->identity);

	if (dialog)
	{
		pb_msg_set_dialog (&pb_msg->identity,
		                   dclient->identity.dialog.dialog_id,
		                   dclient->identity.dialog.dialog_len);
	}

	pb_msg_set_types (pb_msg, TRANSFER, CONTROLCHANNEL);

	dclient_msg_send (dclient, pb_msg);

	return 0;
}
