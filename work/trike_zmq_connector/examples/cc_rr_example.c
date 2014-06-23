#include <stdio.h>
#include <string.h>

#include "PongType.h"

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_rr.h"

#define MESSAGE_COUNT 10
#define MAX_SIZE 255
#define WAIT_COUNTER(CUR_COUNT,COUNT) { while(CUR_COUNT<COUNT){}};

int handle_request(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg);
int handle_reply(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg);

volatile int count=0;

void init_cb(void);

int main(){
    trike_worker_context_t *worker_context=start_worker(init_cb);
    trike_transport_context_t *client_context;
    char bind_addr[]="tcp://*";
    char addr[]="tcp://127.0.0.1";
    char data[]="ping";
        char buf[MAX_SIZE];
        trike_cc_rr_msg_t trike_cc_rr_msg;
        int i;

/*  add_rr_server(worker_context,make_rr_reply(.handle_request=handle_request,
                                                   .req_port=50500,
                                                   .rep_port=50502,
                                                   .addr=bind_addr));*/
    client_context=add_rr_client(worker_context,make_rr_request(.handle_reply=handle_reply,
                                                                    .req_port=50500,
                                                                    .rep_port=50502,
                                                                    .addr=addr));
    printf("Started receive-response example\n");
    if(!client_context) {printf("Sorry, error happens\n"); return 0;}
    printf("Try to send requests to server\n");

        for(i=0;i<MESSAGE_COUNT;i++){
            //snprintf(buf,MAX_SIZE,"%s_%i",data,i);
            int type_size = sizeof(PongType_t);
            char buffer[sizeof(PongType_t)] = {0};
            //void *buffer = 0;
            PongType_t *c_pong;

            c_pong = calloc(1, type_size);
            assert(c_pong);

            OCTET_STRING_fromBuf (&c_pong->buff, "ping", 4);

            xer_fprint(stdout, &asn_DEF_PongType, c_pong);

// ssize_t uper_encode_to_new_buffer(
//     struct asn_TYPE_descriptor_s *type_descriptor,
//     asn_per_constraints_t *constraints,
//     void *struct_ptr,   /* Structure to be encoded 
//     void **buffer_r     /* Buffer allocated and returned */
// );

        //uper_encode_to_buffer(asn_TYPE_descriptor_t *td, void *sptr, void *buffer, size_t buffer_size)
        asn_enc_rval_t rvalue;
        rvalue = uper_encode_to_buffer (&asn_DEF_PongType, c_pong, buffer, type_size);

        if (rvalue.encoded < 0)
        {
            printf ("%d endcode failed\n", i);
            continue;
        }

        trike_cc_rr_msg.dialog_id_data="42";
        trike_cc_rr_msg.dialog_id_size=2;
        trike_cc_rr_msg.request_id=i;
        trike_cc_rr_msg.size=type_size;
        trike_cc_rr_msg.data=buffer;

        PongType_t *p_pong;
        asn_dec_rval_t rval;

        rval = uper_decode_complete(0, &asn_DEF_PongType, (void **)&p_pong, buffer, type_size);
        assert (p_pong);

        switch(rval.code) {
            case RC_OK:
                printf ("uper_decode_complete: RC_OK\n");
                break;
            case RC_FAIL:
                printf ("uper_decode_complete: RC_FAIL\n");
                break;
            case RC_WMORE:
                printf ("uper_decode_complete: RC_WMORE\n");
                break;
            }

            trike_send_cc_rr_msg(client_context,trike_cc_rr_msg);

            FREEMEM(buffer);

            free(c_pong);
        }

    WAIT_COUNTER(count,MESSAGE_COUNT);
        printf("All message sended.");
    return 0;
}

/*int handle_request(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg){
    char buf[]="response";
    trike_cc_rr_msg_t trike_cc_rr_msg;
    trike_cc_rr_msg.dialog_id_data=msg->dialog_id_data;
    trike_cc_rr_msg.dialog_id_size=msg->dialog_id_size;
    trike_cc_rr_msg.request_id=msg->request_id;
    trike_cc_rr_msg.size=sizeof(buf);
    trike_cc_rr_msg.data=buf;
    printf("Received request with id=%i: %s\n",msg->request_id,(char*)msg->data);

    printf("Received request:\n"
           "msg->dialog_id_data: %s\n"
           "msg->dialog_id_size: %d\n"
           "msg->request_id: %d\n"
           "msg->size: %d\n"
           "msg->data: %s\n",
           (char *)&msg->dialog_id_data,
           msg->dialog_id_size,
           msg->request_id,
           msg->size,
           (char*)msg->data
           );


    trike_send_cc_rr_msg(transport_context,trike_cc_rr_msg);
    return 0;
}*/

int handle_reply(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg){
    char response[msg->size];
    memcpy(response,msg->data,msg->size);
    printf("\nReceived response with request_id=%i: %s\n",msg->request_id, response);
    count++;
    return 0;
}

void init_cb(void){
    printf("INIT WORKER\n");
}
