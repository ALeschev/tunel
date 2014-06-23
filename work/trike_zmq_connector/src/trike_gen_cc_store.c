#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_store.h"

typedef struct gen_cc_store_ctx{
    trike_handle_cc_store_msg_cb_t handle_request;
    trike_handle_cc_store_msg_cb_t handle_reply;
    void *user_data;
} gen_cc_store_ctx_t;

gen_cc_store_ctx_t *gen_cc_store_ctx_new(){
    return (gen_cc_store_ctx_t*)malloc(sizeof(gen_cc_store_ctx_t));
}

void gen_cc_store_ctx_free(gen_cc_store_ctx_t *gen_cc_store_ctx){
    free(gen_cc_store_ctx);
}

// Internal function declarations
int cc_store_handle_request(trike_transport_context_t *transport_context,void *data,int size);
int cc_store_handle_reply(trike_transport_context_t *transport_context,void *data,int size);
static int compose_trike_cc_store_msg(TrikeCCMessage *trike_cc_msg,trike_cc_store_msg_t *trike_cc_store_msg);

trike_transport_context_t *add_store_server(trike_worker_context_t *worker_context,trike_gen_cc_store_reply_t reply){
    trike_transport_context_t *transport_context;
    gen_cc_store_ctx_t *gen_cc_store_ctx;
    if(!reply.addr)
        goto fail;
    if(!reply.port)
        goto fail;
    gen_cc_store_ctx=gen_cc_store_ctx_new();
    gen_cc_store_ctx->user_data=reply.user_data;
    gen_cc_store_ctx->handle_request=reply.handle_request;
    transport_context=add_listener(worker_context,cc_store_handle_request,(void*)gen_cc_store_ctx,reply.addr,reply.port,ZMQ_REP);
    return transport_context;
 fail:
    return NULL;
}

trike_transport_context_t  *add_store_client(trike_worker_context_t *worker_context,trike_gen_cc_store_request_t request){
    trike_transport_context_t *transport_context;
    gen_cc_store_ctx_t *gen_cc_store_ctx;
    if(!request.addr)
        goto fail;
    if(!request.port)
        goto fail;
    gen_cc_store_ctx=gen_cc_store_ctx_new();
    gen_cc_store_ctx->user_data=request.user_data;
    gen_cc_store_ctx->handle_reply=request.handle_reply;
    transport_context=add_connection(worker_context,cc_store_handle_reply,(void*)gen_cc_store_ctx,request.addr,request.port,ZMQ_REQ);
    return transport_context;
 fail:
    return NULL;
}

int cc_store_handle_request(trike_transport_context_t *transport_context,void *data,int size){
    gen_cc_store_ctx_t *gen_cc_store_ctx=(gen_cc_store_ctx_t*)transport_context->user_data;
    TrikeCCMessage *trike_cc_msg=trike_ccmessage__unpack(NULL,size,(const uint8_t*)data);
    trike_cc_store_msg_t trike_cc_store_msg;
    int result=0;
    if(!trike_cc_msg) goto fail;
    compose_trike_cc_store_msg(trike_cc_msg,&trike_cc_store_msg);
    if(!gen_cc_store_ctx) goto fail;
    transport_context->user_data=gen_cc_store_ctx->user_data;
    result=gen_cc_store_ctx->handle_request(transport_context,&trike_cc_store_msg);
    transport_context->user_data=(void*)gen_cc_store_ctx;
    trike_ccmessage__free_unpacked(trike_cc_msg,NULL);
    return result;
 fail:
    return -1;
}

int cc_store_handle_reply(trike_transport_context_t *transport_context,void *data,int size){
    gen_cc_store_ctx_t *gen_cc_store_ctx=(gen_cc_store_ctx_t*)transport_context->user_data;
    TrikeCCMessage *trike_cc_msg=trike_ccmessage__unpack(NULL,size,(const uint8_t*)data);
    trike_cc_store_msg_t trike_cc_store_msg;
    int result;
    if(!trike_cc_msg) goto fail;
    compose_trike_cc_store_msg(trike_cc_msg,&trike_cc_store_msg);
    if(!gen_cc_store_ctx) goto fail;
    transport_context->user_data=gen_cc_store_ctx->user_data;
    result=gen_cc_store_ctx->handle_reply(transport_context,&trike_cc_store_msg);
    transport_context->user_data=(void*)gen_cc_store_ctx;
    trike_ccmessage__free_unpacked(trike_cc_msg,NULL);
    return result;
 fail:
    return -1;
}

static int compose_trike_cc_store_msg(TrikeCCMessage *trike_cc_msg,trike_cc_store_msg_t *trike_cc_store_msg){
    trike_cc_store_msg->dialog_id_data=trike_cc_msg->dialog_id.data;
    trike_cc_store_msg->dialog_id_size=trike_cc_msg->dialog_id.len;
    trike_cc_store_msg->request_id=trike_cc_msg->request_id;
    trike_cc_store_msg->data=trike_cc_msg->data.data;
    trike_cc_store_msg->size=trike_cc_msg->data.len;
    return 0;
}

int trike_send_cc_store_msg(trike_transport_context_t *transport_context,trike_cc_store_msg_t msg){
    TrikeCCMessage trike_cc_msg=TRIKE_CCMESSAGE__INIT;
    int result;
    trike_cc_msg.dialog_id.data=(uint8_t*)msg.dialog_id_data;
    trike_cc_msg.dialog_id.len=msg.dialog_id_size;
    trike_cc_msg.request_id=msg.request_id;
    trike_cc_msg.data.len=msg.size;
    trike_cc_msg.data.data=(uint8_t*)msg.data;
    int size=trike_ccmessage__get_packed_size(&trike_cc_msg);
    void *data=malloc(size);
    trike_ccmessage__pack(&trike_cc_msg,(uint8_t*)data);
    result=trike_send_msg(transport_context,data,size);
    free(data);
    return result;
}
