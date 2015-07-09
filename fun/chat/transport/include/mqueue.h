#ifndef MQUEUE_H
#define MQUEUE_H

#include <pthread.h>
#include <semaphore.h>

#define MQUEUESIZE 1024

typedef struct
{
	void **elements;
	void **first;
	void **last;
	int size;
	int count;
	int state;
	sem_t sema;
	pthread_mutex_t lock;
} mqueue_t;


#ifdef MQUEUE_USE_UT
#ifndef MEMDBG
extern int mqueue_init(mqueue_t *queue, int size);
#else /* MEMDBG */
extern int mqueue_init_ex(char* strFileName,unsigned int iLineNo,mqueue_t *queue, int size);
#define mqueue_init(queue, size) mqueue_init_ex(__FILE__,__LINE__,queue, size)
#endif /* MEMDBG */

#else /* MQUEUE_USE_UT */
extern int mqueue_init(mqueue_t *queue, int size);
#endif /* MQUEUE_USE_UT */

extern int mqueue_close(mqueue_t *queue);
extern void mqueue_flush(mqueue_t *queue);
extern void *mqueue_get(mqueue_t *queue);
extern void *mqueue_timedget(mqueue_t *queue, int to);
extern void *mqueue_timedget_msec(mqueue_t *queue, unsigned long to);
extern int mqueue_put(mqueue_t *queue, void *element, int prio);
extern void *mqueue_peek(mqueue_t *queue);
extern void *mqueue_tryget(mqueue_t *queue);
extern int mqueue_get_count(mqueue_t *queue);

#endif // MQUEUE_H
