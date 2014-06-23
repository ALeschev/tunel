#ifndef _TRIKE_ZMQ_SOCKET_API_H
#define _TRIKE_ZMQ_SOCKET_API_H

#include "trike_transport_context.h"
#include "trike_zmq_msg.h"
#include "trike_zmq_socket.h"

trike_zmq_socket_t *trike_zmq_socket_new(void *socket);
int trike_zmq_socket_set_cb(trike_zmq_socket_t *trike_socket,trike_handle_msg_cb_t cb);
int trike_zmq_socket_set_type(trike_zmq_socket_t *trike_socket,int flag);
int trike_zmq_socket_set_transport_context(trike_zmq_socket_t *trike_socket,trike_transport_context_t *transport_context);
trike_zmq_socket_t *trike_zmq_socket_get(trike_zmq_socket_t **trike_socket,void *socket);
int receive_zmq_message(trike_zmq_socket_t *trike_socket,int flags);
int send_zmq_message(trike_zmq_socket_t *trike_socket,trike_zmq_msg_t *trike_msg);

#endif
