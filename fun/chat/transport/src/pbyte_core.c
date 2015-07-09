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
#include "pbyte_msg_api.h"
#include "pbyte_log.h"

#define BUDGET 100
#define MSG_TIMEOUT 10

typedef struct {
	pb_transport_mode_t mode;

	int active;
	char URL[PBYTE_STR_LEN + 1];

	/* server/client */
	void *application;

	void *ctx;
	void *sock;
	zmq_pollitem_t pollitem;
	pthread_t thread_recv_send;

	pthread_t thread_worker[MAX_WORKER_COUNT];
	int worker_count;

	mqueue_t queue_in;
	mqueue_t queue_out;

	pb_logger_t logger;

	int (*message_handler)(void *app, pb_msg_t *message);
	int (*error_handler)(void *app, pb_msg_t *message);

	void (*thread_init)(void);
} pb_core_t;

static void *pcore_thread_recv_send(void *arg);
static void *pcore_thread_worker(void *arg);

void pb_print_buff (pb_logger_t *logger, int lvl, const char *descript, const char *data, int size);
void pb_msg_print(pb_logger_t *logger, int lvl, const char *desc, const pb_msg_t *pb_msg);
const char *pb_pb_msg_type_to_str(pb_msg_type_t type);

/*----------------------------------------------------------------------------*/

static inline void pcore_init (pb_core_t *core)
{
	if (core)
		memset (core, 0, sizeof (pb_core_t));
}

/*----------------------------------------------------------------------------*/

static inline void pcore_thread_init (pb_core_t *core)
{
	if (core && core->thread_init)
		core->thread_init();
}

/*----------------------------------------------------------------------------*/

int pcore_start (void **core_ptr,
                 pcore_params_t *pcore_params,
                 void *application)
{
	int rc = 0, i;
	pb_core_t *core = NULL;

	if (!pcore_params || !application)
	{
		rc = -1;
		goto err;
	}

	*core_ptr = (pb_core_t *)calloc(1, sizeof (pb_core_t));
	if (!*core_ptr)
	{
		rc = -1;
		goto err;
	}

	core = *core_ptr;

	pcore_init(core);

	if (pcore_params->mode != ePBYTE_SERVER &&
	    pcore_params->mode != ePBYTE_CLIENT)
	{
		rc = -2;
		goto err;
	}

	core->mode = pcore_params->mode;
	core->application = application;

	if (pcore_params->message_handler)
		core->message_handler = pcore_params->message_handler;

	if (pcore_params->error_handler)
		core->error_handler = pcore_params->error_handler;

	if (pcore_params->logger)
	{
		sprintf (core->logger.prefix, "PCORE_%s:",
		         (pcore_params->mode == ePBYTE_SERVER)? "S":"C");

		pb_log_set_logger (&core->logger, pcore_params->logger);
		pb_log_set_prio (&core->logger, pcore_params->logger_prio);
	}

	if (pcore_params->thread_init)
	{
		core->thread_init = pcore_params->thread_init;
	} else {
		pb_log (core, PBYTE_ERR,
		        "Failed to start. "
		        "Thread init callback not set");

		rc = -2;
		goto err;
	}

	if (!pcore_params->addr ||
	    (pcore_params->port <= 0 || 65535 <= pcore_params->port) ||
	    (pcore_params->io_threads <= 0))
	{
		pb_log (core, PBYTE_ERR,
		        "Failed to start. "
		        "Invalid params");

		rc = -3;
		goto err;
	}

	if (pcore_params->worker_count <= 0 ||
	    pcore_params->worker_count >= MAX_WORKER_COUNT)
	{
		core->worker_count = DEF_WORKER_COUNT;
	} else {
		core->worker_count = pcore_params->worker_count;
	}

	sprintf(core->URL, "tcp://%s:%d", pcore_params->addr, pcore_params->port);

	core->ctx = zmq_init (pcore_params->io_threads);
	if (!core->ctx)
	{
		pb_log (core, PBYTE_ERR,
		        "Failed to start. "
		        "Can't init context: %s",
		        zmq_strerror(zmq_errno()));

		rc = -4;
		goto err;
	}

	if (pcore_params->mode == ePBYTE_SERVER)
	{
		core->sock = zmq_socket (core->ctx, ZMQ_ROUTER);
		if (!core->sock)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to start. "
			        "Can't init socket: %s",
			        zmq_strerror(zmq_errno()));

			rc = -5;
			goto err;
		}

		pb_log (core, PBYTE_INFO, "Try bind in '%s'", core->URL);

		rc = zmq_bind (core->sock, core->URL);
		if (rc != 0)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to start. "
			        "Bind failed: %s",
			        zmq_strerror(zmq_errno()));

			rc = -6;
			goto err;
		}
	} else {
		core->sock = zmq_socket (core->ctx, ZMQ_DEALER);
		if (!core->sock)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to start. "
			        "Can't init socket: %s",
			        zmq_strerror(zmq_errno()));

			rc = -5;
			goto err;
		}

		rc = zmq_setsockopt (core->sock,
		                     ZMQ_IDENTITY,
		                     pcore_params->identity.identity,
		                     strlen(pcore_params->identity.identity));
		if (rc != 0)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to set identity: %s",
			        zmq_strerror(zmq_errno()));

			rc = -6;
			goto err;
		}

		pb_log (core, PBYTE_INFO,
		        "Try connect to '%s'",
		        core->URL);

		rc = zmq_connect (core->sock, core->URL);
		if (rc != 0)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to start. "
			        "Connect failed: %s",
			        zmq_strerror(zmq_errno()));

			rc = -6;
			goto err;
		}
	}

	mqueue_init (&core->queue_in, MQUEUESIZE);
	mqueue_init (&core->queue_out, MQUEUESIZE);

	core->pollitem.socket  = core->sock;
	core->pollitem.fd      = 0;
	core->pollitem.events  = ZMQ_POLLIN;
	core->pollitem.revents = 0;

	core->active = 1;

	rc = pthread_create(&core->thread_recv_send,
	                    NULL,
	                    &pcore_thread_recv_send,
	                    (void *)core);
	if(rc < 0)
	{
		pb_log (core, PBYTE_ERR,
		        "Failed to start. "
		        "Can't create recv/send switcher thread: %s",
		        strerror(errno));

		rc = -7;
		goto err;
	}

	for (i = 0; i < core->worker_count; i++)
	{
		rc = pthread_create (&core->thread_worker[i],
		                     NULL,
		                     &pcore_thread_worker,
		                     (void *)core);
		if(rc < 0)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to start. "
			        "Can't create primary worker%d thread: %s",
			        i, strerror(errno));

			rc = -8 - i;
			goto err;
		}
	}

	pb_log (core, PBYTE_INFO,
	        "Start success. Worker count: %d",
	        core->worker_count);

	return 0;

