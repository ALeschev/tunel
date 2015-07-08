/*
 * LAV by fun
 */

#ifndef __CLIENT_BASE_H__
#define __CLIENT_BASE_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

/* value limited by uint16_t (client id) type */
#define MAX_CLIENTS 32

#define MAX_STR_LEN 64

typedef enum e_client_state
{
	eCSTATE_NONE = 0,

	eCSTATE_INITED,
	eCSTATE_CONNECTED,

	eCSTATE_MAX,
} e_state_t;

/* remove this shit to...emm..otherwhere */
typedef struct
{
	uint32_t ip;
	size_t ip_len;
	char ipaddr[MAX_STR_LEN];
} s_addr_t;

typedef struct
{
	int fd;
	uint16_t id;
	e_state_t state;
	size_t nick_len;
	char nickname[MAX_STR_LEN];
	s_addr_t ip_addr;
} c_client_t;

typedef struct
{
	c_client_t *clients;
	size_t writes;
} c_client_base_t;

static const char *client_state_str[eCSTATE_MAX] = 
{
	"none",
	"inited",
	"connected"
};

static inline const char *client_state_to_str(e_state_t state)
{
	if ((eCSTATE_NONE < state) && (state < eCSTATE_MAX))
		return client_state_str[state];

	return "Unknown";
}

int c_base_init(c_client_base_t *base, size_t writes);
int c_base_close(c_client_base_t *base);
c_client_t *c_client_add(c_client_base_t *base, int fd, char *nickname, size_t len);
int c_client_rem(c_client_base_t *base, c_client_t *client);
c_client_t *c_client_get_by_id(c_client_base_t *base, uint16_t id);
c_client_t *c_client_get_by_fd(c_client_base_t *base, int fd);
c_client_t *c_client_get_by_nick(c_client_base_t *base, char *nickname, size_t len);
c_client_t *c_client_get_by_ip(c_client_base_t *base, s_addr_t *ip_addr);

#endif /* __CLIENT_BASE_H__ */
