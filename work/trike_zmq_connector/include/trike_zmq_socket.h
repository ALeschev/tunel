#ifndef _TRIKE_ZMQ_SOCKET_H
#define _TRIKE_ZMQ_SOCKET_H

#include "trike_msg_handler.h"
#include "trike_message_queue_api.h"
#include "trike_transport_context.h"

#define MAX_ZMQ_ITEMS 100


typedef enum {
    FREE,
    BUSY
} socket_status_t;

typedef enum {
    REQUEST,
    REPLY,
    NOTIFY
}socket_type_t;

typedef struct trike_zmq_socket{
    char *socket_addr;
    int socket_port;
    void *socket;
    socket_status_t status;
    socket_type_t type;
    trike_queue_t *socket_queue;
    trike_handle_msg_cb_t handle_msg;
    trike_transport_context_t *transport_context;
} trike_zmq_socket_t;

#endif
