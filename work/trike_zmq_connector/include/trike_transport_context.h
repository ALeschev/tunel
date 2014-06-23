#ifndef _TRIKE_TRANSPORT_CONTEXT
#define _TRIKE_TRANSPORT_CONTEXT

#include "trike_message_concurrent_queue.h"

typedef struct trike_transport_context {
    void *socket;
    trike_concurrent_queue_t *c_queue;
    void *user_data;
} trike_transport_context_t;

#endif
