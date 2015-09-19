#ifndef __TTIMER_H__
#define __TTIMER_H__

#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>

typedef struct ttimer
{
    pthread_t id;
    pthread_mutex_t lock;
    struct timeval timeset;
    uint8_t inited;
    uint8_t running;
    void (*handler) (struct ttimer *, void *);
    uint8_t *data;
} ttimer_t;

typedef void (*ttimer_handler_t) (ttimer_t *, void *);

/**
 * ttimer_init - update timer timing
 * @timer - pointer to clear timer
 *
 * init timer insides
 */
int ttimer_init (ttimer_t *timer, uint32_t sec, uint32_t msec);

/**
 * ttimer_start - start timer processing
 * @timer - pointer to initialized timer
 *
 * run timer processing 
 */
int ttimer_start (ttimer_t *timer, ttimer_handler_t handler, uint8_t *data);

/**
 * ttimer_stop - stop the timer processing
 * @timer - pointer to initialized and running timer
 *
 * stop the timer and wait for the completion of a custom processing
 */
int ttimer_stop (ttimer_t *timer);

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
int ttimer_update (ttimer_t *timer, uint32_t sec, uint32_t msec);

#endif /* __TTIMER_H__ */
