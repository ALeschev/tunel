#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ttimer.h"

typedef struct inttimer
{
    pthread_t id;
    pthread_mutex_t lock;
    struct timeval timeset;
    uint8_t inited;
    uint8_t running;
    void (*handler) (ttimer_t *, void *);
    uint8_t *data;
} inttimer_t;

#include <stdio.h>

static void *ttimer_proc (void *data)
{
    ttimer_t *timer = (ttimer_t *)data;
    inttimer_t *p_timer = (inttimer_t *)timer->timer;

    while (1)
    {
        if (p_timer->running)
        {
            pthread_mutex_lock (&p_timer->lock);

            /* try avoid empty switch context */
            if (p_timer->timeset.tv_sec)
            {
                sleep (p_timer->timeset.tv_sec);
                p_timer->timeset.tv_sec = 0;
            }

            /* try avoid empty switch context */
            if (p_timer->timeset.tv_usec)
            {
                usleep (p_timer->timeset.tv_usec);
                p_timer->timeset.tv_usec = 0;
            }

            p_timer->handler (timer, p_timer->data);

            if (!p_timer->timeset.tv_sec && !p_timer->timeset.tv_usec)
            {
                p_timer->running = 0;
                pthread_mutex_unlock (&p_timer->lock);

                return NULL;
            }

            pthread_mutex_unlock (&p_timer->lock);
        }
        else
        {
            break;
        }
    }

    return NULL;
}

/**
 * ttimer_init - update timer timing
 * @timer - pointer to clear timer
 *
 * init timer insides
 */
int ttimer_init (ttimer_t *timer, uint32_t sec, uint32_t msec)
{
    inttimer_t *p_timer;

    if (!timer)
        return -1;

    if (sec == 0 && msec == 0)
        return -2;

    timer->timer = (inttimer_t *) calloc (1, sizeof (inttimer_t));
    if (!timer->timer)
    {
        return -3;
    }

    p_timer = timer->timer;

    memset (p_timer, 0, sizeof (inttimer_t));

    pthread_mutex_init(&p_timer->lock, NULL);
    p_timer->timeset.tv_sec= sec;
    p_timer->timeset.tv_usec = msec * 1000;
    p_timer->inited = 1;
}

/**
 * ttimer_start - start timer processing
 * @timer - pointer to initialized timer
 *
 * run timer processing 
 */
int ttimer_start (ttimer_t *timer, ttimer_handler_t handler, uint8_t *data)
{
    int res = -1;
    inttimer_t *p_timer;

    if (!timer)
        return -1;

    p_timer = timer->timer;

    if (!p_timer->inited)
        return -2;

    pthread_mutex_lock (&p_timer->lock);

    p_timer->data = data;
    p_timer->handler = handler;
    p_timer->running = 1;

    res = pthread_create (&p_timer->id, NULL, ttimer_proc, (void *)timer);

    pthread_mutex_unlock (&p_timer->lock);

    return res;
}

/**
 * ttimer_stop - stop the timer processing
 * @timer - pointer to initialized and running timer
 *
 * stop the timer and wait for the completion of a custom processing
 */
int ttimer_stop (ttimer_t *timer)
{
    int res;
    inttimer_t *p_timer;

    if (!timer)
        return -1;

    p_timer = timer->timer;

    if (!p_timer->inited || !p_timer->running)
        return -2;

    if (p_timer->running)
    {
        p_timer->running = 0;

        res = pthread_join (p_timer->id, NULL);
    }

    pthread_mutex_destroy (&p_timer->lock);

    free (p_timer);

    return res;
}

/**
 * ttimer_update - update timer timing
 * @timer - pointer to initialized and running timer
 * @sec - updated seconds value
 * @msec - updated milliseconds value
 *
 * update timing on a timer
 *
 * NOTE: function must be called only from user timer hander
 */
int ttimer_update (ttimer_t *timer, uint32_t sec, uint32_t msec)
{
    inttimer_t *p_timer;

    if (!timer)
        return -1;

    p_timer = timer->timer;

    if (!p_timer->inited || !p_timer->running)
        return -2;

    if (sec == 0 && msec == 0)
        return -3;

    p_timer->timeset.tv_sec= sec;
    p_timer->timeset.tv_usec = msec * 1000;
}
