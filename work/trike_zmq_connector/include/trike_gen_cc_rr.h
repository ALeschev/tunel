#define MAX_ADDR_SIZE 255

#include "trike_transport_context.h"
#include "trike_cc_message.pb-c.h"

typedef struct trike_cc_rr_msg_{
    void *dialog_id_data;
    int dialog_id_size;
    int request_id;
    void *data;
    int size;
}trike_cc_rr_msg_t;

typedef int (*trike_handle_cc_rr_msg_cb_t)(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg);

typedef struct trike_gen_cc_request_ {
    char *addr;
    int req_port;
    int rep_port;
    trike_handle_cc_rr_msg_cb_t handle_reply;
    void *user_data;
}trike_gen_cc_request_t;


typedef struct trike_gen_cc_reply_ {
    char *addr;
    int rep_port;
    int req_port;
    trike_handle_cc_rr_msg_cb_t handle_request;
    void *user_data;
}trike_gen_cc_reply_t;

#define make_rr_request(...) (trike_gen_cc_request_t){__VA_ARGS__}
#define make_rr_reply(...) (trike_gen_cc_reply_t){__VA_ARGS__}

trike_transport_context_t  *add_rr_client(trike_worker_context_t *worker_context,trike_gen_cc_request_t request);

trike_transport_context_t *add_rr_server(trike_worker_context_t *worker_context,trike_gen_cc_reply_t reply);
int trike_send_cc_rr_msg(trike_transport_context_t *transport_context,trike_cc_rr_msg_t msg);
