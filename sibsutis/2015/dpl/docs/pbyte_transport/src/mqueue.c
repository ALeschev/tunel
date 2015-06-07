
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#ifdef MQUEUE_USE_UT

#include <ut/ut_queues.h>
#include <ut/ut_port.h>
#include <ut/ut_thread.h>
#include <ut/ut_sema.h>

#endif

#include "mqueue.h"


extern int sem_timedwait (sem_t *__restrict __sem, __const struct timespec *__restrict __abstime);

// ----------------------------------------------------------------------------
#ifdef MQUEUE_USE_UT

#ifndef MEMDBG
int mqueue_init(mqueue_t *queue, int size)
#else /* MEMDBG */
int mqueue_init_ex(char* strFileName,unsigned int iLineNo, mqueue_t *queue, int size)
#endif /* MEMDBG */

#else /* MQUEUE_USE_UT */
int mqueue_init(mqueue_t *queue, int size)
#endif /* MQUEUE_USE_UT */
{
	assert(queue);
	memset(queue, 0, sizeof(mqueue_t));

#ifdef MQUEUE_USE_UT

#ifndef MEMDBG
	queue->elements = ut_malloc_ex(size * sizeof(void*));
#else /* MEMDBG */
	queue->elements = UT_AllocMemDbg(strFileName, iLineNo, (size * sizeof(void*)));
#endif /* MEMDBG */

#else /* MQUEUE_USE_UT */
	queue->elements = malloc(size * sizeof(void*));
#endif /* MQUEUE_USE_UT */

	if(queue->elements == NULL) return -1;
	memset(queue->elements, 0, size * sizeof(void*));
	queue->size = size;
	queue->first = queue->elements;
	queue->last = queue->elements;
	queue->count = 0;
	sem_init(&queue->sema, 0, 0);
	pthread_mutex_init(&queue->lock, NULL);
	queue->state = 1;
	return 0;
}

// ----------------------------------------------------------------------------

int mqueue_close(mqueue_t *queue)
{
	mqueue_flush(queue);
	pthread_mutex_lock(&queue->lock);
#ifdef MQUEUE_USE_UT
	ut_free_ex(queue->elements);
#else
	free(queue->elements);
#endif
	queue->state = 0;
	sem_destroy(&queue->sema);
	pthread_mutex_unlock(&queue->lock);
	pthread_mutex_destroy(&queue->lock);
	return 0;
}

// ----------------------------------------------------------------------------

void *mqueue_get(mqueue_t *queue)
{
	return mqueue_timedget(queue, 1);
}

// ----------------------------------------------------------------------------

void *mqueue_timedget(mqueue_t *queue, int to)
{
	struct timeval now;
	struct timespec timeout;
	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + to;
	timeout.tv_nsec = now.tv_usec * 1000;
	int s = sem_timedwait(&queue->sema, &timeout);
	pthread_mutex_lock(&queue->lock);

	if(s < 0 || queue->first == NULL)
	{
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}

	void *tmp = *queue->first;

	*queue->first = NULL;
	queue->first++;
	queue->count--;
	if(queue->first == queue->elements + queue->size) queue->first = queue->elements;
	pthread_mutex_unlock(&queue->lock);

	return tmp;
}

// ----------------------------------------------------------------------------

void *mqueue_timedget_msec(mqueue_t *queue, unsigned long to)
{
	struct timeval now;
	struct timespec timeout;
	gettimeofday(&now, NULL);
	// timeout.tv_sec = now.tv_sec + to/1000000;
	// timeout.tv_nsec = (now.tv_usec /*+ to%1000000*/) * 1000;

	timeout.tv_sec = now.tv_sec;
	timeout.tv_nsec = now.tv_usec * 1000;

	timeout.tv_sec += (to/1000);
	timeout.tv_nsec += ((to%1000) * 1000000);

	if (timeout.tv_nsec>=1000000000)
	{
	    timeout.tv_nsec -= 1000000000;
	    timeout.tv_sec += 1;
	}

	int s = sem_timedwait(&queue->sema, &timeout);
	pthread_mutex_lock(&queue->lock);

	if(s < 0 || queue->first == NULL)
	{
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}

	void *tmp = *queue->first;

	*queue->first = NULL;
	queue->first++;
	queue->count--;
	if(queue->first == queue->elements + queue->size) queue->first = queue->elements;
	pthread_mutex_unlock(&queue->lock);

	return tmp;
}
// ----------------------------------------------------------------------------

int mqueue_can_put(mqueue_t *queue)
{
	int ret = 0;

	if(!queue->state) return -1;

	/*pthread_mutex_lock(&queue->lock);*/

	if(queue->last+1 == queue->first)
	{
		/*que is full!*/
		ret = -1;
	}else{
		if(queue->count == queue->size)
		{
			ret = -1;
		}
	}

	/*pthread_mutex_unlock(&queue->lock);*/

	return ret;
}

// ----------------------------------------------------------------------------

int mqueue_put(mqueue_t *queue, void *element, int prio)
{
	int __attribute__ ((unused)) ret;

	if(!queue->state) return -1;

	if(queue->last+1 == queue->first)
	{
		/*que is full!*/
		return -2;
	}else{
		if(queue->count == queue->size)
		{
			return -3;
		}
	}

	pthread_mutex_lock(&queue->lock);

	if(prio)
	{
		if(queue->first == queue->elements) queue->first = queue->elements + queue->size;
		queue->first--;
		*queue->first = element;
		queue->count++;
	}else{

		*queue->last = element;
		queue->last++;
		queue->count++;
		if(queue->last == queue->elements + queue->size) queue->last = queue->elements;
	}

	pthread_mutex_unlock(&queue->lock);

	ret = sem_post(&queue->sema);

	return 0;
}

// ----------------------------------------------------------------------------

void *mqueue_tryget(mqueue_t *queue)
{
	int s = sem_trywait(&queue->sema);

	pthread_mutex_lock(&queue->lock);

	if(s < 0 || queue->first == NULL)
	{
		pthread_mutex_unlock(&queue->lock);
		return NULL;
	}

	void *tmp = *queue->first;

	*queue->first = NULL;
	queue->first++;
	queue->count--;
	if(queue->first == queue->elements + queue->size) queue->first = queue->elements;

	pthread_mutex_unlock(&queue->lock);

	return tmp;
}

// ----------------------------------------------------------------------------

void *mqueue_peek(mqueue_t *queue)
{
	return (*queue->first);
}

// ----------------------------------------------------------------------------

void mqueue_flush(mqueue_t *queue)
{
	void *tmp;
	do
	{
		tmp = mqueue_tryget(queue);
		if(tmp)
		{
		#ifdef MQUEUE_USE_UT
			ut_free_ex(tmp);
		#else
			free(tmp);
		#endif
		}
	} while(tmp);
}

// ----------------------------------------------------------------------------

int mqueue_get_count(mqueue_t *queue)
{
	int count = 0;

	pthread_mutex_lock(&queue->lock);
	count = queue->count;
	pthread_mutex_unlock(&queue->lock);

	return count;
}

// ----------------------------------------------------------------------------
