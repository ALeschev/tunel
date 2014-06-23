#include "trike_msg_api.h"

#include <stdio.h>
/*-------------------------------------------------------------------------------------*/
/* API functions                                                                       */
/*-------------------------------------------------------------------------------------*/
trike_msg_t *trike_msg_new(){
    return (trike_msg_t*)malloc(sizeof(trike_msg_t));
}

void trike_msg_free(trike_msg_t *msg){
    free(msg);
};

int trike_send_msg(trike_transport_context_t *transport_context,void *data,int size){
    trike_msg_t *trike_msg=trike_msg_new();
    trike_zmq_msg_t *trike_zmq_msg=trike_zmq_msg_new(data,size);
    if(!transport_context) goto fail;
    if(!data) goto fail;
    if(!trike_msg) goto fail;
    trike_msg->transport_context=transport_context;
    if(!trike_zmq_msg) goto fail;
    trike_msg->trike_zmq_msg=trike_zmq_msg;
    //Push to worker queue
    trike_push_task_c(transport_context->c_queue,trike_msg_ser(trike_msg));
    return 0;
 fail:
    if(trike_msg) trike_msg_free(trike_msg);
    if(trike_zmq_msg) trike_zmq_msg_free(trike_zmq_msg);
    return -1;
}

void *trike_msg_ser(trike_msg_t *trike_msg){
    return (void*)trike_msg;
}

trike_msg_t *trike_msg_dser(void *trike_msg){
    return (trike_msg_t*)trike_msg;
}
