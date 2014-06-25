#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_rr.h"

#ifndef ERL_TEST
#	include "PongType.h"
#endif

#define CHECK_TIME_MS 5000000 /*5s*/

#define C_ALIVE 1
#define C_DEAD 0
#define MAX_PING_CNT 5

#define CHECK_REQ "ping"
#define CHECK_ANS "pong"

int handle_request(trike_transport_context_t *transport_context, trike_cc_rr_msg_t *msg);

static int c = 100;

trike_worker_context_t *worker_context;

static int client_status = 0;
static int ping_cnt = 0;

void sig_handler (int sig)
{
	printf ("sig_handler [%d]\n", sig);

	stop_worker(worker_context);

	exit(0);
}

int main()
{
	trike_transport_context_t *server_context;
	trike_gen_cc_reply_t server_params;

	char addr[]="tcp://*";

	signal (SIGINT, sig_handler);
	signal (SIGTERM, sig_handler);

	worker_context = start_worker(NULL);

	server_params.handle_request = handle_request;
	server_params.req_port = 50500;
	server_params.rep_port = 50502;
	server_params.addr = addr;

	server_context = add_rr_server(worker_context, server_params);

	printf("Started receiver example\n");

	if(!server_context)
	{
		printf("Sorry, error happens. (%s)\n", strerror(errno));
		return 0;
	}

	while(c)
	{
		usleep(CHECK_TIME_MS); /*100 ms*/

		if (client_status == C_ALIVE)
		{
			client_status = C_DEAD;
			ping_cnt = 0;
		} else {
			if (ping_cnt >= MAX_PING_CNT)
			{
				printf("Client disconnected!\n");
				ping_cnt = 0;
			}

			trike_cc_rr_msg_t trike_cc_rr_msg;

			trike_cc_rr_msg.dialog_id_data = CHECK_REQ;
			trike_cc_rr_msg.dialog_id_size = strlen(CHECK_REQ);
			trike_cc_rr_msg.request_id     = 0;
			trike_cc_rr_msg.size = strlen(CHECK_REQ);
			trike_cc_rr_msg.data = CHECK_REQ;

			trike_send_cc_rr_msg(server_context, trike_cc_rr_msg);

			ping_cnt++;
		}

		printf ("ping_cnt: %d\n", ping_cnt);
	}

	return 0;
}

int handle_request(trike_transport_context_t *transport_context, trike_cc_rr_msg_t *msg)
{
	int res;

	trike_cc_rr_msg_t trike_cc_rr_msg;

	trike_cc_rr_msg.dialog_id_data = msg->dialog_id_data;
	trike_cc_rr_msg.dialog_id_size = msg->dialog_id_size;
	trike_cc_rr_msg.request_id     = msg->request_id;

#ifdef ERL_TEST

	char msg_err_ans[] = "err";
	char msg_ans[] = "msg out";

	char buff[256] = {0};

	printf("Received request:\n"
           "msg->dialog_id_data: %s\n"
           "msg->dialog_id_size: %d\n"
           "msg->request_id: %d\n"
           "msg->size: %d\n"
           "msg->data: %s\n",
           (char *)msg->dialog_id_data,
           msg->dialog_id_size,
           msg->request_id,
           msg->size,
           (char*)msg->data
           );

	trike_cc_rr_msg.size = sizeof(msg_err_ans) - 1;
	trike_cc_rr_msg.data = msg_err_ans;

	if (msg->size < 256)
	{
		memcpy (buff, msg->data, msg->size);
	} else {
		printf ("invalid msg size: %d\n", msg->size);
		goto send;
	}

	if (!strcmp(buff, "msg in"))
	{
		trike_cc_rr_msg.size = sizeof(msg_ans) - 1;
		trike_cc_rr_msg.data = msg_ans;
	} else
	if (!strcmp(buff, CHECK_REQ))
	{
		trike_cc_rr_msg.size = sizeof(CHECK_ANS) - 1;
		trike_cc_rr_msg.data = CHECK_ANS;
		goto send;
	} else
	if (!strcmp(buff, CHECK_ANS))
	{
		return 0;
	}

	printf ("--------------------\n");

#else
		asn_dec_rval_t rval;
		PongType_t *c_pong;

	printf("Received request:\n"
           "msg->dialog_id_data: %s\n"
           "msg->dialog_id_size: %d\n"
           "msg->request_id: %d\n"
           "msg->size: %d\n",
           (char *)msg->dialog_id_data,
           msg->dialog_id_size,
           msg->request_id,
           msg->size);

		//uper_decode(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td, void **sptr, const void *buffer, size_t size, int skip_bits, int unused_bits)
		rval = uper_decode_complete(0, &asn_DEF_PongType, (void **)&c_pong, msg->data, msg->size);
		assert(c_pong);

		switch(rval.code) {
			case RC_OK:
				printf ("uper_decode_complete: RC_OK\n");
				break;
			case RC_FAIL:
				printf ("uper_decode_complete: RC_FAIL\n");
				return 0;
			case RC_WMORE:
				printf ("uper_decode_complete: RC_WMORE\n");
				return 0;
			}

		xer_fprint(stdout, &asn_DEF_PongType, c_pong);
#endif

send:
	res = trike_send_cc_rr_msg(transport_context, trike_cc_rr_msg);

	printf("Send message %s!\n", res? "failed":"ok");

	--c;

	client_status = C_ALIVE;

	return 0;
}
