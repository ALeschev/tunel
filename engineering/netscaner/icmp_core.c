#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>

#include <pthread.h>
#include <errno.h>

#include "icmp_core.h"

#define LOGING_DEBUG
#define INIT_SYNCH_DEBUG

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
	int dest_count;

	icmp_result_t request_result;

	pid_t pid;
} icmp_pull_t;

typedef struct {
	pthread_t thread_id;
	pid_t pid;
} icmp_thread_t;

/*----------------------------------------------------------------------------*/

pthread_mutex_t thread_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/*----------------------------------------------------------------------------*/

static int  icmp_sock_init (int *sockfd);
static int  icmp_sock_deinit (int *sockfd);

static int  icmp_packet_init (uint8_t **icmp_packet, int *packet_size);
static void icmp_packet_deinit (uint8_t *icmp_packet);

static int  icmp_send_packet (uint8_t *icmp_packet, int packet_size,
                              char *source, char *dest);

void *icmp_control_thread(void *arg);
void *icmp_send_packet_thread (void *arg);

/*----------------------------------------------------------------------------*/

#ifdef LOGING_DEBUG
#define icmp_log(format, ...) icmp_log_ext(__func__, format, ##__VA_ARGS__)
static void icmp_log_ext (const char *func, char *format, ...)
#else
static void icmp_log (char *format, ...)
#endif
{
	va_list	ap;
	int length, size;
	char str[4096] = {0}, *p = "";
	char timestamp[100] = {0};
	struct timeval tp;
	struct tm *pt;
	time_t t;

	gettimeofday(&tp, NULL);
	t = (time_t)tp.tv_sec;
	pt = localtime(&t);
	sprintf(timestamp, " %02d:%02d:%02d.%06lu",
	        pt->tm_hour, pt->tm_min, pt->tm_sec, tp.tv_usec);

	// switch(level)
	// {
	// 	case PBYTE_CRIT: p = "[CRIT] "; break;
	// 	case PBYTE_ERR:  p = "[ERR ] "; break;
	// 	case PBYTE_WARN: p = "[WARN] "; break;
	// 	case PBYTE_INFO: p = "[INFO] "; break;
	// 	case PBYTE_CUT:  p = "[CUT ] "; break;
	// 	case PBYTE_FULL: p = "[FULL] "; break;

	// 	default:         p = ""; break;
	// }

	sprintf(str, "%s icmp core [%ld]: "
#ifdef LOGING_DEBUG
	        "%s: "
#endif
	        "%s", timestamp, syscall(SYS_gettid),
#ifdef LOGING_DEBUG
	        func,
#endif
	        p);

	length = strlen(str);
	size = sizeof(str) - length - 10;
	va_start(ap, format);
	length = vsnprintf(&str[length], size, format, ap);
	va_end(ap);

	size = strlen(str);
	while(size && (str[size-1] == '\n')) size--;
	//str[size++] = '\r';
	str[size++] = '\n';
	str[size++] = 0;

	printf ("%s", str);
}

/*----------------------------------------------------------------------------*/

#ifdef INIT_SYNCH_DEBUG
#define icmp_wait_thread_initing() icmp_wait_thread_initing_ext(__func__)
static inline void icmp_wait_thread_initing_ext (const char *func)
#else
static inline void icmp_wait_thread_initing (void)
#endif
{
	pthread_yield();

	/* 
	 * insert some delay for mutex locking
	 * from thread for initialization
	 */
	usleep(10);

	/* 
	 * waiting when thread initialization was be finished
	 */
#ifdef INIT_SYNCH_DEBUG
	icmp_log ("Wait thread from: %s  - LOCK", func);
#endif

	pthread_mutex_lock(&thread_init_mutex);

#ifdef INIT_SYNCH_DEBUG
	icmp_log ("Wait thread from: %s  - UNLOCK", func);
#endif

	pthread_mutex_unlock(&thread_init_mutex);
}

/*----------------------------------------------------------------------------*/

int icmp_send (icmp_addr_in_t *addr_in, icmp_result_t request_result)
{
	return icmp_send_periodical (addr_in, 0, 1, request_result);
}

/*----------------------------------------------------------------------------*/

int icmp_send_periodical (icmp_addr_in_t *addr_in, int send_period_ms,
                          int send_count, icmp_result_t request_result)
{
	int rc;
	pthread_t icmp_main_thread;
	icmp_settings_t icmp_settings;

	if (!addr_in)
		return -1;

	if (!addr_in->source)
		return -2;

	if (!addr_in->dest)
		return -3;

	if (addr_in->dest_count <= 0)
		return -4;

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
	                    &icmp_control_thread,
	                    (void *)&icmp_settings);
	if(rc < 0)
	{
		icmp_log ("Failed to start. "
		        "Can't create recv/send thread: %s",
		        strerror(errno));
	}

	icmp_wait_thread_initing();

	if (rc < 0)
		icmp_settings.init_result = rc;

	icmp_log ("icmp_settings.init_result: %d\n", icmp_settings.init_result);

	return icmp_settings.init_result;
}

