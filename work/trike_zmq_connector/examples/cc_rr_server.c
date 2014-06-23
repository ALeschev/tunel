#include <stdio.h>
#include <string.h>

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_rr.h"

int handle_request(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg);

int c=10;

int main(){
	trike_worker_context_t *worker_context=start_worker(NULL);
	trike_transport_context_t *client_context;
	char addr[]="tcp://*";
	client_context=add_rr_server(worker_context,make_rr_reply(.handle_request=handle_request,
                                                                  .req_port=50500,
                                                                  .rep_port=50500,
                                                                  .addr=addr));
	printf("Started receiver example\n");
	if(!client_context) {printf("Sorry, error happens\n"); return 0;}
	printf("Try to send requests to server\n");
        while(c){};
	return 0;
}

int handle_request(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg){
    char buf[]="CFW 000000000001 200\r\n"
	"Keep-Alive: 600\r\n"
	"Packages: msc-ivr/1.0,msc-mixer/1.0\r\n";
    trike_cc_rr_msg_t trike_cc_rr_msg;
    trike_cc_rr_msg.dialog_id_data=msg->dialog_id_data;
    trike_cc_rr_msg.dialog_id_size=msg->dialog_id_size;
    trike_cc_rr_msg.request_id=msg->request_id;
    trike_cc_rr_msg.size=sizeof(msg->data);
    trike_cc_rr_msg.data=msg->data;

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


    //printf("Received request with id=%i: %s\n",msg->dialog_id_data, msg->request_id,(char*)msg->data);
    trike_send_cc_rr_msg(transport_context,trike_cc_rr_msg);
    --c;
    return 0;
}
