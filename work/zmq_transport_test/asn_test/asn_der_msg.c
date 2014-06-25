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

#define VERSION 1

char *reject_cause_str[reject_cause_max] =
{
	"prost))",
	"hate_you",
};

static char *body_type_str (SmarTIBody_PR present)
{
	switch (present)
	{
		case SmarTIBody_PR_NOTHING:                  return "PR_NOTHING";
		case SmarTIBody_PR_connectionReqest:         return "PR_connectionReqest";
		case SmarTIBody_PR_connectionResponse:       return "PR_connectionResponse";
		case SmarTIBody_PR_connectionReject:         return "PR_connectionReject";
		case SmarTIBody_PR_connectionUpdateRequest:  return "PR_connectionUpdateRequest";
		case SmarTIBody_PR_connectionUpdateResponse: return "PR_connectionUpdateResponse";
		case SmarTIBody_PR_seize:                    return "PR_seize";
		case SmarTIBody_PR_progress:                 return "PR_progress";
		case SmarTIBody_PR_answer:                   return "PR_answer";
		case SmarTIBody_PR_release:                  return "PR_release";
	}

	return "unknown";
}

static char *get_loc_time ()
{
	static char buffer[128] = {0};

	struct timeval tp;
	struct tm *pt;
	time_t t;

	gettimeofday(&tp, NULL);
	t = (time_t)tp.tv_sec;
	pt = localtime(&t);

	sprintf(buffer, "%2d:%02d:%02d.%06d", pt->tm_hour, pt->tm_min, pt->tm_sec, (int)tp.tv_usec);

	return buffer;
}

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
asn_SmarTIMessage_fill (SmarTIMessage_t *SmarTIMessage, char *swSessionID, int *appSessionID, SmarTIBody_t SmarTIBody)
{
	if (!SmarTIMessage || !swSessionID || !appSessionID)
		goto ext;

	SmarTIMessage->version = VERSION;
	asn_generate_OCTET_STRING (&SmarTIMessage->legID.swSessionID, swSessionID);
	SmarTIMessage->legID.appSessionID = *appSessionID;
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

	printf ("[%s][%d] added '%s'\n", __func__, __LINE__, body_type_str (present));

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
asn_SmarTIMessage_ConnectionResponse (SmarTIMessage_t *SmarTIMessage, char *swSID, int *appSID, long *check_time)
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
		printf ("[%s][%d] asn_SmarTIBody_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, swSID, appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
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
		asn_generate_OCTET_STRING (Timestamp, get_loc_time());

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
asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR present, SmarTIMessage_t *SmarTIMessage,
                                    char *swSID, int *appSID, int set_timestamp, Timestamp_t *Timestamp)
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
		printf ("[%s][%d] asn_SmarTIBody_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, swSID, appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static ConnectionRejectType_t *
asn_generate_ConnectionReject (ConnectionRejectType_t *ConnectionReject, int cause, OCTET_STRING_t *diagnostic)
{
	if (!ConnectionReject || !diagnostic)
		goto ext;

	ConnectionReject->cause = cause;

	asn_generate_OCTET_STRING (diagnostic, reject_cause_str[cause]);

	ConnectionReject->diagnostic = diagnostic;

	return ConnectionReject;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_ConnectionReject (SmarTIMessage_t *SmarTIMessage, char *swSID, int *appSID, int cause, OCTET_STRING_t *diagnostic)
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
		printf ("[%s][%d] asn_SmarTIBody_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, swSID, appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static ReleaseType_t *
asn_generate_Release (ReleaseType_t *Release, Cause_t *cause, Timestamp_t *timestamp,
                           BOOLEAN_t *noCDR, BOOLEAN_t *detached, OCTET_STRING_t *toLog)
{
	if (!Release || !cause || !timestamp ||
	    !noCDR || !detached || !toLog)
	{
		goto ext;
	}

	memcpy (&Release->cause, cause, cause->size);

	asn_generate_OCTET_STRING (&Release->timestamp, get_loc_time());

	Release->noCDR = noCDR;
	Release->detached = detached;
	Release->toLog = toLog;

	return Release;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_Release (SmarTIMessage_t *SmarTIMessage, char *swSID, int *appSID, Cause_t *cause,
                           Timestamp_t *timestamp, BOOLEAN_t *noCDR, BOOLEAN_t *detached, OCTET_STRING_t *toLog)
{
	if (!SmarTIMessage || !cause || !timestamp ||
	    !noCDR || !detached || !toLog)
	{
		goto ext;
	}

	SmarTIBody_t SmarTIBody;
	ReleaseType_t Release;

	memset (&SmarTIBody, 0, sizeof(SmarTIBody));
	memset (&Release, 0, sizeof(Release));

	if (asn_generate_Release (&Release, cause, timestamp, noCDR, detached, toLog) == NULL)
	{
		printf ("[%s][%d] asn_generate_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_release, (void *)&Release) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_add_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, swSID, appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_ConnectionResponse (char *swSID, int appSID, long check_time, void *buffer, int buffer_size)
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

	if (asn_SmarTIMessage_ConnectionResponse (&SmarTIMessage, swSID, &appSID, &check_time) == NULL)
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

int asn_decode_ConnectionResponse (char *swSID, int *appSID, int *updateTimeout, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer || !swSID || !appSID)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	memcpy (swSID, SmarTIMessage->legID.swSessionID.buf, SmarTIMessage->legID.swSessionID.size);
	*appSID = SmarTIMessage->legID.appSessionID;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	*updateTimeout = *SmarTIMessage->body.choice.connectionResponse.updateTimeout;

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("[%s] decode: RC_OK\n", __func__);    break;
		case RC_FAIL:  printf ("[%s] decode: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("[%s] decode: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/


int asn_decode_ConnectionReqest (char *swSID, int *appSID, int *updateTimeout, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer || !swSID || !appSID)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(ConnectionUpdateRequestType_t));

	if (!SmarTIMessage)
		goto ext;

	memcpy (swSID, SmarTIMessage->legID.swSessionID.buf, SmarTIMessage->legID.swSessionID.size);
	*appSID = SmarTIMessage->legID.appSessionID;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	*updateTimeout = *SmarTIMessage->body.choice.connectionReqest.updateTimeout;

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("[%s] decode: RC_OK\n", __func__);    break;
		case RC_FAIL:  printf ("[%s] decode: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("[%s] decode: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/
int asn_encode_ConnectionUpdateRequest (char *swSID, int appSID, int set_timestamp, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;
	Timestamp_t Timestamp;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));
	memset (&Timestamp, 0, sizeof (Timestamp_t));

	if (!buffer || !swSID)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR_connectionUpdateRequest, &SmarTIMessage, swSID, &appSID, set_timestamp, &Timestamp) == NULL)
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

int asn_decode_ConnectionUpdateRequest (char *swSID, int *appSID, char *timestamp, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer || !timestamp || !swSID || !appSID)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	memcpy (swSID, SmarTIMessage->legID.swSessionID.buf, SmarTIMessage->legID.swSessionID.size);
	*appSID = SmarTIMessage->legID.appSessionID;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	Timestamp_t *Timestamp = SmarTIMessage->body.choice.connectionUpdateRequest.timestamp;

	if (!Timestamp || !Timestamp->size)
		goto ext;

	memcpy (timestamp, Timestamp->buf, Timestamp->size);

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("[%s] decode: RC_OK\n", __func__);    break;
		case RC_FAIL:  printf ("[%s] decode: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("[%s] decode: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/


int asn_encode_ConnectionUpdateResponse (char *swSID, int appSID, int set_timestamp, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;
	Timestamp_t Timestamp;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));

	if (!buffer || !swSID)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR_connectionUpdateResponse, &SmarTIMessage, swSID, &appSID, set_timestamp, &Timestamp) == NULL)
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


int asn_decode_ConnectionUpdateResponse (char *swSID, int *appSID, char *timestamp, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;

	if (!buffer || !timestamp || !swSID || !appSID)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;


	memcpy (swSID, SmarTIMessage->legID.swSessionID.buf, SmarTIMessage->legID.swSessionID.size);
	*appSID = SmarTIMessage->legID.appSessionID;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	Timestamp_t *Timestamp = SmarTIMessage->body.choice.connectionUpdateResponse.timestamp;

	if (!Timestamp || !Timestamp->size)
		goto ext;

	memcpy (timestamp, Timestamp->buf, Timestamp->size);

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("[%s] decode: RC_OK\n", __func__);    break;
		case RC_FAIL:  printf ("[%s] decode: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("[%s] decode: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_ConnectionReject (char *swSID, int appSID, int cause, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res;
	SmarTIMessage_t SmarTIMessage;
	OCTET_STRING_t diagnostic;

	memset (&enc_res, 0, sizeof(enc_res));
	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));
	memset (&diagnostic, 0, sizeof (OCTET_STRING_t));

	if (!buffer || !swSID || !appSID)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionReject (&SmarTIMessage, swSID, &appSID, cause, &diagnostic) == NULL)
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

int asn_encode_Release (char *swSID, int appSID, char *str_cause, int noCDR, int detached, char *toLog, void *buffer, int buffer_size)
{
	asn_enc_rval_t  enc_res;
	SmarTIMessage_t SmarTIMessage;
	Cause_t         t_cause;
	Timestamp_t     t_timestamp;
	BOOLEAN_t       t_noCDR;
	BOOLEAN_t       t_detached;
	OCTET_STRING_t  t_toLog;

	if (!buffer || !str_cause || !toLog || !swSID || !appSID)
	{
		enc_res.encoded = -1;
		goto ext;
	}

	memset (&SmarTIMessage, 0, sizeof(SmarTIMessage));
	memset (&enc_res,       0, sizeof(enc_res));
	memset (&t_cause,       0, sizeof(t_cause));
	memset (&t_timestamp,   0, sizeof(t_timestamp));
	memset (&t_noCDR,       0, sizeof(t_noCDR));
	memset (&t_detached,    0, sizeof(t_detached));
	memset (&t_toLog,       0, sizeof(t_toLog));

	asn_generate_OCTET_STRING(&t_cause, str_cause);
	asn_generate_OCTET_STRING(&t_toLog, toLog);
	t_noCDR = noCDR;
	t_detached = detached;

	if (asn_SmarTIMessage_Release (&SmarTIMessage, swSID, &appSID, &t_cause, &t_timestamp, &t_noCDR, &t_detached, &t_toLog) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_decode_Release (char *swSID, int *appSID, char *str_cause, char *timestamp, int *noCDR, int *detached, char *toLog, void *buffer)
{
	asn_dec_rval_t dec_res;
	SmarTIMessage_t *SmarTIMessage = NULL;
	ReleaseType_t *Release;

	if (!buffer || !swSID || !appSID ||
	    !timestamp || !noCDR || !detached || !toLog)
	{
		dec_res.code = RC_FAIL;
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	memcpy (swSID, SmarTIMessage->legID.swSessionID.buf, SmarTIMessage->legID.swSessionID.size);
	*appSID = SmarTIMessage->legID.appSessionID;

	xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	Release = &SmarTIMessage->body.choice.release;

	memcpy (str_cause, Release->cause.buf, Release->cause.size);
	memcpy (timestamp, Release->timestamp.buf, Release->timestamp.size);

	if (Release->noCDR)
		memcpy(noCDR, Release->noCDR, sizeof (BOOLEAN_t));

	if (Release->detached)
		memcpy(detached, Release->detached, sizeof (BOOLEAN_t));

	if (Release->toLog)
		memcpy(toLog, Release->toLog->buf, Release->toLog->size);

ext:
	switch(dec_res.code)
	{
		case RC_OK:    printf ("[%s] decode: RC_OK\n", __func__);    break;
		case RC_FAIL:  printf ("[%s] decode: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("[%s] decode: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}
