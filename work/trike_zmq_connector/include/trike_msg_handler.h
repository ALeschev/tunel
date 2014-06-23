#ifndef _TRIKE_MSG_HANDLER_H
#define _TRIKE_MSG_HANDLER_H

#include "trike_transport_context.h"
typedef int (*trike_handle_msg_cb_t)(trike_transport_context_t *transport_context,void *data,int size);

#endif
