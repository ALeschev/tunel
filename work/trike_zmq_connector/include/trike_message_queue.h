#ifndef _TRIKE_MESSAGE_QUEUE_H
#define _TRIKE_MESSAGE_QUEUE_H

typedef struct trike_task {
    struct trike_task *prev;
    struct trike_task *next;
    void *msg;
} trike_task_t;

typedef struct trike_queue{
    trike_task_t *head;
    trike_task_t *tail;
    int task_count;
} trike_queue_t;

#endif