/*----------------------------------------------------------------------------*/

static inline void icmp_proc_pull (icmp_pull_t *icmp_pull,
                                   icmp_thread_t *icmp_send_thread)
{
	int rc;

	rc = pthread_create(&icmp_send_thread->thread_id,
	                    NULL,
	                    &icmp_send_packet_thread,
	                    (void *)icmp_pull);
	if(rc < 0)
	{
		icmp_log ("Failed to start. "
		          "Can't create recv/send thread: %s",
		          strerror(errno));
	}

	icmp_wait_thread_initing();

	icmp_send_thread->pid = icmp_pull->pid;
}

/*----------------------------------------------------------------------------*/

static void icmp_start_sending (icmp_settings_t *icmp_settings)
{
	int i, rc;
	uint8_t pulls_count = 0, cur_pull = -1;
	icmp_thread_t *icmp_send_threads;
	icmp_addr_in_t *addr_in;
	icmp_pull_t icmp_pull;

	addr_in = &icmp_settings->addr_in;

	for (i = 0; i < addr_in->dest_count; i++)
	{
		icmp_log ("%d: %s\n", i, icmp_settings->addr_in.dest[i].addr);
	}

	icmp_pull.source = addr_in->source;

	pulls_count = (uint8_t)(addr_in->dest_count / ADDR_BY_PULL) + 1;

	icmp_send_threads = (icmp_thread_t *) calloc (1, sizeof (icmp_thread_t) * pulls_count);
	if (!icmp_send_threads)
	{
		icmp_log ("!!!!!!!!!!!!");
		return;
	}

	icmp_pull.request_result = icmp_settings->request_result;

	for (i = 0; i < addr_in->dest_count; i += ADDR_BY_PULL)
	{
		cur_pull = (uint8_t)(i / ADDR_BY_PULL);

		if (i + ADDR_BY_PULL >= addr_in->dest_count)
			break;

		icmp_pull.dest = &addr_in->dest[i];
		icmp_pull.dest_count = ADDR_BY_PULL;

		icmp_proc_pull (&icmp_pull, &icmp_send_threads[cur_pull]);
	}

	icmp_pull.dest = &addr_in->dest[i];
	icmp_pull.dest_count = addr_in->dest_count - i;

	/* processing residue */
	icmp_proc_pull (&icmp_pull, &icmp_send_threads[cur_pull]);

	for (i = 0; i < pulls_count; i++)
	{
		icmp_log ("Wait thread %ld pull %d", icmp_send_threads[i].pid, i);
		rc = pthread_join (icmp_send_threads[i].thread_id, NULL);
		if (rc)
		{
			switch (rc)
			{
				case EDEADLK:
					icmp_log ("A deadlock was detected");
				break;
				case EINVAL: 
					icmp_log ("Another thread is already waiting to join with this thread");
				break;
				case ESRCH:  
					icmp_log ("No thread with the ID thread could be found");
				break;
				default:
					icmp_log ("Unknown err: %d", rc);
			}
		} else {
			icmp_log ("Thread %ld pull %d - exit success", icmp_send_threads[i].pid, i);
		}
	}

	free (icmp_send_threads);

	icmp_log ("%s - exit", __func__);
}

/*----------------------------------------------------------------------------*/

void *icmp_control_thread(void *arg)
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
		free (addr_in->source);
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

	icmp_log ("%s - inited\n", __func__);

	if (icmp_st_static.send_period_ms < 0)
		icmp_st_static.send_period_ms = 0;

	while (send_count++ != icmp_st_static.send_count)
	{
		icmp_start_sending(&icmp_st_static);

		if ((send_count + 1) < icmp_st_static.send_count)
			usleep (icmp_st_static.send_period_ms);
	}

	if (addr_in->source)
		free (addr_in->source);

	if (addr_in->dest)
		free (addr_in->dest);

	icmp_log ("%s - done\n", __func__);

	return NULL;
}

