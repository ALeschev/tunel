#ifndef _TRIKE_MESSAGE_CONCURRENT_QUEUE_H
#define _TRIKE_MESSAGE_CONCURRENT_QUEUE_H

#include <pthread.h>
#include "trike_message_queue.h"

typedef struct trike_concurrent_queue{
    trike_queue_t *queue;
    pthread_mutex_t lock;
} trike_concurrent_queue_t;

//API for concurrent QUEUE
//Create a new queue
trike_concurrent_queue_t *trike_new_queue_c();
//Push message
int trike_push_task_c(trike_concurrent_queue_t *c_queue,void *msg);
//Pop message
void *trike_pop_task_c(trike_concurrent_queue_t *c_queue);
//Poll queue
int trike_poll_queue_c(trike_concurrent_queue_t *c_queue);
//Free queue
void trike_free_queue_c(trike_concurrent_queue_t *c_queue);

#endif