err:
	switch(rc)
	{
		case -7:
			if (core && core->sock)
				zmq_unbind (core->sock, core->URL);
		case -6:
			if (core && core->sock)
				zmq_close (core->sock);
		case -5:
			if (core && core->ctx)
				zmq_ctx_destroy (core->ctx);
		case -4:
		case -3:
		case -2:
			free (core);
		case -1: break;

		default:
			pcore_stop (core);
			break;
	}

	return rc;

}

/*----------------------------------------------------------------------------*/

int pcore_stop(void *core_ptr)
{
	int rc = 0, i;
	pb_core_t *core = NULL;

	if (!core_ptr)
		return 0;

	core = (pb_core_t *)core_ptr;

	pb_log (core, PBYTE_WARN, "Stop detected");

	if (!core->active)
		return 0;

	core->active = 0;

	rc = pthread_join (core->thread_recv_send, NULL);

	for (i = 0; i < core->worker_count; i++)
	{
		rc |= pthread_join (core->thread_worker[i], NULL);
	}

	if (rc == 0)
	{
		sleep (1);
		zmq_unbind (core->sock, core->URL);
		zmq_close (core->sock);
		zmq_ctx_destroy (core->ctx);

		mqueue_close (&core->queue_in);
		mqueue_close (&core->queue_out);

		pcore_init (core);
	}

	pb_log (core, PBYTE_INFO, "Stop success [%d]", rc);

	free (core);

	return rc;
}

/*----------------------------------------------------------------------------*/

static inline void pcore_proc_error (pb_core_t *core, pb_msg_t *pb_msg)
{
	if (core && core->error_handler)
		core->error_handler (core->application, pb_msg);
}

/*----------------------------------------------------------------------------*/

int pcore_msg_send (void *core_ptr, pb_msg_t *pb_msg)
{
	int rc;
	pb_core_t *core = NULL;

	if (!core_ptr || !pb_msg)
		goto err;

	core = (pb_core_t *)core_ptr;

	rc = mqueue_put (&core->queue_out, (void *)pb_msg, 0);
	if (rc < 0)
	{
		pb_log (core, PBYTE_ERR,
		        "Send failed. "
		        "Failed to put message in queue [%d]", rc);
		goto err;
	}

#ifdef QUEUE_DEBUG
		int qelems = mqueue_get_count(&core->queue_in);

		pb_log (core, (qelems > 1)? PBYTE_INFO:PBYTE_CUT,
		        "PUT message to OUTPUT queue. "
		        "Message in OUTPUT queue: %d",
		        qelems);
#endif

	return 0;

err:
	pcore_proc_error (core, pb_msg);

	if (pb_msg)
		pb_msg_unpack_free (&pb_msg);

	return -1;
}

