/*
 * LAV by fun
 */

#include <stdlib.h>
#include <errno.h>

#include "logger.h"
#include "client_base.h"

#define return_if_invalid(prefix, condition, return_code)        \
            if (condition)                                       \
            {                                                    \
                log_error("%s: base condition failed", prefix);  \
                return return_code;                              \
            }

#define return_if_base_invalid(base, return_code)                        \
               return_if_invalid(__func__, !base, return_code);          \
               return_if_invalid(__func__, !base->clients, return_code); \
               return_if_invalid(__func__, !base->writes, return_code)

static void c_client_set_state(c_client_t *client, e_state_t state)
{
	if (!client)
		return;

	if (client->state != state)
	{
		log_info("client#%d [%s]: %s -> %s", client->id, client->nickname,
				client_state_to_str(client->state), client_state_to_str(state));

		client->state = state;
	}
}

int c_base_init(c_client_base_t *base, size_t writes)
{
	int i;

	return_if_invalid(__func__, !base, -1);
	return_if_invalid(__func__, base->clients, -2);

	base->clients = (c_client_t *) calloc(1, sizeof (c_client_t) * writes);
	if (!base->clients)
	{
		log_error("failed to allocate base: %s", strerror(errno));
		return errno;
	}

	base->writes = writes;

	for (i = 0; i < base->writes; i++)
	{
		memset(&base->clients[i], 0, sizeof(c_client_t));
		base->clients[i].id = i;
		c_client_set_state(&base->clients[i], eCSTATE_INITED);
	}

	return 0;
}

int c_base_close(c_client_base_t *base)
{
	int i;

	return_if_base_invalid(base, -1);

	for (i = 0; i < base->writes; i++)
	{
		if (base->clients[i].state != eCSTATE_INITED)
		{
			c_client_set_state(&base->clients[i], eCSTATE_INITED);
			memset(&base->clients[i], 0, sizeof(c_client_t));
		}
	}

	free(base->clients);
	base->clients = NULL;

	return 0;
}

c_client_t *c_client_add(c_client_base_t *base, int trid, char *nickname, size_t len)
{
	int i;
	size_t nick_len = MAX_STR_LEN - 1;

	return_if_base_invalid(base, NULL);

	if (!nickname)
		return NULL;

	if (len < MAX_STR_LEN)
	{
		nick_len = len;
	}

	for(i = 0; i < base->writes; i++)
	{
		if (base->clients[i].state == eCSTATE_INITED)
		{
			base->clients[i].trid = trid;
			base->clients[i].id = i;
			base->clients[i].nick_len = nick_len;
			strncpy(base->clients[i].nickname, nickname, nick_len);
			c_client_set_state(&base->clients[i], eCSTATE_CONNECTED);

			return &base->clients[i];
		}
	}

	return NULL;
}

int c_client_rem(c_client_base_t *base, c_client_t *client)
{
	int i;

	return_if_base_invalid(base, -1);

	if (!client)
		return -1;

	if (client->id < base->writes)
	{
		memset(client, 0, sizeof (c_client_t));
		c_client_set_state(client, eCSTATE_INITED);

		// memset(&client_base[client->id], 0, sizeof(c_client_t));
		// c_client_set_state(&client_base[client->id], eCSTATE_INITED);
	}

	return 0;
}

c_client_t *c_client_get_by_id(c_client_base_t *base, uint16_t id)
{
	return_if_base_invalid(base, NULL);

	if (id < base->writes)
		return &base->clients[id];

	return NULL;
}

c_client_t *c_client_get_by_trid(c_client_base_t *base, int trid)
{
	int i;

	return_if_base_invalid(base, NULL);

	for(i = 0; i < base->writes; i++)
	{
		if ((base->clients[i].state == eCSTATE_CONNECTED)
			&& (base->clients[i].trid == trid))
		{
				return &base->clients[i];
		}
	}

	return NULL;
}

c_client_t *c_client_get_by_nick(c_client_base_t *base, char *nickname, size_t len)
{
	int i;

	return_if_base_invalid(base, NULL);

	if (!nickname || len >= MAX_STR_LEN)
		return NULL;

	for(i = 0; i < base->writes; i++)
	{
		if ((base->clients[i].state == eCSTATE_CONNECTED)
			&& (base->clients[i].nick_len == len))
		{
			if (!strcmp(base->clients[i].nickname, nickname))
				return &base->clients[i];
		}
	}

	return NULL;
}

void c_base_show(c_client_base_t *base, int connected_only)
{
	int i, print = 1;

	return_if_base_invalid(base, );

	for (i = 0; i < base->writes; i++)
	{
		if (connected_only)
		{
			if (base->clients[i].state != eCSTATE_CONNECTED)
			{
				print = 0;
			}
		}

		if (print)
		{
			log_info("id %d; trid %d; nick '%s'; state '%s';",
			         base->clients[i].id, base->clients[i].trid,
			         base->clients[i].nickname,
			         client_state_to_str(base->clients[i].state));
		}

		print = 1;
	}
}
