#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "trike_message_queue.h"
#include "trike_message_queue_api.h"

//Internal function declaration
static trike_task_t *trike_new_task();
static void trike_free_task(trike_task_t *task);
static void trike_init_queue(trike_queue_t *queue,trike_task_t *task);
static trike_task_t *trike_find_tail(trike_queue_t *queue);
static int trike_add_to_head(trike_queue_t *queue,trike_task_t *new_task);
static int trike_set_queue_head(trike_queue_t *queue,trike_task_t *task);
static int trike_set_queue_tail(trike_queue_t *queue,trike_task_t *task);

/*-------------------------------------------------------------------------------------*/
/* API functions                                                                       */
/*-------------------------------------------------------------------------------------*/
trike_queue_t *trike_new_queue(){
    trike_queue_t *queue=(trike_queue_t*)malloc(sizeof(trike_queue_t));
    if(!queue) return NULL;
    queue->task_count=0;
    queue->head=NULL;
    queue->tail=NULL;
    return queue;
}

int trike_push_task(trike_queue_t *queue,void *msg){
    if(!queue) return -1;
    trike_task_t *new_task=trike_new_task();
    if(!new_task) return -1;
    queue->task_count++;
    new_task->msg=msg;
    if(!queue->head){
        trike_init_queue(queue,new_task);
        return 0;
    }
    trike_add_to_head(queue,new_task);
    trike_set_queue_head(queue,new_task);
    return 0;
}

void *trike_pop_task(trike_queue_t *queue){
    if(!queue) return NULL;
    trike_task_t *task=trike_find_tail(queue);
    if(!task) return NULL;
    void *msg=task->msg;
    trike_task_t *prev=task->prev;
    if(prev)
        prev->next=NULL;
    else
        trike_set_queue_head(queue,prev);
    trike_set_queue_tail(queue,prev);
    trike_free_task(task);
    queue->task_count--;
    return msg;
}

int trike_poll_queue(trike_queue_t *queue){
    if(!queue) return -1;
    return queue->task_count;
}

void trike_free_queue(trike_queue_t *queue){
    if(!queue) return;
    free(queue);
}

/*-------------------------------------------------------------------------------------*/
/* Internal functions                                                                  */
/*-------------------------------------------------------------------------------------*/
static trike_task_t *trike_new_task(){
    trike_task_t *task=(trike_task_t*)malloc(sizeof(trike_task_t));
    task->next=NULL;
    task->prev=NULL;
    return task;
}

static void trike_free_task(trike_task_t *task){
    free(task);
}

static void trike_init_queue(trike_queue_t *queue,trike_task_t *task){
    trike_set_queue_head(queue,task);
    trike_set_queue_tail(queue,task);
}

static trike_task_t *trike_find_tail(trike_queue_t *queue){
    return queue->tail;
}

static int trike_add_to_head(trike_queue_t *queue,trike_task_t *new_task){
    trike_task_t *head=queue->head;
    new_task->next=head;
    head->prev=new_task;
    new_task->prev=NULL;
    return 0;
};

static int trike_set_queue_tail(trike_queue_t *queue,trike_task_t *task){
    queue->tail=task;
    return 0;
}


static int trike_set_queue_head(trike_queue_t *queue,trike_task_t *task){
    queue->head=task;
    return 0;
}
