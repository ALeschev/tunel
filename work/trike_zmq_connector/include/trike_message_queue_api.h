#ifndef _TRIKE_MESSAGE_QUEUE_API
#define _TRIKE_MESSAGE_QUEUE_API

#include "trike_message_queue.h"
//Init queue functions
trike_queue_t *trike_new_queue();

//Push and pop functions
int trike_push_task(trike_queue_t* queue,void *msg);
void *trike_pop_task(trike_queue_t* queue);
int trike_poll_queue(trike_queue_t *queue);

//Free functions
void trike_free_queue(trike_queue_t *queue);


#endif
