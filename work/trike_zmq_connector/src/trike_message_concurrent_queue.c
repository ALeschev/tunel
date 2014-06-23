#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include "trike_message_concurrent_queue.h"

#ifndef MOCK_MESSAGE_QUEUE
#include "trike_message_queue_api.h"
#endif

/*-------------------------------------------------------------------------------------*/
/* Mock declarations                                                                   */
/*-------------------------------------------------------------------------------------*/
#ifdef MOCK_MESSAGE_QUEUE
static trike_queue_t *trike_new_queue();

static int trike_push_task(trike_queue_t *queue,void *msg);

static void *trike_pop_task(trike_queue_t *queue);

static int trike_poll_queue(trike_queue_t *queue);

static void trike_free_queue(trike_queue_t *queue);
#endif


/*-------------------------------------------------------------------------------------*/
/* API functions                                                                       */
/*-------------------------------------------------------------------------------------*/

trike_concurrent_queue_t *trike_new_queue_c(){
    trike_concurrent_queue_t *c_queue=(trike_concurrent_queue_t*)malloc(sizeof(trike_concurrent_queue_t));
    if(!c_queue) return NULL;
    trike_queue_t *queue=trike_new_queue();
    pthread_mutexattr_t attrs;
    int result;
    c_queue->queue=queue;
    if(!queue){trike_free_queue_c(c_queue);return NULL;}
    pthread_mutexattr_init(&attrs);
    result=pthread_mutex_init(&c_queue->lock,&attrs);
    if(result<0){
        trike_free_queue_c(c_queue);
        return NULL;
    }
    return c_queue;
}

int trike_push_task_c(trike_concurrent_queue_t *c_queue,void *msg){
    int result;
    if(!c_queue) goto fail;
    pthread_mutex_lock(&c_queue->lock);
    result=trike_push_task(c_queue->queue,msg);
    pthread_mutex_unlock(&c_queue->lock);
    return result;
 fail:
    return -1;
}

void *trike_pop_task_c(trike_concurrent_queue_t *c_queue){
    void *result;
    if(!c_queue) return NULL;
    pthread_mutex_lock(&c_queue->lock);
    result=trike_pop_task(c_queue->queue);
    pthread_mutex_unlock(&c_queue->lock);
    return result;
}

int trike_poll_queue_c(trike_concurrent_queue_t *c_queue){
    int result;
    if(!c_queue) return -1;
    pthread_mutex_lock(&c_queue->lock);
    result=trike_poll_queue(c_queue->queue);
    pthread_mutex_unlock(&c_queue->lock);
    return result;
}

void trike_free_queue_c(trike_concurrent_queue_t *c_queue){
    if(!c_queue) return;
    pthread_mutex_lock(&c_queue->lock);
    if(c_queue->queue) trike_free_queue(c_queue->queue);
    pthread_mutex_unlock(&c_queue->lock);
}


/*-------------------------------------------------------------------------------------*/
/* Mock functions                                                                      */
/*-------------------------------------------------------------------------------------*/
#ifdef MOCK_MESSAGE_QUEUE
static trike_queue_t *trike_new_queue(){
    return (trike_queue_t*)malloc(sizeof(trike_queue_t));
}

static int trike_push_task(trike_queue_t *queue,void *msg){
    return 0;
}

static void *trike_pop_task(trike_queue_t *queue){
    return NULL;
}

static int trike_poll_queue(trike_queue_t *queue){
    return 0;
}

static void trike_free_queue(trike_queue_t *queue){free(queue);}
#endif
