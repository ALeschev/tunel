#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <zmq.h>

#include "trike_zmq_socket_api.h"
#include "trike_zmq_msg_api.h"

// Internal function declarations
static int send_next_message(trike_zmq_socket_t *trike_socket);
static int receive_zmq_message_(trike_zmq_socket_t *trike_socket,int flags);
static int send_zmq_message_(trike_zmq_socket_t *trike_socket,void *data);
static int handle_message(trike_zmq_socket_t *trike_socket,void *data,int size);

/*-------------------------------------------------------------------------------------*/
/* API functions                                                                       */
/*-------------------------------------------------------------------------------------*/
trike_zmq_socket_t *trike_zmq_socket_new(void *socket){
    trike_zmq_socket_t *trike_socket=(trike_zmq_socket_t *)malloc(sizeof(trike_zmq_socket_t));
    trike_socket->socket=socket;
    trike_socket->status=FREE;
    if(!(trike_socket->socket_queue=trike_new_queue()))
        goto fail;
    return trike_socket;
 fail:
    if(trike_socket) free(trike_socket);
    return NULL;
}

int trike_zmq_socket_set_cb(trike_zmq_socket_t *trike_socket,trike_handle_msg_cb_t cb){
    if(!trike_socket) return -1;
    trike_socket->handle_msg=cb;
    return 0;
}

int trike_zmq_socket_set_type(trike_zmq_socket_t *trike_socket,int flag){
    if(!trike_socket) return -1;
    switch(flag){
    case ZMQ_REQ:
        trike_socket->type=REQUEST;
        break;
    case ZMQ_REP:
        trike_socket->type=REPLY;
        break;
    default:
        trike_socket->type=NOTIFY;
        break;
    }
    return 0;
}

int trike_zmq_socket_set_transport_context(trike_zmq_socket_t *trike_socket,trike_transport_context_t *transport_context){
    if(!transport_context) return -1;
    trike_socket->transport_context=transport_context;
    return 0;
}

trike_zmq_socket_t *trike_zmq_socket_get(trike_zmq_socket_t **trike_sockets,void *socket){
    if(!trike_sockets)return NULL;
    int i=0;
    while(trike_sockets[i]->socket!=socket)
        if(++i>MAX_ZMQ_ITEMS) return 0;
    return trike_sockets[i];
}

int receive_zmq_message(trike_zmq_socket_t *trike_socket,int flags){
    receive_zmq_message_(trike_socket,flags);
    trike_socket->status=FREE;
    send_next_message(trike_socket);
    return 0;
}

int send_zmq_message(trike_zmq_socket_t *trike_socket,trike_zmq_msg_t *trike_msg){
    void *buf;
    int result;
    if(!trike_msg) goto fail;
    if(!trike_socket) goto fail;
    result=trike_ser_zmq_msg(trike_msg,&buf);
    if(result) goto fail;
    result=trike_push_task(trike_socket->socket_queue,buf);
    if(result) goto fail;
    return send_next_message(trike_socket);
 fail:
    return -1;
}

/*-------------------------------------------------------------------------------------*/
/* Internal functions                                                                  */
/*-------------------------------------------------------------------------------------*/
static int send_next_message(trike_zmq_socket_t *trike_socket){
    if(!trike_socket) goto fail;
    void *msg;
    if(trike_socket->status==FREE){
        if(!(msg=trike_pop_task(trike_socket->socket_queue))){
            return 0;
        }
        send_zmq_message_(trike_socket,msg);
    };
    return 0;
 fail:
    return -1;
}

static int receive_zmq_message_(trike_zmq_socket_t *trike_socket,int flags){
    trike_transport_context_t *transport_context;
    zmq_msg_t zmq_msg;
    zmq_msg_t *zmq_msg_p=&zmq_msg;
    zmq_msg_init(zmq_msg_p);
    zmq_msg_recv(zmq_msg_p,trike_socket->socket,flags);
    handle_message(trike_socket,zmq_msg_data(zmq_msg_p),zmq_msg_size(zmq_msg_p));
    zmq_msg_close(zmq_msg_p);
    return 0;
}

static int send_zmq_message_(trike_zmq_socket_t *trike_socket,void *data){
    zmq_msg_t zmq_msg;
    zmq_msg_t *zmq_msg_p=&zmq_msg;
    trike_zmq_msg_t *trike_zmq_msg_p;
    if(trike_socket->type==REQUEST)
        trike_socket->status=BUSY;
    trike_dser_zmq_msg(&trike_zmq_msg_p,data);

    zmq_msg_init_size(zmq_msg_p,trike_zmq_msg_p->size);
    memcpy(zmq_msg_data(zmq_msg_p),trike_zmq_msg_p->data,trike_zmq_msg_p->size);
    zmq_msg_send(zmq_msg_p,trike_socket->socket,ZMQ_NOBLOCK);
    trike_zmq_msg_free(trike_zmq_msg_p);
    return 0;
}

static int handle_message(trike_zmq_socket_t *trike_socket,void *data,int size){
    if(!trike_socket->handle_msg) return 0;
    switch(trike_socket->type){
    case REQUEST:
        trike_socket->handle_msg(trike_socket->transport_context,data,size);
        break;
    case REPLY:
        trike_socket->handle_msg(trike_socket->transport_context,data,size);
        break;
    case NOTIFY:
        trike_socket->handle_msg(trike_socket->transport_context,data,size);
        break;
    default: break;
    }
    return 0;
}