/*----------------------------------------------------------------------------*/

static int pcore_check_send (pb_core_t *core)
{
	int rc = -1, size = 0, budget;
	char *data = NULL, *output_data = NULL;
	pb_msg_t *pb_msg = NULL;
	char identity_str[256] = {0};

	if (!core)
		return -1;

	budget = BUDGET;

	while (--budget > 0)
	{
		data = mqueue_timedget_msec (&core->queue_out, MSG_TIMEOUT);
		if (!data)
			break;

#ifdef QUEUE_DEBUG
		int qelems = mqueue_get_count(&core->queue_out);

		pb_log (core, (qelems > 1)? PBYTE_INFO:PBYTE_CUT,
		        "GET message from OUTPUT queue. "
		        "Message in OUTPUT queue: %d",
		        qelems);
#endif

		pb_msg = (pb_msg_t *)data;

		pb_pb_identity_to_str (&pb_msg->identity, 1, identity_str, sizeof (identity_str));

		pb_log (core, PBYTE_INFO,
		        "%s: Send '%s'",
		        identity_str,
		        pb_pb_msg_type_to_str (pb_msg->message_params.msg_type));

		pb_msg_print (&core->logger, PBYTE_INFO, "output", pb_msg);

		rc = pb_msg_pack (pb_msg, &output_data, &size);
		if (rc < 0 || size <= 0 || !output_data)
		{
			pb_log (core, PBYTE_ERR,
			        "Failed to pack message. "
			        "Skip message");
			goto skip_message;
		}

		// pb_print_buff (&core->logger, PBYTE_INFO,
		//                "output message",
		//                output_data, size);

		if (core->mode == ePBYTE_SERVER)
		{
#ifdef TIME_DEBUG
			pb_print_msg_time(&core->logger, "Output queue -> send", pb_msg);
#endif
			/* send identity first for ZMQ-routing */
			rc = zmq_send (core->sock, pb_msg->identity.identity,
			               pb_msg->identity.identity_len, ZMQ_SNDMORE);
			if (rc != pb_msg->identity.identity_len)
			{
				pb_log (core, PBYTE_WARN,
				        "Send identity failed: %s",
				        zmq_strerror(zmq_errno()));
				goto skip_message;
			}
		}

		rc = zmq_send (core->sock, output_data, size, 0);
		if (rc != size)
		{
			pb_log (core, PBYTE_WARN,
			        "Send message failed: %s",
			        zmq_strerror(zmq_errno()));
			goto skip_message;
		}

		pb_log (core, PBYTE_INFO,
		        "%s: '%s': Message was sent",
		        identity_str,
		        pb_pb_msg_type_to_str(pb_msg->message_params.msg_type));

		pb_msg_pack_free (&output_data);
		pb_msg_unpack_free (&pb_msg);
	}

	return 0;

skip_message:
	pcore_proc_error (core, pb_msg);

	if (output_data)
		pb_msg_pack_free (&output_data);

	if (pb_msg)
		pb_msg_unpack_free (&pb_msg);

	return rc;
}

/*----------------------------------------------------------------------------*/

static int pcore_recv_identity (pb_core_t *core, pb_identity_t *identity)
{
	zmq_msg_t msg;
	int rc, more;

	zmq_msg_init (&msg);

	rc = zmq_recvmsg (core->sock, &msg, 0);
	if (rc < 0)
	{
		pb_log (core, PBYTE_WARN,
		        "Recieve identity failed: %s",
		        zmq_strerror(zmq_errno()));
		zmq_msg_close(&msg);
		return -1;
	}

	more = zmq_msg_more (&msg);
	if (more != 1)
	{
		pb_log (core, PBYTE_WARN,
		        "No more data after identity");
		zmq_msg_close(&msg);
		return -2;
	}

	if (zmq_msg_size (&msg) >= PBYTE_IDENTITY_LEN)
	{
		pb_log (core, PBYTE_WARN,
		        "Recieve message failed. "
		        "Large identity");
		zmq_msg_close(&msg);
		return -3;
	}

	// pb_print_buff (&core->logger, PBYTE_INFO,
	//                "incomming identity",
	//                zmq_msg_data(msg),
	//                zmq_msg_size(msg));

	memcpy (identity->identity, zmq_msg_data (&msg), zmq_msg_size (&msg));
	identity->identity_len = zmq_msg_size (&msg);

	zmq_msg_close(&msg);

	return 0;
}

/*----------------------------------------------------------------------------*/

