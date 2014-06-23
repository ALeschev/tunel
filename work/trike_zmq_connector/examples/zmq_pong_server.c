#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_rr.h"

//#define ERL_TEST 1

#ifndef ERL_TEST
#	include "PongType.h"
#endif

int handle_request(trike_transport_context_t *transport_context, trike_cc_rr_msg_t *msg);

int c = 100;

int main()
{
	trike_worker_context_t *worker_context = start_worker(NULL);
	trike_transport_context_t *server_context;
	trike_gen_cc_reply_t server_params;

	char addr[]="tcp://*";

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

	while(c){}

	stop_worker(worker_context);

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

	char err_ans[] = "err";
	char pong_ans[] = "pong";

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

	trike_cc_rr_msg.size = sizeof(err_ans) - 1;
	trike_cc_rr_msg.data = err_ans;

	if (msg->size < 256)
	{
		memcpy (buff, msg->data, msg->size);
	} else {
		printf ("invalid msg size: %d\n", msg->size);
		goto send;
	}

	if (!strcmp(buff, "ping"))
	{
		trike_cc_rr_msg.size = sizeof(pong_ans) - 1;
		trike_cc_rr_msg.data = pong_ans;
	}

	printf("\nSend answer:\n"
           "msg->dialog_id_data: %s\n"
           "msg->dialog_id_size: %d\n"
           "msg->request_id: %d\n"
           "msg->size: %d\n"
           "msg->data: %s\n",
           (char *)trike_cc_rr_msg.dialog_id_data,
           trike_cc_rr_msg.dialog_id_size,
           trike_cc_rr_msg.request_id,
           trike_cc_rr_msg.size,
           (char*)trike_cc_rr_msg.data
           );

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

	return 0;
}
