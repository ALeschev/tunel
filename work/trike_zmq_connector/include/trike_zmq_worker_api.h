#ifndef _TRIKE_ZMQ_WORKER_API_H
#define _TRIKE_ZMQ_WORKER_API_H

#include "trike_transport_context.h"
#include "trike_msg_handler.h"
#include "trike_zmq_worker.h"

//Start/stop worker function
trike_worker_context_t *start_worker(init_cb_t init_cb);
int stop_worker(trike_worker_context_t *worker_context);
//Add connection functions
trike_transport_context_t *add_connection(trike_worker_context_t *worker_context,trike_handle_msg_cb_t cb,void *user_data,const char *addr,int port,int type);
trike_transport_context_t *add_listener(trike_worker_context_t *worker_context,trike_handle_msg_cb_t cb,void *user_data,const char *addr,int port,int type);
int set_option(trike_transport_context_t *transport_context,int name,const void *key,size_t value);

#endif
