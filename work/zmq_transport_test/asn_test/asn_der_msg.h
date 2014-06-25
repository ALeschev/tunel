#ifndef _ASN_DER_MSG_H_
#define _ASN_DER_MSG_H_

enum reject_cause {
	reject_prost = 0,
	reject_hate_you,

	reject_cause_max
};

int asn_der_encode_ConnectionResponse (long check_time, void *buffer, int buffer_size);
int asn_ber_decode_ConnectionResponse (int *updateTimeout, void *buffer);

int asn_der_encode_ConnectionReject (int cause, void *buffer, int buffer_size);

int asn_der_encode_ConnectionUpdateRequest (int set_timestamp, void *buffer, int buffer_size);
int asn_ber_decode_ConnectionUpdateRequest (char *timestamp, void *buffer);

int asn_der_encode_ConnectionUpdateResponse (int set_timestamp, void *buffer, int buffer_size);
int asn_ber_decode_ConnectionUpdateResponse (char *timestamp, void *buffer);

#endif /*_ASN_DER_MSG_H_*/