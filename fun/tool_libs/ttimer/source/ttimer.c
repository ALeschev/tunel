
#include <unistd.h>
#include <string.h>

#include "ttimer.h"

static void *ttimer_proc (void *data)
{
    ttimer_t *p_timer = (ttimer_t *)data;

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

            p_timer->handler (p_timer, p_timer->data);

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
    if (!timer)
        return -1;

    if (sec == 0 && msec == 0)
        return -2;

    memset (timer, 0, sizeof (ttimer_t));

    pthread_mutex_init(&timer->lock, NULL);
    timer->timeset.tv_sec= sec;
    timer->timeset.tv_usec = msec * 1000;
    timer->inited = 1;
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

    if (!timer || !timer->inited)
        return -1;

    pthread_mutex_lock (&timer->lock);

    timer->data = data;
    timer->handler = handler;
    timer->running = 1;

    res = pthread_create (&timer->id, NULL, ttimer_proc, (void *)timer);

    pthread_mutex_unlock (&timer->lock);

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

    if (!timer || !timer->inited || !timer->running)
        return -1;

    if (timer->running)
    {
        timer->running = 0;

        res = pthread_join (timer->id, NULL);
    }

    pthread_mutex_destroy (&timer->lock);

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
    if (!timer|| !timer->inited || !timer->running)
        return -1;

    if (sec == 0 && msec == 0)
        return -2;

    timer->timeset.tv_sec= sec;
    timer->timeset.tv_usec = msec * 1000;
}
