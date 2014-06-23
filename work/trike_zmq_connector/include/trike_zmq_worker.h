#ifndef _TRIKE_ZMQ_WORKER_H
#define _TRIKE_ZMQ_WORKER_H

#include <zmq.h>
#include "trike_message_concurrent_queue.h"
#include "trike_zmq_socket.h"

#define POLL_TIMEOUT 1
#define MAX_URL_SIZE 255

typedef enum {INACTIVE,ACTIVE} worker_status;

typedef void (*init_cb_t)(void);

typedef struct trike_worker_context{
    pthread_t pthread;
    worker_status status;
    void *zmq_context;
    // ZMQ polling items
    zmq_pollitem_t zmq_items[MAX_ZMQ_ITEMS];
    trike_zmq_socket_t *trike_sockets[MAX_ZMQ_ITEMS];
    int nitems;
    //Queue
    trike_concurrent_queue_t *c_queue;
    // Flags
    long flags;
    init_cb_t init_cb;
    // Locks
    pthread_mutex_t service_lock;
} trike_worker_context_t;

#endif
