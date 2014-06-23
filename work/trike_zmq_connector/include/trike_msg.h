#ifndef _TRIKE_MSG_H
#define _TRIKE_MSG_H

#include <stdlib.h>

#include "trike_zmq_msg.h"
#include "trike_transport_context.h"

typedef struct trike_msg{
    trike_transport_context_t *transport_context;
    trike_zmq_msg_t *trike_zmq_msg;
} trike_msg_t;

#endif
