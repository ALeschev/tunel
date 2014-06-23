#ifndef  _TRIKE_ZMQ_MSG_API_H
#define _TRIKE_ZMQ_MSG_API_H

#include "trike_zmq_msg.h"

trike_zmq_msg_t *trike_zmq_msg_new(void *data,int size);
void trike_zmq_msg_free(trike_zmq_msg_t *msg);
int trike_make_zmq_msg(trike_zmq_msg_t *msg,void *data,int size);
int trike_init_zmq_msg(trike_zmq_msg_t *msg,void *bytes);
int trike_ser_zmq_msg(trike_zmq_msg_t *msg,void **buf);
int trike_dser_zmq_msg(trike_zmq_msg_t **msg,void *bytes);

#endif
