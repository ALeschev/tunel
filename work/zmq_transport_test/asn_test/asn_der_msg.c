#include <time.h>

#include "asn_der_msg.h"

#include "SmarTIMessage.h"
#include "SmarTIBody.h"
#include "LegId.h"
#include "ReleaseType.h"
#include "Cause.h"
#include "der_encoder.h"
#include "ber_decoder.h"

#include "ConnectionRejectType.h"
#include "ConnectionRequestType.h"
#include "ConnectionResponseType.h"
#include "ConnectionUpdateRequestType.h"
#include "ConnectionUpdateResponseType.h"


char *reject_cause_str[reject_cause_max] =
{
	"prost))",
	"hate_you",
};

/*------------------------------------------------------------------------------------------------------------*/

static OCTET_STRING_t *
asn_generate_OCTET_STRING (OCTET_STRING_t *octet_str, char *str)
{
	if (!str || !octet_str)
		goto ext;

	OCTET_STRING_fromBuf (octet_str, str, strlen(str));

	return octet_str;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_fill (SmarTIMessage_t *SmarTIMessage, long version, char *swSessionID, char *appSessionID, SmarTIBody_t SmarTIBody)
{
	if (!SmarTIMessage || !swSessionID || !appSessionID)
		goto ext;

	SmarTIMessage->version = version;
	asn_generate_OCTET_STRING (&SmarTIMessage->legID.swSessionID, swSessionID);
	asn_generate_OCTET_STRING (&SmarTIMessage->legID.appSessionID, appSessionID);
	SmarTIMessage->body = SmarTIBody;

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIBody_t *
asn_SmarTIBody_fill (SmarTIBody_t *SmarTIBody, SmarTIBody_PR present, void *msg)
{
	if (!SmarTIBody || present < 0 || !msg)
		goto ext;

	union SmarTIBody_u *BodyChoise;

	BodyChoise = &SmarTIBody->choice;

	SmarTIBody->present = present;

	switch (present)
	{
		case SmarTIBody_PR_NOTHING:
			break;
/*		case SmarTIBody_PR_connectionReqest:
			BodyChoise->connectionReqest = *((ConnectionRequestType_t *)msg);
			break;*/
		case SmarTIBody_PR_connectionResponse:
			BodyChoise->connectionResponse = *((ConnectionResponseType_t *)msg);
			break;
		case SmarTIBody_PR_connectionReject:
			BodyChoise->connectionReject = *((ConnectionRejectType_t *)msg);
			break;
		case SmarTIBody_PR_connectionUpdateRequest:
			BodyChoise->connectionUpdateRequest = *((ConnectionUpdateRequestType_t *)msg);
			break;
		case SmarTIBody_PR_connectionUpdateResponse:
			BodyChoise->connectionUpdateResponse = *((ConnectionUpdateResponseType_t *)msg);
			break;
		case SmarTIBody_PR_seize:
			BodyChoise->seize = *((SeizeType_t *)msg);
			break;
		case SmarTIBody_PR_progress:
			BodyChoise->progress = *((ProgressType_t *)msg);
			break;
		case SmarTIBody_PR_answer:
			BodyChoise->answer = *((AnswerType_t *)msg);
			break;
		case SmarTIBody_PR_release:
			BodyChoise->release = *((ReleaseType_t *)msg);
			break;

		default:
			goto ext;
	}

	return SmarTIBody;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static ConnectionResponseType_t *
asn_generate_ConnectionResponse (ConnectionResponseType_t *ConnectionResponse, long *check_time)
{
	if (check_time <= 0 || !ConnectionResponse)
		goto ext;

	ConnectionResponse->updateTimeout = check_time;

	return ConnectionResponse;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_ConnectionResponse (SmarTIMessage_t *SmarTIMessage, long *check_time)
{
	if (!SmarTIMessage || !check_time || *check_time <= 0)
		goto ext;

	SmarTIBody_t SmarTIBody;
	ConnectionResponseType_t ConnectionResponse;

	memset (&SmarTIBody, 0, sizeof(SmarTIBody));
	memset (&ConnectionResponse, 0, sizeof(ConnectionResponse));

	if (asn_generate_ConnectionResponse (&ConnectionResponse, check_time) == NULL)
	{
		printf ("[%s][%d] asn_generate_ConnectionResponse: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_connectionResponse, (void *)&ConnectionResponse) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_add_ConnectionResponse: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, 42, "swSessionID", "appSessionID", SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_der_encode_ConnectionResponse (long check_time, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));

	if (!buffer)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionResponse (&SmarTIMessage, &check_time) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionResponse: fail\n", __func__, __LINE__);
		goto ext;
	}

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_ber_decode_ConnectionResponse (int *updateTimeout, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	*updateTimeout = *SmarTIMessage->body.choice.connectionResponse.updateTimeout;

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("decode: RC_OK\n");    break;
		case RC_FAIL:  printf ("decode: RC_FAIL\n");  goto ext;
		case RC_WMORE: printf ("decode: RC_WMORE\n"); goto ext;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_ber_decode_ConnectionReqest (int *updateTimeout, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(ConnectionUpdateRequestType_t));

	if (!SmarTIMessage)
		goto ext;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	*updateTimeout = *SmarTIMessage->body.choice.connectionReqest.updateTimeout;

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("decode: RC_OK\n");    break;
		case RC_FAIL:  printf ("decode: RC_FAIL\n");  goto ext;
		case RC_WMORE: printf ("decode: RC_WMORE\n"); goto ext;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

static void * /* common for ConnectionUpdateRequest/ConnectionUpdateResponse */
asn_generate_ConnectionUpdate (SmarTIBody_PR present, void *ConnectionUpdate, int set_timestamp, Timestamp_t *Timestamp)
{
	if (!ConnectionUpdate)
		goto ext;

	ConnectionUpdateRequestType_t *p_UpdateRequest = NULL;
	ConnectionUpdateResponseType_t *p_UpdateResponse = NULL;

	if (present == SmarTIBody_PR_connectionUpdateRequest)
	{
		p_UpdateRequest = (ConnectionUpdateRequestType_t *)ConnectionUpdate;
	} else
	if (present == SmarTIBody_PR_connectionUpdateResponse)
	{
		p_UpdateResponse = (ConnectionUpdateResponseType_t *)ConnectionUpdate;
	} else {
		goto ext;
	}


	if (set_timestamp)
	{
		char buffer[128] = {0};

		struct timeval tp;
		struct tm *pt;
		time_t t;

		memset (Timestamp, 0, sizeof (Timestamp_t));

		gettimeofday(&tp, NULL);
		t = (time_t)tp.tv_sec;
		pt = localtime(&t);

		sprintf(buffer, "%2d:%02d:%02d.%06d", pt->tm_hour, pt->tm_min, pt->tm_sec, (int)tp.tv_usec);

		asn_generate_OCTET_STRING (Timestamp, buffer);

		if (present == SmarTIBody_PR_connectionUpdateRequest)
			p_UpdateRequest->timestamp = Timestamp;
		else
			p_UpdateResponse->timestamp = Timestamp;
	}

	return ConnectionUpdate;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t * /* common for ConnectionUpdateRequest/ConnectionUpdateResponse */
asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR present, SmarTIMessage_t *SmarTIMessage, int set_timestamp, Timestamp_t *Timestamp)
{
	if (!SmarTIMessage)
		goto ext;

	SmarTIBody_t SmarTIBody;
	ConnectionUpdateRequestType_t ConnectionUpdateRequest;
	ConnectionUpdateResponseType_t ConnectionUpdateResponse;

	void *p_UpdateRequest = NULL;

	memset (&SmarTIBody, 0, sizeof(SmarTIBody));
	memset (&ConnectionUpdateRequest, 0, sizeof(ConnectionUpdateRequest));
	memset (&ConnectionUpdateResponse, 0, sizeof(ConnectionUpdateResponse));

	if (present == SmarTIBody_PR_connectionUpdateRequest)
	{
		p_UpdateRequest = (void *)&ConnectionUpdateRequest;
	} else
	if (present == SmarTIBody_PR_connectionUpdateResponse)
	{
		p_UpdateRequest = (void *)&ConnectionUpdateResponse;
	} else {
		goto ext;
	}

	if (asn_generate_ConnectionUpdate (present, p_UpdateRequest, set_timestamp, Timestamp) == NULL)
	{
		printf ("[%s][%d] asn_generate_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, present, p_UpdateRequest) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_add_ConnectionResponse: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, 42, "swSessionID", "appSessionID", SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_der_encode_ConnectionUpdateRequest (int set_timestamp, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;
	Timestamp_t Timestamp;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));

	if (!buffer)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR_connectionUpdateRequest, &SmarTIMessage, set_timestamp, &Timestamp) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_ber_decode_ConnectionUpdateRequest (char *timestamp, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer || !timestamp)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	Timestamp_t *Timestamp = SmarTIMessage->body.choice.connectionUpdateRequest.timestamp;

	if (!Timestamp || !Timestamp->size)
		goto ext;

	memcpy (timestamp, Timestamp->buf, Timestamp->size);

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("decode: RC_OK\n");    break;
		case RC_FAIL:  printf ("decode: RC_FAIL\n");  break;
		case RC_WMORE: printf ("decode: RC_WMORE\n"); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_der_encode_ConnectionUpdateResponse (int set_timestamp, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;
	Timestamp_t Timestamp;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));

	if (!buffer)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR_connectionUpdateResponse, &SmarTIMessage, set_timestamp, &Timestamp) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (set_timestamp)
		printf("Timestamp.buf %s\n", Timestamp.buf);

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_ber_decode_ConnectionUpdateResponse (char *timestamp, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer || !timestamp)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	Timestamp_t *Timestamp = SmarTIMessage->body.choice.connectionUpdateResponse.timestamp;

	if (!Timestamp || !Timestamp->size)
		goto ext;

	memcpy (timestamp, Timestamp->buf, Timestamp->size);

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("decode: RC_OK\n");    break;
		case RC_FAIL:  printf ("decode: RC_FAIL\n");  break;
		case RC_WMORE: printf ("decode: RC_WMORE\n"); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

static ConnectionRejectType_t *
asn_generate_ConnectionReject (ConnectionRejectType_t *ConnectionReject, int cause, OCTET_STRING_t *diagnostic)
{
	if (!ConnectionReject || !diagnostic)
		goto ext;

	ConnectionReject->cause = cause;

	memset (diagnostic, 0, sizeof (OCTET_STRING_t));

	asn_generate_OCTET_STRING (diagnostic, reject_cause_str[cause]);

	ConnectionReject->diagnostic = diagnostic;

	return ConnectionReject;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_ConnectionReject (SmarTIMessage_t *SmarTIMessage, int cause, OCTET_STRING_t *diagnostic)
{
	if (!SmarTIMessage || !diagnostic || cause < 0)
		goto ext;

	SmarTIBody_t SmarTIBody;
	ConnectionRejectType_t ConnectionReject;

	memset (&SmarTIBody, 0, sizeof(SmarTIBody));
	memset (&ConnectionReject, 0, sizeof(ConnectionReject));

	if (asn_generate_ConnectionReject (&ConnectionReject, cause, diagnostic) == NULL)
	{
		printf ("[%s][%d] asn_generate_ConnectionReject: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_connectionReject, (void *)&ConnectionReject) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_add_ConnectionResponse: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, 42, "swSessionID", "appSessionID", SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_der_encode_ConnectionReject (int cause, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;
	OCTET_STRING_t diagnostic;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));

	if (!buffer)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionReject (&SmarTIMessage, cause, &diagnostic) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}