void *icmp_send_packet_thread (void *arg)
{
	pthread_mutex_lock(&thread_init_mutex);

	int i = 0, rc, packet_size;
	icmp_pull_t *icmp_pull = (icmp_pull_t *)arg;;
	icmp_addr_t *source = icmp_pull->source;
	icmp_addr_t *dest = icmp_pull->dest;
	icmp_result_t request_result = icmp_pull->request_result;
	int dest_count = icmp_pull->dest_count;

	icmp_pull->pid = syscall(SYS_gettid);

	pthread_mutex_unlock(&thread_init_mutex);

	icmp_log ("%s - inited\n", __func__);

	uint8_t *icmp_packet;

	rc = icmp_packet_init (&icmp_packet, &packet_size);
	if (rc != 0)
	{
		icmp_log ("failed to init icmp packet");
		return NULL;
	}

	for (i = 0; i < dest_count; i++)
	{
		icmp_log ("%s -> %s", source->addr, dest->addr);

		rc = icmp_send_packet (icmp_packet, packet_size,
		                       source->addr, dest->addr);

		icmp_log(" rc %d request_result %p", rc, request_result);

		if (request_result)
		{
			request_result (dest->addr, rc);
		}

		dest++;
	}

	icmp_packet_deinit (icmp_packet);

	icmp_log ("%s - done\n", __func__);

	return NULL;
}

static int icmp_sock_init (int *sockfd)
{
	int on = 1, rc;

	*sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (*sockfd < 0) 
	{
		icmp_log("could not create socket: %s", strerror(errno));
		return -1;
	}

	// We shall provide IP headers
	rc = setsockopt (*sockfd, IPPROTO_IP, IP_HDRINCL, (const char*)&on, sizeof (on));
	if (rc == -1)
	{
		icmp_log("IP_HDR: %s", strerror(errno));
		return -2;
	}

	// //allow socket to send datagrams to broadcast addresses
	// rc = setsockopt (*sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof (on));
	// if (rc == -1)
	// {
	// 	icmp_log("BROADCAST: %s", strerror(errno));
	// 	return -3;
	// }

	return 0;
}

static int icmp_sock_deinit (int *sockfd)
{
	close(*sockfd);

	return 0;
}

