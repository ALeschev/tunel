#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "pbyte_msg_api.h"
#include "pbyte_log.h"

/*----------------------------------------------------------------------------*/

const char *pb_msg_type_to_str(msg_type_t type)
{
	switch(type)
	{
		case CONREQ: return "CONREQ";
		case CONRSP: return "CONRSP";
		case CONUPD: return "CONUPD";
		case CONUPDACK: return "CONUPDACK";
		case CONCLOSE: return "CONCLOSE";
		case TRANSFER: return "TRANSFER";
	}

	return "UNDEF";
}

/*----------------------------------------------------------------------------*/

const char *pb_payload_type_to_str(data_type_t type)
{
	switch(type)
	{
		case CONTROLCHANNEL: return "CONTROLCHANNEL";
	}

	return "UNDEF";
}

/*----------------------------------------------------------------------------*/

int pb_identity_to_str (const identity_t *identity, int ext, char *out_buf, int buf_len)
{
	int i, offset = 0;

	if (!out_buf || buf_len <= 0)
		return -1;

	memset (out_buf, 0, buf_len);

	if (!identity)
	{
		strncpy (out_buf, "Identity error", (buf_len > 14)? 14:buf_len - 1);
		return -1;
	}

	if (buf_len <= identity->identity_len * 2)
	{
		strncpy (out_buf, "Small buffer size", (buf_len > 17)? 17:buf_len - 1);
		return -2;
	}

	for (i = 0; i < identity->identity_len && i < PBYTE_IDENTITY_LEN; i++)
	{
		offset += sprintf(out_buf + offset, "%02X",
		                  (unsigned char)identity->identity[i]);
	}

	/*
	 * identity some like a 6D3137313035313262353365646139346234
	 * total len - 36
	 */

	if (ext && ((buf_len - offset) > 15))
	{
		sprintf(out_buf + offset, " [%d] %sctive",
		        identity->idx,
		        identity->active? "A":"Ina");
	}

	return 0;
}

/*----------------------------------------------------------------------------*/

void pb_print_buff (pb_logger_t *logger, int lvl, const char *descript, const char *data, int size)
{
	if (!logger)
		return;

	if (!pb_log_check_enable (logger, lvl))
		return;

	if (descript && strlen (descript))
		pb_logger (logger, lvl, "%s:", descript);

	if (!data || size == 0)
	{
		pb_logger (logger, lvl, "     (empty)");
		return;
	}

#if 0
	int i;
	char out_buffer[64] = {0};
	char tmp_buff[64] = {0};

	for (i = 0; i < size; i++)
	{
		if (!(i%16))
		{
			if (i != 0)
			{
				pb_logger (logger, lvl, "%s", out_buffer);
			}
			sprintf (out_buffer, "%d: ", i);
		}
		sprintf (tmp_buff, "%02X ", (unsigned)data[i]);
		strcat (out_buffer, tmp_buff);
	}

	if (i % 16)
	{
		// strcat (out_buffer, tmp_buff);
		pb_logger (logger, lvl, "%s", out_buffer);
	}

	pb_logger (logger, lvl, "Total: %d bytes", size);
#else
	pb_logger (logger, lvl, "%.*s", size, data);

	pb_logger (logger, lvl, "Total: %d bytes", size);
#endif
}

/*----------------------------------------------------------------------------*/

void pb_msg_print(pb_logger_t *logger, int lvl, const char *desc, const pb_msg_t *pb_msg)
{
	const msg_params_t *p_params;
	char identity_str[256] = {0};

	if (!logger || !pb_msg)
		return;

	if (lvl < PBYTE_CUT)
		return;

	if (!pb_log_check_enable (logger, lvl))
		return;

	p_params = &pb_msg->message_params;

	pb_identity_to_str(&pb_msg->identity, 0, identity_str, sizeof (identity_str));

	pb_logger (logger, lvl, "description:  '%s'", desc? desc:"");
	pb_logger (logger, lvl, "identity:     '%s'", identity_str);
	pb_logger (logger, lvl, "message type: '%s'", pb_msg_type_to_str(p_params->msg_type));
	pb_logger (logger, lvl, "payload type: '%s'", pb_payload_type_to_str(p_params->data_type));
	pb_logger (logger, lvl, "dialog:       '%s'", pb_msg->identity.dialog.dialog_id);
	pb_logger (logger, lvl, "dialog_len:   '%d'", pb_msg->identity.dialog.dialog_len);

	pb_print_buff (logger, PBYTE_FULL, desc, pb_msg->data, pb_msg->size);
}

/*----------------------------------------------------------------------------*/

int pb_msg_set_identity (pb_msg_t *pb_msg,
                         identity_t *identity)
{
	if (!pb_msg || !identity)
		return -1;

	pb_msg->identity.active = identity->active;
	pb_msg->identity.idx = identity->idx;
	pb_msg->identity.identity_len = identity->identity_len;
	memcpy (pb_msg->identity.identity, identity->identity, identity->identity_len);

	return 0;
}

/*----------------------------------------------------------------------------*/

