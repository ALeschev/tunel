#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_store.h"

#define MESSAGE_COUNT 10
#define MAX_SIZE 255
#define WAIT_COUNTER(CUR_COUNT,COUNT) { while(CUR_COUNT<COUNT){}};

int handle_request(trike_transport_context_t *transport_context,trike_cc_store_msg_t *msg);
int handle_reply(trike_transport_context_t *transport_context,trike_cc_store_msg_t *msg);

volatile int count=0;

int main(){
    trike_worker_context_t *worker_context=start_worker(NULL);
    trike_transport_context_t *client_context;
    trike_cc_store_msg_t trike_cc_store_msg;
    char bind_addr[]="tcp://*";
    char addr[]="tcp://127.0.0.1";
    char data[]="asd";
    char buf[MAX_SIZE];
    int i;
    add_store_server(worker_context,make_store_reply(.handle_request=handle_request,
						     .port=50500,
						     .addr=bind_addr));
    client_context=add_store_client(worker_context,make_store_request(.handle_reply=handle_reply,
								      .port=50500,
								      .addr=addr));
    printf("Started receive-response example\n");
    if(!client_context) {printf("Sorry, error happens\n"); return 0;}
    printf("Try to send store requests to server\n");
    for(i=0;i<MESSAGE_COUNT;i++){
	snprintf(buf,MAX_SIZE,"%s_%i",data,i);
	trike_cc_store_msg.dialog_id_data=buf;
	trike_cc_store_msg.dialog_id_size=sizeof(buf);
	trike_cc_store_msg.request_id=i;
	trike_cc_store_msg.size=strlen(buf);
	trike_cc_store_msg.data=buf;
	trike_send_cc_store_msg(client_context,trike_cc_store_msg);
    }
    WAIT_COUNTER(count,MESSAGE_COUNT);
    printf("All message sended.");
    return 0;
}

int handle_request(trike_transport_context_t *transport_context,trike_cc_store_msg_t *msg){
    trike_cc_store_msg_t trike_cc_store_msg;
    char buf[]="response";
    trike_cc_store_msg.dialog_id_data=msg->dialog_id_data;
    trike_cc_store_msg.dialog_id_size=msg->dialog_id_size;
    trike_cc_store_msg.request_id=msg->request_id;
    trike_cc_store_msg.size=sizeof(buf);
    trike_cc_store_msg.data=buf;
    printf("Received request with id=%i: %s\n",msg->request_id,(char*)msg->data);
    trike_send_cc_store_msg(transport_context,trike_cc_store_msg);
    return 0;
}

int handle_reply(trike_transport_context_t *transport_context,trike_cc_store_msg_t *msg){
    char response[msg->size];
    memcpy(response,msg->data,msg->size);
    printf("Received response with request_id=%i: %s\n",msg->request_id,response);
    count++;
    return 0;
}
