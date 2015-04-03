#ifndef _ICMP_CORE_H_
#define _ICMP_CORE_H_

typedef void (*icmp_result_t)(char *dest_address, int send_result);

#define T100MS 1000000

#define ADDR_BY_PULL 3

#define MAX_DEST_ADDR_LEN 64

typedef struct {
	char addr[MAX_DEST_ADDR_LEN];
} icmp_addr_t;

typedef struct {
	icmp_addr_t *source;
	icmp_addr_t *dest;
	int dest_count;
} icmp_addr_in_t;

int icmp_send_packet
(
	icmp_addr_in_t *addr_in,
	/* added wait reply timeout */
	icmp_result_t request_result
);

int icmp_send_periodical
(
	icmp_addr_in_t *addr_in,
	/* added wait reply timeout */
	int send_period_ms, /* milliseconds */
	int send_count,
	icmp_result_t request_result
);

#endif /* _ICMP_CORE_H_ */
