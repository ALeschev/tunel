#ifndef __PBYTE_MSG_API_H__
#define __PBYTE_MSG_API_H__

#include "pbyte_transport.h"

int pb_msg_set_identity (pb_msg_t *pb_msg,
                            const pb_identity_t *identity);
int pb_msg_set_dialog (pb_identity_t *identity,
                          const char *dialog_id,
                          int dialog_len);
int pb_msg_set_types (pb_msg_t *pb_msg,
                         pb_msg_type_t msg_type,
                         pb_data_type_t data_type);

int pb_msg_unpack (pb_msg_t **pb_msg, const char *data, int size);
int pb_msg_pack (const pb_msg_t *pb_msg, char **data, int *size);
int pb_msg_init (pb_msg_t **pb_msg, int data_size);
int pb_msg_put_data (pb_msg_t *pb_msg, char *data, int size);
void pb_msg_unpack_free (pb_msg_t **pb_msg);
void pb_msg_pack_free (char **data);
// void pb_msg_unpack_free (pb_msg_t *pb_msg);

int pb_pb_identity_to_str (const pb_identity_t *identity, int ext, char *out_buf, int buf_len);

#endif