static unsigned short in_cksum(unsigned short *ptr, int nbytes)
{
	register long sum;
	u_short oddbyte;
	register u_short answer;

	sum = 0;
	while (nbytes > 1)
	{
		sum += *ptr++;
		nbytes -= 2;
	}

	if (nbytes == 1)
	{
		oddbyte = 0;
		*((u_char *) & oddbyte) = *(u_char *) ptr;
		sum += oddbyte;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return (answer);
}

static int icmp_packet_init (uint8_t **icmp_packet, int *packet_size)
{
	struct iphdr *ip_hdr;
	struct icmphdr *icmp_hdr;
	int payload_size = 10;

	*packet_size = sizeof (struct iphdr) + sizeof (struct icmphdr) + payload_size;

	*icmp_packet = (uint8_t *) malloc (*packet_size);
	if (!*icmp_packet) 
	{
		icmp_log("%s", strerror(errno));
		return -1;
	}

	ip_hdr = (struct iphdr *) (*icmp_packet);
	icmp_hdr = (struct icmphdr *) ((*icmp_packet) + sizeof (struct iphdr));

	memset (*icmp_packet, 0, *packet_size);

	ip_hdr->version = 4;
	ip_hdr->ihl = 5;
	ip_hdr->tos = 0;
	ip_hdr->tot_len = htons (*packet_size);
	ip_hdr->id = rand ();
	ip_hdr->frag_off = 0;
	ip_hdr->ttl = 255;
	ip_hdr->protocol = IPPROTO_ICMP;

	// ip_hdr->saddr = saddr;
	// ip_hdr->daddr = daddr;

	//ip->check = in_cksum ((u16 *) ip, sizeof (struct iphdr));

	icmp_hdr->type = ICMP_ECHO;
	icmp_hdr->code = 0;
	icmp_hdr->un.echo.sequence = rand();
	icmp_hdr->un.echo.id = rand();

	memset((*icmp_packet) + sizeof(struct iphdr) + sizeof(struct icmphdr),
	        rand() % 255,
	        payload_size);

	icmp_hdr->checksum = 0;
	icmp_hdr->checksum = in_cksum((unsigned short *)icmp_hdr,
	                          sizeof(struct icmphdr) + payload_size);

	return 0;
}

static void icmp_packet_deinit (uint8_t *icmp_packet)
{
	if (!icmp_packet)
		return;

	free (icmp_packet);
}

static int icmp_send_packet (uint8_t *icmp_packet, int packet_size,
                             char *source, char *dest)
{
	int rc, ping_result = 0;
	int addrlen = 0;
	int sockfd;
	struct sockaddr_in servaddr;
	struct timeval tv;
	struct iphdr *ip_hdr;
	uint8_t icmp_recv_packet[128] = {0};
	uint32_t net_source, net_dest;
	fd_set rfds;

	rc = icmp_sock_init (&sockfd);
	if (rc != 0)
	{
		icmp_log ("failed to init socket");
		return -1;
	}

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	net_source = inet_addr(source);
	net_dest = inet_addr(dest);

	ip_hdr = (struct iphdr *)icmp_packet;
	ip_hdr->saddr = net_source;
	ip_hdr->daddr = net_dest;

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = net_dest;
	memset(&servaddr.sin_zero, 0, sizeof (servaddr.sin_zero));

	icmp_log ("SEND packet:\n");
	// icmp_log ("ip->version %d\n", ip_hdr->version);
	// icmp_log ("ip_hdr->ihl %d\n", ip_hdr->ihl);
	// icmp_log ("ip_hdr->tos %d\n", ip_hdr->tos);
	// icmp_log ("ip_hdr->tot_len %d\n", ip_hdr->tot_len);
	// icmp_log ("ip_hdr->id %d\n", ip_hdr->id);
	// icmp_log ("ip_hdr->frag_off %d\n", ip_hdr->frag_off);
	// icmp_log ("ip_hdr->ttl %d\n", ip_hdr->ttl);
	// icmp_log ("ip_hdr->protocol %d\n", ip_hdr->protocol);
	icmp_log ("ip_hdr->saddr %08x\n", ip_hdr->saddr);
	icmp_log ("ip_hdr->daddr %08x\n", ip_hdr->daddr);

	/* loopback */
	if (ip_hdr->saddr == ip_hdr->daddr)
	{
		icmp_sock_deinit (&sockfd);
		return 0;
	}

	rc = sendto(sockfd, icmp_packet, packet_size, 0,
	            (struct sockaddr*) &servaddr, sizeof (servaddr));
	if (rc < 1)
	{
		icmp_log("send failed: %s", strerror(errno));
		icmp_sock_deinit (&sockfd);
		return -3;
	}

	tv.tv_sec = 1;
	tv.tv_usec = 0;

retry:
	if(select (sockfd + 1, &rfds, NULL, NULL, &tv))
	{
		if(FD_ISSET(sockfd, &rfds))
		{
			rc = recvfrom(sockfd, (void *)icmp_recv_packet,
			              sizeof (icmp_recv_packet), 0,
			              (struct sockaddr*) &servaddr, &addrlen);
			if (rc > 1)
			{
				// struct icmphdr *icmp_hdr;

				ip_hdr = (struct iphdr *) icmp_recv_packet;
				// icmp_hdr = (struct icmphdr *) (ip_hdr + sizeof (struct iphdr));

				icmp_log ("<%p> RECV packet: %d from %08x", ip_hdr, sockfd, ip_hdr->saddr);

				if (ip_hdr->saddr != net_dest)
				{
					icmp_log ("FFFFFFUUUUU: MYSTA FAKIG BLYADISH SYKEN! %08x -> %08x !!", ip_hdr->saddr, net_dest);
					FD_ZERO(&rfds);
					FD_SET(sockfd, &rfds);
					tv.tv_sec = 1;
					tv.tv_usec = 0;
					goto retry;
				}

				// icmp_log ("ip->version %d\n", ip_hdr->version);
				// icmp_log ("ip_hdr->ihl %d\n", ip_hdr->ihl);
				// icmp_log ("ip_hdr->tos %d\n", ip_hdr->tos);
				// icmp_log ("ip_hdr->tot_len %d\n", ip_hdr->tot_len);
				// icmp_log ("ip_hdr->id %d\n", ip_hdr->id);
				// icmp_log ("ip_hdr->frag_off %d\n", ip_hdr->frag_off);
				// icmp_log ("ip_hdr->ttl %d\n", ip_hdr->ttl);
				// icmp_log ("ip_hdr->protocol %d\n", ip_hdr->protocol);
				icmp_log ("ip_hdr->saddr %08x\n", ip_hdr->saddr);
				icmp_log ("ip_hdr->daddr %08x\n", ip_hdr->daddr);
				// icmp_log ("icmp_hdr->type %d\n", icmp_hdr->type);
				// icmp_log ("icmp_hdr->code %d\n", icmp_hdr->code);
				// icmp_log ("icmp_hdr->un.echo.sequence %d\n", icmp_hdr->un.echo.sequence);
				// icmp_log ("icmp_hdr->un.echo.id %d\n", icmp_hdr->un.echo.id);
				// icmp_log ("icmp_hdr->checksum %x\n", icmp_hdr->checksum);

				// ping_result = 0;
			}
		}
	} else {
		ping_result = ICMP_TIMEOUT;
	}

	icmp_sock_deinit (&sockfd);

	return ping_result;
}