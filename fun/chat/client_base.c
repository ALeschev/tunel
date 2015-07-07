/*
 * LAV by fun
 */

#include "log.h"
#include "client_base.h"

static c_client_t client_base[MAX_CLIENTS];

static void c_client_set_state(c_client_t *client, uint8_t state)
{
	if (!client)
		return;

	if (client->state != state)
	{
		log_info("client#%d [%s]: %s -> %s", client->id, client->nickname,
				client_state_to_str(client->state), client_state_to_str(state));
	}
}

int c_base_init(void)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		memset(&client_base[i], 0, sizeof(c_client_t));
		c_client_set_state(&client_base[i], eCSTATE_INITED);
	}

	return 0;
}

int c_base_close(void)
{
	int i;

	/* shit. shit. shit. shit */

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		memset(&client_base[i], 0, sizeof(c_client_t));
		c_client_set_state(&client_base[i], eCSTATE_INITED);
	}

	return 0;
}

c_client_t *c_client_add(int fd, char *nickname, size_t len)
{
	int i;
	size_t nick_len = MAX_STR_LEN - 1;

	if (!nickname)
		return -1;

	if (len < MAX_STR_LEN)
	{
		nick_len = len;
	}

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if (client_base[i].state == eCSTATE_INITED)
		{
			client_base[i].fd = fd;
			client_base[i].id = i;
			client_base[i].nick_len = nick_len;
			strncpy(client_base[i].nickname, nickname, nick_len);
			c_client_set_state(&client_base[i], eCSTATE_CONNECTED);

			return &client_base[i];
		}
	}

	return NULL;
}

int c_client_rem(c_client_t *client)
{
	int i;

	if (!client)
		return -1;

	if (client->id < MAX_CLIENTS)
	{
		memset(client, 0, sizeof (c_client_t));
		c_client_set_state(client, eCSTATE_INITED);

		// memset(&client_base[client->id], 0, sizeof(c_client_t));
		// c_client_set_state(&client_base[client->id], eCSTATE_INITED);
	}

	return 0;
}

c_client_t *c_client_get_by_id(uint16_t id)
{
	if (id < MAX_CLIENTS)
		return &client_base[id];

	return NULL;
}

c_client_t *c_client_get_by_fd(int fd)
{
	int i;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if ((client_base[i].state == eCSTATE_CONNECTED)
			&& (client_base[i].fd == fd))
		{
			return &client_base[i];
		}
	}

	return NULL;
}

c_client_t *c_client_get_by_nick(char *nickname, size_t len)
{
	int i;

	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if ((client_base[i].state == eCSTATE_CONNECTED)
			&& (client_base[i].nick_len == len))
		{
			if (!strcmp(client_base[i].nickname, nickname, len))
				return &client_base[i];
		}
	}

	return NULL;
}
