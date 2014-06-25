#ifndef _ASN_DER_MSG_H_
#define _ASN_DER_MSG_H_

enum reject_cause {
	reject_prost = 0,
	reject_hate_you,

	reject_cause_max
};

int asn_encode_ConnectionResponse (char *swSID, int appSID, long check_time, void *buffer, int buffer_size);
int asn_decode_ConnectionResponse (char *swSID, int *appSID, int *updateTimeout, void *buffer);

int asn_encode_ConnectionReject (char *swSID, int appSID, int cause, void *buffer, int buffer_size);

int asn_encode_ConnectionUpdateRequest (char *swSID, int appSID, int set_timestamp, void *buffer, int buffer_size);
int asn_decode_ConnectionUpdateRequest (char *swSID, int *appSID, char *timestamp, void *buffer);

int asn_encode_ConnectionUpdateResponse (char *swSID, int appSID, int set_timestamp, void *buffer, int buffer_size);
int asn_decode_ConnectionUpdateResponse (char *swSID, int *appSID, char *timestamp, void *buffer);

int asn_encode_Release (char *swSID, int appSID, char *str_cause, int noCDR, int detached, char *toLog, void *buffer, int buffer_size);
int asn_decode_Release (char *swSID, int *appSID, char *str_cause, char *timestamp, int *noCDR, int *detached, char *toLog, void *buffer);

#endif /*_ASN_DER_MSG_H_*/