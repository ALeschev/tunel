#include <stdlib.h>
#include <string.h>
#include "trike_zmq_msg_api.h"

trike_zmq_msg_t *trike_zmq_msg_new(void *data,int size){
    trike_zmq_msg_t *trike_zmq_msg=(trike_zmq_msg_t*)malloc(sizeof(trike_zmq_msg_t));
    trike_zmq_msg->data=malloc(size);
    memcpy(trike_zmq_msg->data,data,size);
    trike_zmq_msg->size=size;
    return trike_zmq_msg;
}

void trike_zmq_msg_free(trike_zmq_msg_t *trike_zmq_msg){
    free(trike_zmq_msg->data);
    free(trike_zmq_msg);
}

int trike_make_zmq_msg(trike_zmq_msg_t *msg,void *data,int size){
    if(!msg) return -1;
    msg->data=data;
    msg->size=size;
    return 0;
}

int trike_ser_zmq_msg(trike_zmq_msg_t *msg,void **buf){
    if((buf==NULL)||(msg==NULL)) return -1;
    *buf=(void*)msg;
    return 0;
}

int trike_dser_zmq_msg(trike_zmq_msg_t **msg,void *bytes){
    if((bytes==NULL)||(msg==NULL)) return -1;
    *msg=(trike_zmq_msg_t *)bytes;
    return 0;
}
