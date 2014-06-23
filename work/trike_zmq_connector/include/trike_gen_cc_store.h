#ifndef _TRIKE_GEN_CC_STORE_H
#define _TRIKE_GEN_CC_STORE_H

#include "trike_msg_handler.h"
#include "trike_cc_message.pb-c.h"

typedef struct trike_cc_store_msg_{
    void *dialog_id_data;
    int dialog_id_size;
    int request_id;
    void *data;
    int size;
}trike_cc_store_msg_t;

typedef int (*trike_handle_cc_store_msg_cb_t)(trike_transport_context_t *transport_context,trike_cc_store_msg_t *msg);

typedef struct trike_gen_cc_store_request{
    char *addr;
    int port;
    trike_handle_cc_store_msg_cb_t handle_reply;
    void *user_data;
} trike_gen_cc_store_request_t;

typedef struct trike_gen_cc_store_reply{
    char *addr;
    int port;
    trike_handle_cc_store_msg_cb_t handle_request;
    void *user_data;
} trike_gen_cc_store_reply_t;

#define make_store_request(...) (trike_gen_cc_store_request_t){__VA_ARGS__}
#define make_store_reply(...) (trike_gen_cc_store_reply_t){__VA_ARGS__}

trike_transport_context_t  *add_store_client(trike_worker_context_t *worker_context,trike_gen_cc_store_request_t request);
trike_transport_context_t *add_store_server(trike_worker_context_t *worker_context,trike_gen_cc_store_reply_t reply);
int trike_send_cc_store_msg(trike_transport_context_t *transport_context,trike_cc_store_msg_t msg);

#endif
