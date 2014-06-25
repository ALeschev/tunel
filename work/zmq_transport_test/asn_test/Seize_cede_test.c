#include <stdio.h>
#include <string.h>

#include "asn_der_msg.h"

int main(int argc, char const *argv[])
{
	char buffer[1024] = {0};

#if 0 /* ConnectionResponseType TEST*/
	if (asn_der_encode_ConnectionResponse (165, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		int timeout = -1;

		asn_ber_decode_ConnectionResponse (&timeout, buffer);

		printf("timeout %d\n", timeout);
	}
#endif

#if 0 /* ConnectionUpdateReques TEST*/
	if (asn_der_encode_ConnectionUpdateRequest (1, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char timestamp[128] = {0};

		asn_ber_decode_ConnectionUpdateRequest (timestamp, buffer);

		if (strlen(timestamp))
			printf("timestamp %s\n", timestamp);
		else
			printf("timestamp not set\n");
	}
#endif

#if 0 /* ConnectionUpdateResponse TEST*/
	if (asn_der_encode_ConnectionUpdateResponse (1, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char timestamp[128] = {0};

		asn_ber_decode_ConnectionUpdateResponse (timestamp, buffer);

		if (strlen(timestamp))
			printf("timestamp %s\n", timestamp);
		else
			printf("timestamp not set\n");
	}
#endif

#if 1 /* ConnectionRejectType TEST*/
	if (asn_der_encode_ConnectionReject (reject_hate_you, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		// int timeout = -1;

		// asn_ber_decode_ConnectionResponse (&timeout, buffer);

		// printf("timeout %d\n", timeout);
	}
#endif

ext:
	return 0;
}