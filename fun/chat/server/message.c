/*
 * LAV by fun
 */

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include "logger.h"
#include "message.h"

m_message_t *m_message_init(m_header_t *header, uint8_t *data, size_t len)
{
	m_message_t *new_msg;

	if (!header || !data)
	{
		return NULL;
	}

	new_msg = (m_message_t *) calloc (1, sizeof (m_header_t) + len + 1);
	if (!new_msg)
	{
		log_error("failed to create new message: %u -> %u: %s",
				  header->from_id, header->to_id, strerror(errno));

		return NULL;
	}

	memcpy(&new_msg->header, header, sizeof (m_header_t));
	memcpy(new_msg->payload, data, len);

	return new_msg;
}

int m_message_close(m_message_t *message)
{
	if (message)
	{
		free(message);
	}

	return 0;
}