int pb_msg_set_dialog (identity_t *identity,
                          char *dialog_id,
                          int dialog_len)
{
	if (!identity || !dialog_id)
		return -1;

	identity->dialog.dialog_len = dialog_len;
	memcpy (identity->dialog.dialog_id, dialog_id, dialog_len);

	return 0;
}

/*----------------------------------------------------------------------------*/

int pb_msg_set_types (pb_msg_t *pb_msg,
                         msg_type_t msg_type,
                         data_type_t data_type)
{
	if (!pb_msg)
		return -1;

	pb_msg->message_params.msg_type = msg_type;
	pb_msg->message_params.data_type = data_type;

	return 0;
}

/*----------------------------------------------------------------------------*/

int pb_msg_unpack (pb_msg_t **pb_msg, char *data, int size)
{
	int step = 0;
	msg_params_t *p_params;

	if (!data || size <= 0)
		return -1;

	*pb_msg = (pb_msg_t *)calloc(1, sizeof(pb_msg_t));
	if (!*pb_msg)
		return -2;

	p_params = &(*pb_msg)->message_params;

	p_params->msg_type = (msg_type_t)data[step++];
	if (p_params->msg_type == TRANSFER)
	{
		p_params->data_type = (data_type_t)data[step++];
		memcpy (&(*pb_msg)->identity.dialog.dialog_len, &data[step], sizeof (unsigned char));
		step += sizeof (unsigned char);
		memcpy ((*pb_msg)->identity.dialog.dialog_id, &data[step], (*pb_msg)->identity.dialog.dialog_len);
		step += (*pb_msg)->identity.dialog.dialog_len;
		(*pb_msg)->size = size - step;

		(*pb_msg)->data = (char *)calloc(1, (*pb_msg)->size + 1);
		if (!(*pb_msg)->data)
		{
			free(*pb_msg);
			return -3;
		}

		memcpy ((*pb_msg)->data, &data[step], (*pb_msg)->size);
		(*pb_msg)->data[(*pb_msg)->size] = '\0';
	}

	return 0;
}

/*----------------------------------------------------------------------------*/

int pb_msg_pack (pb_msg_t *pb_msg, char **data, int *size)
{
	int step = 0;
	dialog_t *p_dialog;
	msg_params_t *p_params;

	if (!pb_msg)
		return -1;

	p_params = &pb_msg->message_params;
	p_dialog = &pb_msg->identity.dialog;

	if (p_params->msg_type == TRANSFER)
	{
		*size = 3 /* field: msg_type, data_type, dialog_len*/ +
		        p_dialog->dialog_len + pb_msg->size;

	} else {
		*size = 1;
	}

	if (p_dialog->dialog_len >= MAX_ALPHANUMS)
	{
		*size -= p_dialog->dialog_len + MAX_ALPHANUMS - 1;
		p_dialog->dialog_len = MAX_ALPHANUMS - 1;
	}

	*data = (char *)calloc(1, *size);
	if (!*data)
		return -2;

	if (p_params->msg_type == TRANSFER)
	{
		(*data)[step++] = p_params->msg_type;
		(*data)[step++] = p_params->data_type;
		memcpy (&(*data)[step], &p_dialog->dialog_len, sizeof (unsigned char));
		step += sizeof (unsigned char);
		memcpy (&(*data)[step], p_dialog->dialog_id, p_dialog->dialog_len);
		step += p_dialog->dialog_len;

		memcpy (&(*data)[step], pb_msg->data, pb_msg->size);
	} else {
		(*data)[step++] = p_params->msg_type;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/

int pb_msg_init (pb_msg_t **pb_msg, int data_size)
{
	int error = 0;

	if (!*pb_msg)
	{
		*pb_msg = (pb_msg_t *)calloc(1, sizeof (pb_msg_t));
		if (!*pb_msg)
		{
			error = -1;
			goto err;
		}

		memset (*pb_msg, 0, sizeof (pb_msg_t));
	}

	if (!(*pb_msg)->data && data_size > 0)
	{
		(*pb_msg)->data = (char *)calloc(1, data_size + 1);
		if (!(*pb_msg)->data)
		{
			error = -2;
			goto err;
		}

		memset ((*pb_msg)->data, 0, data_size + 1);
	}

	return 0;

err:
	if (*pb_msg)
		free(*pb_msg);

	return error;
}

/*----------------------------------------------------------------------------*/

int pb_msg_put_data (pb_msg_t *pb_msg, char *data, int size)
{
	if (!pb_msg || !data || size <= 0)
		return -1;

	memcpy (pb_msg->data, data, size);
	pb_msg->size = size;

	return 0;
}

/*----------------------------------------------------------------------------*/

void pb_msg_unpack_free (pb_msg_t *pb_msg)
{
	if (!pb_msg)
		return;

	if (pb_msg->data && (pb_msg->size > 0))
	{
		free(pb_msg->data);
		pb_msg->data = NULL;
	}

	free(pb_msg);
	pb_msg = NULL;
}

/*----------------------------------------------------------------------------*/

void pb_msg_pack_free (char *data)
{
	if (!data)
		return;

	free (data);
	data = NULL;
}

/*----------------------------------------------------------------------------*/