static int pcore_check_recv (pb_core_t *core)
{
	int rc, count, budget;
	zmq_msg_t msg;
	pb_msg_t *pb_msg = NULL;
	pb_identity_t identity = {0};

	budget = BUDGET;

	while (--budget > 0)
	{
		count = zmq_poll (&core->pollitem, 1, MSG_TIMEOUT);
		if (!count)
			break;

		zmq_msg_init (&msg);

		if (count < 0) {
			pb_log (core, PBYTE_WARN, "Poll: %s", zmq_strerror(zmq_errno()));
			goto skip_message;
		}

		if (core->mode == ePBYTE_SERVER)
		{
			/* identity comes first */
			if (pcore_recv_identity (core, &identity) != 0)
				goto skip_message;
		}

		/* then the first part of the message body */
		rc = zmq_recvmsg (core->sock, &msg, 0);
		if (rc <= 0)
		{
			pb_log (core, PBYTE_WARN,
			        "Recieve message failed: %s",
			        zmq_strerror(zmq_errno()));
			goto skip_message;
		}

		// pb_print_buff (&core->logger, PBYTE_INFO,
		//                "incomming message",
		//                zmq_msg_data(msg),
		//                zmq_msg_size(msg));

		rc = pb_msg_unpack (&pb_msg, (char *)zmq_msg_data(&msg), zmq_msg_size(&msg));
		if (rc != 0)
		{
			pb_log (core, PBYTE_WARN,
			        "Recieve message failed. "
			        "Failed to unpack message");
			goto skip_message;
		}

		pb_msg_set_identity (pb_msg, &identity);

#ifdef TIME_DEBUG
		pb_clockgettime(&pb_msg->start);
#endif

		rc = mqueue_put(&core->queue_in, (void *)pb_msg, 0);
		if (rc < 0)
		{
			pb_log (core, PBYTE_WARN,
			        "Recieve message failed. "
			        "Failed to put message in queue");
			goto skip_message;
		}

#ifdef QUEUE_DEBUG
		int qelems = mqueue_get_count(&core->queue_in);

		pb_log (core, (qelems > 1)? PBYTE_INFO:PBYTE_CUT,
		        "PUT message to INPUT queue. "
		        "Messages in INPUT queue: %d",
		        qelems);
#endif

		pb_msg = NULL;
		zmq_msg_close(&msg);
	}

	return 0;

skip_message:
	if (pb_msg)
		pb_msg_unpack_free (&pb_msg);

	zmq_msg_close(&msg);

	return -1;
}

/*----------------------------------------------------------------------------*/

static void *pcore_thread_recv_send(void *arg)
{
	pb_core_t *core = NULL;

	prctl(PR_SET_NAME, "pbyte_rs_switcher", 0, 0, 0);

	if (!arg)
		goto err;

	core = (pb_core_t *)arg;

	pcore_thread_init (core);

	pb_log (core, PBYTE_INFO, "Recv/Send switcher thread: Inited");

	while (core->active)
	{
		pcore_check_send (core);
		pcore_check_recv (core);
	}

	pb_log (core, PBYTE_INFO, "Recv/Send switcher thread: Normal exit");

	return NULL;

err:
	pb_log (core, PBYTE_ERR, "Recv/Send switcher thread: Force exit");

	return NULL;
}

/*----------------------------------------------------------------------------*/

static void *pcore_thread_worker(void *arg)
{
	pb_core_t *core = NULL;
	pb_msg_t *pb_msg = NULL;

	prctl(PR_SET_NAME, "pbyte_worker", 0, 0, 0);

	if (!arg)
		return NULL;

	core = (pb_core_t *)arg;

	pcore_thread_init (core);

	pb_log (core, PBYTE_INFO, "Worker thread: Inited");

	while (core->active)
	{
		pb_msg = (pb_msg_t *)mqueue_timedget_msec(&core->queue_in, MSG_TIMEOUT);
		if (pb_msg != NULL)
		{

#ifdef QUEUE_DEBUG
		int qelems = mqueue_get_count(&core->queue_in);

		pb_log (core, (qelems > 1)? PBYTE_INFO:PBYTE_CUT,
		        "GET message from INPUT queue. "
		        "Message in INPUT queue: %d",
		        qelems);
#endif

			pb_msg_print(&core->logger, PBYTE_INFO,
			            "input", pb_msg);

#ifdef TIME_DEBUG
			pb_print_msg_time(&core->logger, "Input queue -> RServer", pb_msg);
#endif

			if (core->message_handler)
				core->message_handler(core->application, pb_msg);

			pb_msg_unpack_free (&pb_msg);
		}
	}

	pb_log (core, PBYTE_INFO, "Worker thread: Normal exit");

	return NULL;
}
