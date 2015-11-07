#include <stdio.h>
#include <unistd.h>

#include "ttimer.h"

#define MAX_TIMER_COUNT 1
#define ACTIVE_TIMER_COUNT 1

struct hander_data
{
    int timer_num;
    int tick_count;
};

void castom_hander (ttimer_t *timer, void *data)
{
    struct hander_data *p_data = (struct hander_data *)data;

    struct timeval tp;
    struct tm *pt;
    time_t t;

    gettimeofday(&tp, NULL);
    t = (time_t)tp.tv_sec;
    pt = localtime(&t);

    printf ("%02d:%02d:%02d.%06lu\ttimer %d tick %d\n",
            pt->tm_hour, pt->tm_min, pt->tm_sec, tp.tv_usec,
            p_data->timer_num, p_data->tick_count++);

    ttimer_update (timer, 0, 100);
}

int main (void)
{
    ttimer_t timer[MAX_TIMER_COUNT];
    struct hander_data h_data[MAX_TIMER_COUNT];
    int i;

    for (i = 0; i < ACTIVE_TIMER_COUNT; i++)
    {
        ttimer_init (&timer[i], 0, 100);
    }

    for (i = 0; i < ACTIVE_TIMER_COUNT; i++)
    {
        h_data[i].timer_num = i;
        h_data[i].tick_count = 0;
        ttimer_start (&timer[i], castom_hander, (uint8_t *)&h_data[i]);
        printf ("timer %d started\n", i);
    }

    sleep (1);

    for (i = 0; i < ACTIVE_TIMER_COUNT; i++)
    {
        ttimer_stop (&timer[i]);
        printf ("timer %d was stopped\n", i);
    }

    return 0;
}

