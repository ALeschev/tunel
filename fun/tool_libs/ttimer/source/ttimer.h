#ifndef __TTIMER_H__
#define __TTIMER_H__

#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct ttimer
{
    void *timer;
} ttimer_t;

typedef void (*ttimer_handler_t) (ttimer_t *, void *);

/**
 * ttimer_init - update timer timing
 * @timer - pointer to clear timer
 *
 * init timer insides
 *
 * %-1: NULL timer pointer
 * %-2: bad timing value
 * %-3: failed to allocate timer
 */
int ttimer_init (ttimer_t *timer, uint32_t sec, uint32_t msec);

/**
 * ttimer_start - start timer processing
 * @timer - pointer to initialized timer
 *
 * run timer processing
 *
 * %-1: NULL timer pointer
 * %-2: timer not inited
 * %errno: failed to start timer
 */
int ttimer_start (ttimer_t *timer, ttimer_handler_t handler, uint8_t *data);

/**
 * ttimer_stop - stop the timer processing
 * @timer - pointer to initialized and running timer
 *
 * stop the timer and wait for the completion of a custom processing
 *
 * %-1: NULL timer pointer
 * %-2: timer not inited or not running
 * %errno: failed to joint timer
 */
int ttimer_stop (ttimer_t *timer);

/**
 * ttimer_update - update timer timing
 * @timer - pointer to initialized and running timer
 * @sec - updated seconds value
 * @msec - updated milliseconds value
 *
 * update timing on a timer
 * NOTE: function must be called only from user timer hander
 *
 * %-1: NULL timer pointer
 * %-2: timer not inited or not running
 * %-3: bad timing value
 */
int ttimer_update (ttimer_t *timer, uint32_t sec, uint32_t msec);

#endif /* __TTIMER_H__ */
