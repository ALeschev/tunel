/*
 * LAV by fun
 */

#ifndef __CLIENT_BASE_H__
#define __CLIENT_BASE_H__

#include <stdint.h>
#include <pthread.h>

/* value limited by uint16_t (client id) type */
#define MAX_CLIENTS 32

#define MAX_STR_LEN 64

enum e_client_state
{
	eCSTATE_NONE = 0,

	eCSTATE_INITED,
	eCSTATE_CONNECTED,

	eCSTATE_MAX,
};

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
	uint8_t state;
	size_t nick_len;
	char nickname[MAX_STR_LEN];
	s_addr_t ip_addr;
} c_client_t;

char client_state_str[eCSTATE_MAX][] = 
{
	"none",
	"inited",
	"connected"
};

char *client_state_to_str(uint8_t state)
{
	char err_state[64] = {0};

	if (state < eCSTATE_MAX)
		return client_state_str[state];

	sprintf(err_state, "Unknown %u", state)

	return err_state;
}

int c_base_init(void);
int c_base_close(void);
c_client_t *c_client_add(int fd, char *nickname, size_t len);
int c_client_rem(c_client_t *client);
c_client_t *c_client_get_by_id(uint16_t id);
c_client_t *c_client_get_by_fd(int fd);
c_client_t *c_client_get_by_nick(char *nickname, size_t len);
c_client_t *c_client_get_by_ip(s_addr_t *ip_addr);

#endif /* __CLIENT_BASE_H__ */
