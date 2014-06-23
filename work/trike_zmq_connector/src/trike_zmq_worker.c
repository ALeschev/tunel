#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <zmq.h>

#include "trike_zmq_socket_api.h"
#include "trike_transport_context_api.h"
#include "trike_msg_api.h"
#include "trike_msg_handler.h"
#include "trike_zmq_worker.h"

//Internal delclaration functions
static void *thread_worker_loop(void *args);
static trike_worker_context_t *alloc_worker_context();

static int receive_messages(trike_worker_context_t *worker_context,int count);
static int send_messages(trike_worker_context_t *worker_context,int count);

/*-------------------------------------------------------------------------------------*/
/* API functions                                                                       */
/*-------------------------------------------------------------------------------------*/
trike_worker_context_t *start_worker(init_cb_t init_cb){
    pthread_attr_t attr;
    pthread_mutexattr_t mutex_attr;
    trike_worker_context_t *worker_context;
    int result=0;
    worker_context=alloc_worker_context();
    worker_context->status=ACTIVE; // Set active status for worker
    worker_context->zmq_context=zmq_ctx_new(); // Allocate ZeroMQ context
    worker_context->c_queue=trike_new_queue_c();
    worker_context->init_cb=init_cb;
    pthread_mutexattr_init(&mutex_attr);
    result=pthread_mutex_init(&worker_context->service_lock,&mutex_attr); // Init service lock
    if(result<0) return NULL;
    result=pthread_attr_init(&attr);
    if(result<0) return NULL;
    result=pthread_create(&worker_context->pthread,&attr,&thread_worker_loop,worker_context); // Start new worker
    if(result<0) return NULL;
    return worker_context;
}

int stop_worker(trike_worker_context_t *worker_context){
    worker_context->status=INACTIVE;
    return pthread_join(worker_context->pthread,NULL);
}

trike_transport_context_t *add_connection(trike_worker_context_t *worker_context,trike_handle_msg_cb_t cb,void *user_data,const char *addr,int port,int type){
    void *socket=zmq_socket(worker_context->zmq_context,type);
    char url[MAX_URL_SIZE]={0};
    trike_transport_context_t *trike_transport=NULL;
    trike_zmq_socket_t *trike_socket=trike_zmq_socket_new(socket);
    snprintf(url,MAX_URL_SIZE,"%s:%i",addr,port); //URL compose
    if(zmq_connect(socket,url)) goto fail; //Connect to URL
    //Initialize trike_transport_context
    trike_transport=trike_transport_new();
    trike_transport->socket=socket;
    trike_transport->user_data=user_data;
    trike_transport->c_queue=worker_context->c_queue;
    //Add a connection socket to socket list
    pthread_mutex_lock(&worker_context->service_lock);
    worker_context->trike_sockets[worker_context->nitems]=trike_socket;
    worker_context->zmq_items[worker_context->nitems].socket=socket;
    worker_context->zmq_items[worker_context->nitems].fd=0;
    worker_context->zmq_items[worker_context->nitems].events=ZMQ_POLLIN;
    worker_context->zmq_items[worker_context->nitems].revents=0;
    worker_context->nitems++;
    trike_zmq_socket_set_cb(trike_socket,cb);
    trike_zmq_socket_set_type(trike_socket,type);
    trike_zmq_socket_set_transport_context(trike_socket,trike_transport);
    pthread_mutex_unlock(&worker_context->service_lock);
    return trike_transport;
 fail:
    zmq_close(socket);
    return NULL;
}

trike_transport_context_t *add_listener(trike_worker_context_t *worker_context,trike_handle_msg_cb_t cb,void *user_data,const char *addr,int port,int type){
    void *socket=zmq_socket(worker_context->zmq_context,type);
    char url[MAX_URL_SIZE]={0};
    trike_transport_context_t *trike_transport=NULL;
    trike_zmq_socket_t *trike_socket=trike_zmq_socket_new(socket);
    snprintf(url,MAX_URL_SIZE,"%s:%i",addr,port); //URL compose
    if(zmq_bind(socket,url)) goto fail; //Bind to URL
    //Initialize trike_transport_context
    trike_transport=trike_transport_new();
    trike_transport->socket=socket;
    trike_transport->user_data=user_data;
    trike_transport->c_queue=worker_context->c_queue;
    trike_transport->user_data=user_data;
    // Add a listener socket to socket_list
    pthread_mutex_lock(&worker_context->service_lock);
    worker_context->trike_sockets[worker_context->nitems]=trike_socket;
    worker_context->zmq_items[worker_context->nitems].socket=socket;
    worker_context->zmq_items[worker_context->nitems].fd=0;
    worker_context->zmq_items[worker_context->nitems].events=ZMQ_POLLIN;
    worker_context->zmq_items[worker_context->nitems].revents=0;
    worker_context->nitems++;
    trike_zmq_socket_set_cb(trike_socket,cb);
    trike_zmq_socket_set_type(trike_socket,type);
    trike_zmq_socket_set_transport_context(trike_socket,trike_transport);
    pthread_mutex_unlock(&worker_context->service_lock);
    return trike_transport;
 fail:
    zmq_close(socket);
    return NULL;
}

int set_option(trike_transport_context_t *transport_context,int name,const void *key,size_t value){
    return zmq_setsockopt(transport_context->socket,name,key,value);
}

/*-------------------------------------------------------------------------------------*/
/* Internal functions                                                                  */
/*-------------------------------------------------------------------------------------*/
static void *thread_worker_loop(void *args){
    trike_worker_context_t *worker_context=(trike_worker_context_t *)args;
    int count;
    if(!worker_context) return NULL;
    if(worker_context->init_cb)
	worker_context->init_cb();
    while(worker_context->status==ACTIVE){
        count=zmq_poll(worker_context->zmq_items,worker_context->nitems,POLL_TIMEOUT);
        if(count>0){
            receive_messages(worker_context,count);
        }
        count=trike_poll_queue_c(worker_context->c_queue);
        if(count>0){
            send_messages(worker_context,count);
        }
    }
    return NULL;
}

static int receive_messages(trike_worker_context_t *worker_context,int count){
    int i=0,j=0;
    trike_zmq_socket_t *trike_socket=NULL;
    for(i=0;i<worker_context->nitems;i++){
        if(worker_context->zmq_items[i].revents==ZMQ_POLLIN){
            trike_socket=trike_zmq_socket_get(worker_context->trike_sockets,worker_context->zmq_items[i].socket);
            receive_zmq_message(trike_socket,ZMQ_DONTWAIT);
            if(++j>=count) break; // Leave receive loop when all sockets checked.
        }
    }
    return 0;
}

static int send_messages(trike_worker_context_t *worker_context,int count){
    trike_msg_t *trike_msg=NULL;
    trike_zmq_socket_t *trike_socket=NULL;
    void *msg=NULL;
    int i=0,j=0;
    if(!worker_context) goto fail;
    for(i=0;i<worker_context->nitems;i++){
        if((msg=trike_pop_task_c(worker_context->c_queue))){
            trike_msg=trike_msg_dser(msg);
            trike_socket=trike_zmq_socket_get(worker_context->trike_sockets,trike_msg->transport_context->socket);
            send_zmq_message(trike_socket,trike_msg->trike_zmq_msg);
            trike_msg_free(trike_msg);
        }
        if(++j==count) break;
    }
    return 0;
 fail:
    return -1;
}

static trike_worker_context_t *alloc_worker_context(){
    return (trike_worker_context_t *)malloc(sizeof(trike_worker_context_t));
}
