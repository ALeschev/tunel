#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_rr.h"

int handle_request(trike_transport_context_t *transport_context, trike_cc_rr_msg_t *msg);

int c = 10;

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
	char err_ans[] = "err";
	char pong_ans[] = "pong";

	char *buff;

	trike_cc_rr_msg_t trike_cc_rr_msg;

	memcpy (&trike_cc_rr_msg, msg, sizeof (trike_cc_rr_msg_t));

/*	trike_cc_rr_msg.dialog_id_data = msg->dialog_id_data;
	trike_cc_rr_msg.dialog_id_size = msg->dialog_id_size;
	trike_cc_rr_msg.request_id     = msg->request_id;

	trike_cc_rr_msg.size           = sizeof(err_ans);
	trike_cc_rr_msg.data           = err_ans;*/

	// if (strcmp((char*)msg->data, "pin"))
	// {
	// 	trike_cc_rr_msg.size           = sizeof(err_ans);
	// 	trike_cc_rr_msg.data           = err_ans;
	// } else {
	// 	trike_cc_rr_msg.size           = sizeof(pong_ans);
	// 	trike_cc_rr_msg.data           = pong_ans;
	// }

//	printf("Received request with id=%i: %s\n", msg->request_id, (char*)msg->data);

    printf("Received request:\n"
           "msg->dialog_id_data: %s\n"
           "msg->dialog_id_size: %d\n"
           "msg->request_id: %d\n"
           "msg->data: %s\n",
           (char *)msg->dialog_id_data,
           msg->dialog_id_size,
           msg->request_id,
           (char*)msg->data
           );

    printf("Send answer:\n"
           "msg->dialog_id_data: %s\n"
           "msg->dialog_id_size: %d\n"
           "msg->request_id: %d\n"
           "msg->data: %s\n",
           (char *)trike_cc_rr_msg.dialog_id_data,
           trike_cc_rr_msg.dialog_id_size,
           trike_cc_rr_msg.request_id,
           (char*)trike_cc_rr_msg.data
           );

	trike_send_cc_rr_msg(transport_context, trike_cc_rr_msg);

//	printf("Send answer with id=%i: %s\n", msg->request_id, (char *)trike_cc_rr_msg.data);

	--c;

	return 0;
}
