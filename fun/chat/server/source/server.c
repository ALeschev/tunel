/*
 * LAV by fun
 */

#include <stdio.h>

#include "log.h"
#include "client_base.h"
#include "message.h"
#include "transport.h"

/*
c_client_t *c_client_add(int fd, char *nickname, size_t len);
int c_client_rem(c_client_t *client);
c_client_t *c_client_get_by_id(uint16_t id);
c_client_t *c_client_get_by_fd(int fd);
c_client_t *c_client_get_by_nick(char *nickname, size_t len);
c_client_t *c_client_get_by_ip(s_addr_t *ip_addr);
*/



int main(void)
{
	int res;
	int i;
	char nickname[MAX_STR_LEN] = {0};
	c_client_t *test_client;

	c_client_base_t client_base = {NULL, 0};

	log_info("test");

	res = c_base_init(&client_base, MAX_CLIENTS);
	if (res != 0)
	{
		log_error("failed to init client base [%d]", res);
		return;
	}

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		sprintf (nickname, "test_nick%d", i);
		test_client = c_client_add(&client_base, i, nickname, strlen(nickname));
		if (!test_client)
		{
			log_error("failed to add client: %d %s", i, nickname);
			continue;
		}
	}

	c_base_close(&client_base);

	return 0;
}