#include <stdio.h>
#include <string.h>

#include "asn_der_msg.h"

int main(int argc, char const *argv[])
{
	char buffer[1024] = {0};

#if 0 /* ConnectionResponseType TEST*/
	if (asn_encode_ConnectionResponse (165, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		int timeout = -1;

		asn_decode_ConnectionResponse (&timeout, buffer);

		printf("timeout %d\n", timeout);
	}
#endif

#if 0 /* ConnectionUpdateReques TEST*/
	if (asn_encode_ConnectionUpdateRequest (1, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char timestamp[128] = {0};

		asn_decode_ConnectionUpdateRequest (timestamp, buffer);

		if (strlen(timestamp))
			printf("timestamp %s\n", timestamp);
		else
			printf("timestamp not set\n");
	}
#endif

#if 0 /* ConnectionUpdateResponse TEST*/
	if (asn_encode_ConnectionUpdateResponse (1, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char timestamp[128] = {0};

		asn_decode_ConnectionUpdateResponse (timestamp, buffer);

		if (strlen(timestamp))
			printf("timestamp %s\n", timestamp);
		else
			printf("timestamp not set\n");
	}
#endif

#if 0 /* ConnectionRejectType TEST*/
	if (asn_encode_ConnectionReject (reject_hate_you, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		// int timeout = -1;

		// asn_decode_ConnectionResponse (&timeout, buffer);

		// printf("timeout %d\n", timeout);
	}
#endif

#if 1 /* ReleaseType TEST*/
	if (asn_encode_Release ("6845ws", 1024, "test_cause", 0, 14, "this is bad user and i release him :)", buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		int APPSID = 0;
		char SWSID[64] = {0};
		char str_cause[128] = {0};
		char timestamp[128] = {0};
		int  noCDR = 0;
		int  detached = 0;
		char toLog[128] = {0};

		asn_decode_Release (SWSID, &APPSID, str_cause, timestamp, &noCDR, &detached, toLog, buffer);

		printf("str_cause: %s\ntimestamp: %s\nnoCDR: %d\ndetached: %d\ntoLog: %s\n", str_cause, timestamp, noCDR, detached, toLog);
	}
#endif

ext:
	return 0;
}