#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>

#include "icmp_core.h"

typedef struct {
	icmp_addr_in_t addr_in;
	int send_period_ms;
	int send_count;
	icmp_result_t request_result;

	int init_result;
} icmp_settings_t;

typedef struct {
	icmp_addr_t *source;
	icmp_addr_t *dest;
} icmp_pull_t;

/*----------------------------------------------------------------------------*/

pthread_mutex_t thread_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/*----------------------------------------------------------------------------*/

void *icmp_send_thread(void *arg);

/*----------------------------------------------------------------------------*/

int icmp_send_packet (icmp_addr_in_t *addr_in, icmp_result_t request_result)
{
	icmp_send_periodical (addr_in, T100MS, 1, request_result);
}

/*----------------------------------------------------------------------------*/

static void icmp_proc_pull (icmp_settings_t *icmp_settings,
                            int dest_start, int dest_end)
{
	static int pull_num = 0;
	int i;

	printf ("pull %d:\n", pull_num++);

	for (i = dest_start; i < dest_end; i++)
	{
		printf ("\t%s\n", icmp_settings->addr_in.dest[i].addr);
	}
}

/*----------------------------------------------------------------------------*/

static void icmp_start_sending (icmp_settings_t *icmp_settings)
{
	int i, dest_end = 0;
	uint8_t pulls_count = 0;
	icmp_addr_in_t *addr_in;

	addr_in = &icmp_settings->addr_in;

	// for (i = 0; i < addr_in->dest_count; i++)
	// {
	// 	printf ("%d: %s\n", i, icmp_settings->addr_in.dest[i].addr);
	// }

	pulls_count = (uint8_t)(addr_in->dest_count / ADDR_BY_PULL) + 1;

	for (i = 0; i < addr_in->dest_count; i += ADDR_BY_PULL)
	{
		if (i + ADDR_BY_PULL >= addr_in->dest_count)
			break;

		icmp_proc_pull (icmp_settings, i, i + ADDR_BY_PULL);
	}

	/* processing residue */
	icmp_proc_pull (icmp_settings, i, addr_in->dest_count);
}

/*----------------------------------------------------------------------------*/

int icmp_send_periodical (icmp_addr_in_t *addr_in, int send_period_ms,
                          int send_count, icmp_result_t request_result)
{
	int rc;
	pthread_t icmp_main_thread;
	icmp_settings_t icmp_settings;

	// if (!addr_in)
	// 	return -1;

	// if (!addr_in->source_address)
	// 	return -2;

	// if (!addr_in->dest_addreses)
	// 	return -3;

	// if (addr_in->dest_addr_count <= 0)
	// 	return -4;

	// memset (&icmp_settings, 0, sizeof (icmp_settings));

	icmp_settings.addr_in.source = addr_in->source;
	icmp_settings.addr_in.dest = addr_in->dest;
	icmp_settings.addr_in.dest_count = addr_in->dest_count;
	icmp_settings.send_period_ms = send_period_ms;
	icmp_settings.send_count = send_count;
	icmp_settings.request_result = request_result;
	icmp_settings.init_result = 0;

	rc = pthread_create(&icmp_main_thread,
	                    NULL,
	                    &icmp_send_thread,
	                    (void *)&icmp_settings);
	if(rc < 0)
	{
		printf ("Failed to start. "
		        "Can't create recv/send thread: %s",
		        strerror(errno));
	}

	/* 
	 * insert some delay for mutex locking
	 * from icmp_send_thread for initialization
	 */
	usleep(1);

	pthread_mutex_lock(&thread_init_mutex);

	if (rc < 0)
		icmp_settings.init_result = rc;

	printf ("icmp_settings.init_result: %d\n", icmp_settings.init_result);

	pthread_mutex_unlock(&thread_init_mutex);

	return icmp_settings.init_result;
}

/*----------------------------------------------------------------------------*/

void *icmp_send_thread(void *arg)
{
	pthread_mutex_lock(&thread_init_mutex);

	int i;
	int send_count = 0;
	icmp_addr_in_t *addr_in;
	icmp_settings_t *icmp_st_tmp;
	icmp_settings_t icmp_st_static;

	if (!arg)
	{
		icmp_st_tmp->init_result = -1;
		pthread_mutex_unlock(&thread_init_mutex);
		return;
	}

	icmp_st_tmp = (icmp_settings_t *)arg;

	memset (&icmp_st_static, 0, sizeof (icmp_st_static));

	addr_in = &icmp_st_static.addr_in;

	/*
	 * START COPYING FOR SAVING ICMP SETTINGS ON THREAD LEVEL
	 */

	/* saving "source address" */

	addr_in->source = (icmp_addr_t *) calloc (1, sizeof (icmp_addr_t));
	if (!addr_in->source)
	{
		icmp_st_tmp->init_result = 1;
		pthread_mutex_unlock(&thread_init_mutex);
		return;
	}

	memcpy(addr_in->source, icmp_st_tmp->addr_in.source, sizeof (icmp_addr_t));

	/* saving "dest addresses" */

	addr_in->dest_count = icmp_st_tmp->addr_in.dest_count;

	addr_in->dest = (icmp_addr_t *) calloc (1, sizeof (icmp_addr_t) * addr_in->dest_count);
	if (!addr_in->dest)
	{
		icmp_st_tmp->init_result = 2;
		pthread_mutex_unlock(&thread_init_mutex);
		return;
	}

	for (i = 0; i < addr_in->dest_count; i++)
	{
		memcpy(&addr_in->dest[i],
		       &icmp_st_tmp->addr_in.dest[i],
		       sizeof (icmp_addr_t));
	}

	/* saving "send parameters" */

	icmp_st_static.send_period_ms = icmp_st_tmp->send_period_ms;
	icmp_st_static.send_count = icmp_st_tmp->send_count;
	icmp_st_static.request_result = icmp_st_tmp->request_result;

	/* init success */

	icmp_st_tmp->init_result = 0;
	pthread_mutex_unlock(&thread_init_mutex);

	if (icmp_st_static.send_period_ms < 0)
		icmp_st_static.send_period_ms = 0;

	while (send_count++ != icmp_st_static.send_count)
	{
		icmp_start_sending(&icmp_st_static);

		usleep (icmp_st_static.send_period_ms);
	}

ext:
	if (addr_in->source)
		free (addr_in->source);

	if (addr_in->dest)
		free (addr_in->dest);

	pthread_mutex_unlock(&thread_init_mutex);

	printf ("%s - done\n", __func__);
}

void icmp_send_packet_thread (void *arg)
{

}
