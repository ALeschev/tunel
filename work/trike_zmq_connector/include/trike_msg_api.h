#ifndef _TRIKE_MSG_API_H
#define _TRIKE_MSG_API_H

#include <stdlib.h>
#include <string.h>

#include "trike_msg.h"
#include "trike_zmq_msg.h"
#include "trike_zmq_msg_api.h"
#include "trike_transport_context.h"

trike_msg_t *trike_msg_new();
void trike_msg_free(trike_msg_t *msg);
int trike_send_msg(trike_transport_context_t *transport_context,void *data,int size);
void *trike_msg_ser(trike_msg_t *trike_msg);
trike_msg_t *trike_msg_dser(void *trike_msg);

#endif
