#include <sys/time.h>
#include <time.h>

#include <errno.h>

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

#define xfree(ptr) if (ptr){free(ptr); ptr=NULL;}

#define VERSION 1

static int traceON = 0;

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
		case SmarTIBody_PR_connectionRequest:        return "PR_connectionRequest";
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

void asn_set_trace (int trace)
{
	traceON = trace;
}

/*------------------------------------------------------------------------------------------------------------*/

static OCTET_STRING_t *
asn_generate_OCTET_STRING (OCTET_STRING_t *octet_str, char *str)
{
	if (!str || !octet_str)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (octet_str)? "ok":"fail", (str)? "ok":"fail");

		goto ext;
	}

	OCTET_STRING_fromBuf (octet_str, str, strlen(str));

	return octet_str;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_fill (SmarTIMessage_t *SmarTIMessage, char *swSessionID, char *appSessionID, SmarTIBody_t SmarTIBody)
{
	if (!SmarTIMessage || !swSessionID || !appSessionID)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]; 3rd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (swSessionID)? "ok":"fail", (appSessionID)? "ok":"fail");

		goto ext;
	}

	SmarTIMessage->version = VERSION;
	asn_generate_OCTET_STRING (&SmarTIMessage->legID.swSessionID, swSessionID);
	asn_generate_OCTET_STRING (&SmarTIMessage->legID.appSessionID, appSessionID);
	memcpy (&SmarTIMessage->body, &SmarTIBody, sizeof (SmarTIBody_t));

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIBody_t *
asn_SmarTIBody_fill (SmarTIBody_t *SmarTIBody, SmarTIBody_PR present, void *msg)
{
	if (!SmarTIBody || present < 0 || !msg)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]; 3rd [%s];\n", __func__, __LINE__,
		       (SmarTIBody)? "ok":"fail", (present < 0)? "ok":"fail", (msg)? "ok":"fail");

		goto ext;
	}

	union SmarTIBody_u *BodyChoise;

	BodyChoise = &SmarTIBody->choice;

	SmarTIBody->present = present;

	switch (present)
	{
		case SmarTIBody_PR_NOTHING:
			break;
/*		case SmarTIBody_PR_connectionRequest:
			BodyChoise->connectionRequest = *((ConnectionRequestType_t *)msg);
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

	if (traceON >= 30)
		printf ("[%s][%d] added '%s' to message body\n", __func__, __LINE__, body_type_str (present));

	return SmarTIBody;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static ConnectionResponseType_t *
asn_generate_ConnectionResponse (ConnectionResponseType_t *ConnectionResponse, long *check_time)
{
	if (check_time <= 0 || !ConnectionResponse)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (check_time <= 0)? "ok":"fail", (ConnectionResponse)? "ok":"fail");

		goto ext;
	}

	ConnectionResponse->updateTimeout = check_time;

	return ConnectionResponse;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_ConnectionResponse (SmarTIMessage_t *SmarTIMessage, char *swSID, char *appSID, long *check_time)
{
	if (!SmarTIMessage || !check_time || !swSID || !appSID || *check_time <= 0)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]; 3rd [%s]; 4th [%s]; 5th [%s]\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (check_time)? "ok":"fail", (swSID)? "ok":"fail",
		       (appSID)? "ok":"fail", (*check_time <= 0)? "ok":"fail");

		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	ConnectionResponseType_t ConnectionResponse = {0};

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
	{
		printf("[%s][%d] arg error: 1st [fail];\n", __func__, __LINE__);
		goto ext;
	}

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
		printf("[%s][%d] arg error: unknown message type [%d]\n", __func__, __LINE__, present);
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
                                    char *swSID, char *appSID, int set_timestamp, Timestamp_t *Timestamp)
{
	if (!SmarTIMessage || !swSID || !appSID || !Timestamp)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]; 3rd [%s]; 4th [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (swSID)? "ok":"fail", (appSID)? "ok":"fail",
		       (Timestamp)? "ok":"fail");

		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	ConnectionUpdateRequestType_t ConnectionUpdateRequest = {0};
	ConnectionUpdateResponseType_t ConnectionUpdateResponse = {0};

	void *p_UpdateRequest = NULL;

	if (present == SmarTIBody_PR_connectionUpdateRequest)
	{
		p_UpdateRequest = (void *)&ConnectionUpdateRequest;
	} else
	if (present == SmarTIBody_PR_connectionUpdateResponse)
	{
		p_UpdateRequest = (void *)&ConnectionUpdateResponse;
	} else {
		printf("[%s][%d] arg error: unknown message type [%d]\n", __func__, __LINE__, present);
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
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (ConnectionReject)? "ok":"fail", (diagnostic)? "ok":"fail");

		goto ext;
	}

	ConnectionReject->cause = cause;

	asn_generate_OCTET_STRING (diagnostic, reject_cause_str[cause]);

	ConnectionReject->diagnostic = diagnostic;

	return ConnectionReject;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_ConnectionReject (SmarTIMessage_t *SmarTIMessage, char *swSID, char *appSID, int cause, OCTET_STRING_t *diagnostic)
{
	if (!SmarTIMessage || !diagnostic || !swSID || !appSID || cause < 0)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]; 3rd [%s]; 4th [%s]; 5th [%s]\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (diagnostic)? "ok":"fail", (swSID)? "ok":"fail",
		       (appSID)? "ok":"fail", (cause < 0)? "ok":"fail");

		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	ConnectionRejectType_t ConnectionReject = {0};

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
asn_generate_Release (ReleaseType_t *Release, ReleaseBasic_t *basic, ReleaseOptional_t *optional)
{
	if (!Release || !basic)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (Release)? "ok":"fail", (basic)? "ok":"fail");

		goto ext;
	}

	asn_generate_OCTET_STRING(&Release->cause, basic->cause);
	asn_generate_OCTET_STRING(&Release->timestamp, get_loc_time());

	return Release;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_Release (SmarTIMessage_t *SmarTIMessage, msg_info_t *msg_info, ReleaseBasic_t *basic, ReleaseOptional_t *optional)
{
	if (!SmarTIMessage || !basic)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (basic)? "ok":"fail");
		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	ReleaseType_t Release;

	memset (&Release, 0, sizeof (Release));

	if (asn_generate_Release (&Release, basic, optional) == NULL)
	{
		printf ("[%s][%d] asn_generate_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_release, (void *)&Release) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_add_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, msg_info->swSID, msg_info->appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_ConnectionResponse (msg_info_t *msg_info, long check_time, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};

	if (!buffer || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (msg_info)? "ok":"fail");
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionResponse (&SmarTIMessage, msg_info->swSID, msg_info->appSID, &check_time) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionResponse: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

static int asn_get_ConnectionResponse (SmarTIMessage_t *SmarTIMessage, dec_msg_t **dec_msg)
{
	if (!SmarTIMessage || !*dec_msg)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (dec_msg)? "ok":"fail");
		goto ext;
	}

	ConnectionResponseType_t *ConnectionResponse;

	ConnectionResponse = &SmarTIMessage->body.choice.connectionResponse;

	if (!ConnectionResponse->updateTimeout)
		goto ext;

	(*dec_msg)->options = (long *)calloc(1, sizeof (long));
	if (!(*dec_msg)->options)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	*(*dec_msg)->present = SmarTIBody_PR_connectionResponse;
	memcpy((*dec_msg)->options, ConnectionResponse->updateTimeout, sizeof (long));

	return 0;

ext:
	return -1;
}

/*------------------------------------------------------------------------------------------------------------*/

static void asn_clean_ConnectionResponse (dec_msg_t **dec_msg)
{
	xfree((*dec_msg)->options);
}

/*------------------------------------------------------------------------------------------------------------*/

static int asn_get_ConnectionRequest (SmarTIMessage_t *SmarTIMessage, dec_msg_t **dec_msg)
{
	if (!SmarTIMessage || !*dec_msg)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (dec_msg)? "ok":"fail");
		goto ext;
	}

	ConnectionRequestType_t *ConnectionRequest;

	ConnectionRequest = &SmarTIMessage->body.choice.connectionRequest;

	if (!ConnectionRequest->updateTimeout)
		goto ext;

	(*dec_msg)->options = (long *)calloc(1, sizeof (long));
	if (!(*dec_msg)->options)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	*(*dec_msg)->present = SmarTIBody_PR_connectionRequest;
	memcpy((*dec_msg)->options, ConnectionRequest->updateTimeout, sizeof (long));

	return 0;

ext:
	return -1;
}

/*------------------------------------------------------------------------------------------------------------*/

static void asn_clean_ConnectionRequest (dec_msg_t **dec_msg)
{
	xfree((*dec_msg)->options);
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_ConnectionUpdateRequest (msg_info_t *msg_info, int set_timestamp, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};
	Timestamp_t Timestamp = {0};

	if (!buffer || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (msg_info)? "ok":"fail");
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR_connectionUpdateRequest, &SmarTIMessage, msg_info->swSID,
	                                        msg_info->appSID, set_timestamp, &Timestamp) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

static int asn_get_ConnectionUpdateRequest (SmarTIMessage_t *SmarTIMessage, dec_msg_t **dec_msg)
{
	if (!SmarTIMessage || !*dec_msg)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (dec_msg)? "ok":"fail");
		goto ext;
	}

	ConnectionUpdateRequestType_t *ConnectionUpdateRequest;

	ConnectionUpdateRequest = &SmarTIMessage->body.choice.connectionUpdateRequest;

	if (!ConnectionUpdateRequest->timestamp)
		goto ext;

	(*dec_msg)->options = (char *)calloc(1, ConnectionUpdateRequest->timestamp->size);
	if (!(*dec_msg)->options)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	*(*dec_msg)->present = SmarTIBody_PR_connectionUpdateRequest;
	memcpy((*dec_msg)->options, ConnectionUpdateRequest->timestamp->buf, ConnectionUpdateRequest->timestamp->size);

	return 0;

ext:
	return -1;
}

/*------------------------------------------------------------------------------------------------------------*/

static void asn_clean_ConnectionUpdateRequest (dec_msg_t **dec_msg)
{
	xfree ((*dec_msg)->options);
}

int asn_encode_ConnectionUpdateResponse (msg_info_t *msg_info, int set_timestamp, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};
	Timestamp_t Timestamp = {0};

	if (!buffer || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (msg_info)? "ok":"fail");
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionUpdate (SmarTIBody_PR_connectionUpdateResponse, &SmarTIMessage, msg_info->swSID,
	                                        msg_info->appSID, set_timestamp, &Timestamp) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/


static int asn_get_ConnectionUpdateResponse (SmarTIMessage_t *SmarTIMessage, dec_msg_t **dec_msg)
{
	if (!SmarTIMessage || !*dec_msg)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (dec_msg)? "ok":"fail");
		goto ext;
	}

	ConnectionUpdateResponseType_t *ConnectionUpdateResponse;

	ConnectionUpdateResponse = &SmarTIMessage->body.choice.connectionUpdateResponse;

	if (!ConnectionUpdateResponse->timestamp)
		goto ext;

	(*dec_msg)->options = (char *)calloc(1, ConnectionUpdateResponse->timestamp->size);
	if (!(*dec_msg)->options)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	*(*dec_msg)->present = SmarTIBody_PR_connectionUpdateResponse;
	memcpy((*dec_msg)->options, ConnectionUpdateResponse->timestamp->buf, ConnectionUpdateResponse->timestamp->size);

	return 0;

ext:
	return -1;
}

/*------------------------------------------------------------------------------------------------------------*/

static void asn_clean_ConnectionUpdateResponse (dec_msg_t **dec_msg)
{
	xfree((*dec_msg)->options);
}

int asn_encode_ConnectionReject (msg_info_t *msg_info, int cause, void *buffer, int buffer_size)
{
	asn_enc_rval_t enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};
	OCTET_STRING_t diagnostic = {0};

	if (!buffer || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (msg_info)? "ok":"fail");
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_ConnectionReject (&SmarTIMessage, msg_info->swSID, msg_info->appSID, cause, &diagnostic) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_ConnectionUpdate: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_Release (msg_info_t *msg_info, ReleaseBasic_t *basic, ReleaseOptional_t *optional, void *buffer, int buffer_size)
{
	asn_enc_rval_t  enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};

	if (!buffer || !basic || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]; 3rd [%s]\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail", (msg_info)? "ok":"fail");
		enc_res.encoded = -1;
		goto ext;
	}

	if (asn_SmarTIMessage_Release (&SmarTIMessage, msg_info, basic, optional) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_decode_Release (ReleaseBasic_t *basic, ReleaseOptional_t *optional, void *buffer)
{
	asn_dec_rval_t dec_res = {0};
	SmarTIMessage_t *SmarTIMessage = NULL;
	ReleaseType_t *Release;

	if (!buffer || !basic)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail");
		dec_res.code = RC_FAIL;
		goto ext;
	}


	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	Release = &SmarTIMessage->body.choice.release;

	memcpy (basic->cause, Release->cause.buf, Release->cause.size);
	memcpy (basic->timestamp, Release->timestamp.buf, Release->timestamp.size);

	if (traceON >= 50)
	{
		printf("\n[Release] OPTIONS params:\n"
		       "noCDR    %s\n"
		       "detached %s\n"
		       "toLog    %s\n",
		       (Release->noCDR)? "yes":"no",
		       (Release->detached)? "yes":"no",
		       (Release->toLog)? "yes":"no");
	}

	// if (Release->noCDR)

	// if (Release->detached)

	// if (Release->toLog)

ext:
	switch(dec_res.code)
	{
		case RC_OK:
			if (traceON)
				printf ("%s: RC_OK\n", __func__);
			break;
		case RC_FAIL:  printf ("%s: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("%s: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_CalledPartyNumber (SeizeType_t *Seize, CdPN_t CalledPN)
{
	if (!Seize)
		return 1;

	char s_nai[16] = {0};
	char s_npi[16] = {0};

	sprintf (s_nai, "%d", CalledPN.nai);
	sprintf (s_npi, "%d", CalledPN.npi);

	asn_generate_OCTET_STRING(&Seize->cdpn.nai, s_nai);
	asn_generate_OCTET_STRING(&Seize->cdpn.npi, s_npi);
	Seize->cdpn.inn     = CalledPN.inn;

	asn_generate_OCTET_STRING(&Seize->cdpn.address, CalledPN.strCalled);

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_CallingPartyNumber (SeizeType_t *Seize, CallingPartyNumber_t *CallingPartyNumber, number_t *cgpn)
{
	if (!Seize || !CallingPartyNumber || !cgpn)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", cgpn->type);
	sprintf (s_numplan, "%d", cgpn->numplan);

	// asn_generate_OCTET_STRING(&CallingPartyNumber->nai, s_type);
	// asn_generate_OCTET_STRING(&CallingPartyNumber->npi, s_numplan);
	// CallingPartyNumber->screening = cgpn->screen;
	// CallingPartyNumber->apri      = 0; /* ! */
	// CallingPartyNumber->ni        = 0; /* ! */

	// asn_generate_OCTET_STRING(&CallingPartyNumber->address, cgpn->sym);

	// Seize->cgpn = CallingPartyNumber;

	Seize->cgpn = (CallingPartyNumber_t *)calloc(1, sizeof (CallingPartyNumber_t));
	if (!Seize->cgpn)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	asn_generate_OCTET_STRING(&Seize->cgpn->nai, s_type);
	asn_generate_OCTET_STRING(&Seize->cgpn->npi, s_numplan);
	Seize->cgpn->screening = cgpn->screen;
	Seize->cgpn->apri      = 0; /* ! */
	Seize->cgpn->ni        = 0; /* ! */

	asn_generate_OCTET_STRING(&Seize->cgpn->address, cgpn->sym);

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_CallReference (SeizeType_t *Seize, CallReference_t *CallReference, int *callRef)
{
	if (!Seize || !CallReference || !callRef)
		return 1;

	char buff[16] = {0};

	snprintf(buff, 16, "%d", *callRef);

	// asn_generate_OCTET_STRING(CallReference, buff);

	// Seize->callRef = CallReference;

	Seize->callRef = OCTET_STRING_new_fromBuf (&asn_DEF_OCTET_STRING, buff, 5);

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_CallingPartysCategory (SeizeType_t *Seize, CallingPartysCategory_t *CallingPartysCategory, int *category)
{
	if (!Seize || !CallingPartysCategory || !category)
		return 1;

	char s_category[16] = {0};
	sprintf (s_category, "%d", *category);

	// asn_generate_OCTET_STRING (CallingPartysCategory, s_category);

	// Seize->category = CallingPartysCategory;

	Seize->category = OCTET_STRING_new_fromBuf (&asn_DEF_OCTET_STRING, s_category, 1);

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_TrunkGroupId (SeizeType_t *Seize, TrunkGroupId_t *TrunkGroupId, long *TGID)
{
	if (!Seize || !TrunkGroupId || !TGID)
		return 1;

	// TrunkGroupId = TGID;

	// Seize->tgId = TrunkGroupId;

	Seize->tgId = (TrunkGroupId_t *)calloc(1, sizeof (TrunkGroupId_t));
	if (!Seize->tgId)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	*Seize->tgId = *TGID;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_OriginalCalledNumber (SeizeType_t *Seize, OriginalCalledNumber_t *OriginalCalledNumber, number_t *origNum)
{
	if (!Seize || !OriginalCalledNumber || !origNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", origNum->type);
	sprintf (s_numplan, "%d", origNum->numplan);

	// asn_generate_OCTET_STRING(&OriginalCalledNumber->nai, s_type);
	// asn_generate_OCTET_STRING(&OriginalCalledNumber->npi, s_numplan);
	// asn_generate_OCTET_STRING(&OriginalCalledNumber->address, origNum->sym);
	// OriginalCalledNumber->apri = 0; /* ! */

	// Seize->originalCDPN = OriginalCalledNumber;

	Seize->originalCDPN = (OriginalCalledNumber_t *)calloc(1, sizeof (OriginalCalledNumber_t));
	if (!Seize->originalCDPN)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	asn_generate_OCTET_STRING(&Seize->originalCDPN->nai, s_type);
	asn_generate_OCTET_STRING(&Seize->originalCDPN->npi, s_numplan);
	asn_generate_OCTET_STRING(&Seize->originalCDPN->address, origNum->sym);
	Seize->originalCDPN->apri = 0; /* ! */

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_GenericNumber (SeizeType_t *Seize, GenericNumber_t *GenericNumber, number_t *genNum)
{
	if (!Seize || !GenericNumber || !genNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", genNum->type);
	sprintf (s_numplan, "%d", genNum->numplan);

	// asn_generate_OCTET_STRING(&GenericNumber->nai, s_type);
	// asn_generate_OCTET_STRING(&GenericNumber->npi, s_numplan);
	// asn_generate_OCTET_STRING(&GenericNumber->address, genNum->sym);
	// asn_generate_OCTET_STRING(&GenericNumber->nqi, "");
	// GenericNumber->screening = genNum->screen;
	// GenericNumber->apri      = 0; /* ! */
	// GenericNumber->ni        = 0; /* ! */

	// Seize->genericNumber = GenericNumber;

	Seize->genericNumber = (GenericNumber_t *)calloc(1, sizeof (GenericNumber_t));
	if (!Seize->genericNumber)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	asn_generate_OCTET_STRING(&Seize->genericNumber->nai, s_type);
	asn_generate_OCTET_STRING(&Seize->genericNumber->npi, s_numplan);
	asn_generate_OCTET_STRING(&Seize->genericNumber->address, genNum->sym);
	asn_generate_OCTET_STRING(&Seize->genericNumber->nqi, "");
	Seize->genericNumber->screening = genNum->screen;
	Seize->genericNumber->apri      = 0; /* ! */
	Seize->genericNumber->ni        = 0; /* ! */

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_RedirectingNumber (SeizeType_t *Seize, RedirectingNumber_t *RedirectingNumber, number_t *redirNum)
{
	if (!Seize || !RedirectingNumber || !redirNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", redirNum->type);
	sprintf (s_numplan, "%d", redirNum->numplan);

	// asn_generate_OCTET_STRING(&RedirectingNumber->nai, s_type);
	// asn_generate_OCTET_STRING(&RedirectingNumber->npi, s_numplan);
	// asn_generate_OCTET_STRING(&RedirectingNumber->address, redirNum->sym);
	// RedirectingNumber->apri = 0; /* ! */

	// Seize->redirectingNumber = RedirectingNumber;

	Seize->redirectingNumber = (RedirectingNumber_t *)calloc(1, sizeof (RedirectingNumber_t));
	if (!Seize->redirectingNumber)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	asn_generate_OCTET_STRING(&Seize->redirectingNumber->nai, s_type);
	asn_generate_OCTET_STRING(&Seize->redirectingNumber->npi, s_numplan);
	asn_generate_OCTET_STRING(&Seize->redirectingNumber->address, redirNum->sym);
	Seize->redirectingNumber->apri = 0; /* ! */

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_RedirectionInformation (SeizeType_t *Seize, RedirectionInformation_t *RedirectionInformation, RedirInfo_t *info)
{
	if (!Seize || !RedirectionInformation || !info)
		return 1;

	// RedirectionInformation->redirecting  = info->redirecting;
	// RedirectionInformation->oreason      = info->oreason;
	// RedirectionInformation->reason       = info->reason;

	// RedirectionInformation->counter.buf = (uint8_t *)&info->counter;
	// RedirectionInformation->counter.size = sizeof(info->counter);

	// Seize->redirectionInformation = RedirectionInformation;

	Seize->redirectionInformation = (RedirectionInformation_t *)calloc(1, sizeof (RedirectionInformation_t));
	if (!Seize->redirectionInformation)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	Seize->redirectionInformation->redirecting  = info->redirecting;
	Seize->redirectionInformation->oreason      = info->oreason;
	Seize->redirectionInformation->reason       = info->reason;

	uint8_t *buffer = Seize->redirectionInformation->counter.buf;
	int size = sizeof(info->counter);

	Seize->redirectionInformation->counter.buf = (uint8_t *)malloc(sizeof (uint8_t) * 3 + 1);
	if (!Seize->redirectionInformation->counter.buf)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		return 1;
	}

	memcpy(Seize->redirectionInformation->counter.buf, &info->counter, size);
	Seize->redirectionInformation->counter.size = size;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_ForwardCallIndicators (SeizeType_t *Seize, ForwardCallIndicators_t *ForwardCallIndicators, char *fci)
{
	if (!Seize || !ForwardCallIndicators || !fci)
		return 1;

	// asn_generate_OCTET_STRING(ForwardCallIndicators, fci);

	// Seize->fci = ForwardCallIndicators;

	Seize->fci = OCTET_STRING_new_fromBuf (&asn_DEF_OCTET_STRING, fci, 2);

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_UserServiceInformation (SeizeType_t *Seize, UserServiceInformation_t *UserServiceInformation, char *usi)
{
	if (!Seize || !UserServiceInformation || !usi)
		return 1;

	// asn_generate_OCTET_STRING(UserServiceInformation, usi);

	// Seize->usi = UserServiceInformation;

	Seize->usi = OCTET_STRING_new_fromBuf (&asn_DEF_OCTET_STRING, usi, strlen(usi));

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_UserTeleserviceInformation (SeizeType_t *Seize, UserTeleserviceInformation_t *UserTeleserviceInformation, char *uti)
{
	if (!Seize || !UserTeleserviceInformation || !uti)
		return 1;

	// asn_generate_OCTET_STRING(UserTeleserviceInformation, uti);

	// Seize->uti = UserTeleserviceInformation;

	Seize->uti = OCTET_STRING_new_fromBuf (&asn_DEF_OCTET_STRING, uti, strlen(uti));

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_seize_add_TransmissionMediumRequirement (SeizeType_t *Seize, TransmissionMediumRequirement_t *TransmissionMediumRequirement, char *tmr)
{
	if (!Seize || !TransmissionMediumRequirement || !tmr)
		return 1;

	// asn_generate_OCTET_STRING(TransmissionMediumRequirement, tmr);

	// Seize->tmr = TransmissionMediumRequirement;

	Seize->tmr = OCTET_STRING_new_fromBuf (&asn_DEF_OCTET_STRING, tmr, 2);

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_Seize (SmarTIMessage_t *SmarTIMessage, msg_info_t *msg_info, SeizeBasic_t *basic,
                         SeizeOptional_t *optional, CallingPartyNumber_t *CallingPartyNumber,
                         CallReference_t *CallReference, CallingPartysCategory_t *CallingPartysCategory,
                         TrunkGroupId_t *TrunkGroupId, OriginalCalledNumber_t *OriginalCalledNumber,
                         GenericNumber_t *GenericNumber, RedirectingNumber_t *RedirectingNumber,
                         RedirectionInformation_t *RedirectionInformation, ForwardCallIndicators_t *ForwardCallIndicators,
                         UserServiceInformation_t *UserServiceInformation, UserTeleserviceInformation_t *UserTeleserviceInformation,
		                 TransmissionMediumRequirement_t *TransmissionMediumRequirement)
{
	if (!SmarTIMessage)
	{
		printf("[%s][%d] arg error: 1st [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail");
		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	SeizeType_t Seize;

	memset (&Seize, 0, sizeof(Seize));

	asn_generate_OCTET_STRING(&Seize.vatsId, basic->vatsId);
	asn_generate_OCTET_STRING(&Seize.applicationId, basic->applicationId);
	asn_generate_OCTET_STRING(&Seize.timestamp, get_loc_time());
	if (asn_seize_add_CalledPartyNumber (&Seize, basic->CalledPN))
	{
		printf("[%s][%d] addition CalledPartyNumber failed\n", __func__, __LINE__);
		goto ext;
	}

	if (optional)
	{
		asn_seize_add_CallingPartyNumber     (&Seize, CallingPartyNumber, optional->cgpn);
		asn_seize_add_CallReference          (&Seize, CallReference, optional->callRef);
		asn_seize_add_CallingPartysCategory  (&Seize, CallingPartysCategory, optional->category);
		asn_seize_add_TrunkGroupId           (&Seize, TrunkGroupId, optional->TGID);
		asn_seize_add_OriginalCalledNumber   (&Seize, OriginalCalledNumber, optional->origNum);
		asn_seize_add_GenericNumber          (&Seize, GenericNumber, optional->genNum);
		asn_seize_add_RedirectingNumber      (&Seize, RedirectingNumber, optional->redirNum);
		asn_seize_add_RedirectionInformation (&Seize, RedirectionInformation, optional->info);

		asn_seize_add_ForwardCallIndicators         (&Seize, ForwardCallIndicators, optional->fci);
		asn_seize_add_UserServiceInformation        (&Seize, UserServiceInformation, optional->usi);
		asn_seize_add_UserTeleserviceInformation    (&Seize, UserTeleserviceInformation, optional->uti);
		asn_seize_add_TransmissionMediumRequirement (&Seize, TransmissionMediumRequirement, optional->tmr);
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_seize, (void *)&Seize) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, msg_info->swSID, msg_info->appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_Seize (msg_info_t *msg_info, SeizeBasic_t *basic, SeizeOptional_t *optional, void *buffer, int buffer_size)
{
	asn_enc_rval_t  enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};

	CallingPartyNumber_t     CallingPartyNumber;
	CallReference_t          CallReference = {0};
	CallingPartysCategory_t  CallingPartysCategory = {0};
	TrunkGroupId_t           TrunkGroupId = {0};
	OriginalCalledNumber_t   OriginalCalledNumber;
	GenericNumber_t          GenericNumber;
	RedirectingNumber_t      RedirectingNumber;
	RedirectionInformation_t RedirectionInformation = {0};

	ForwardCallIndicators_t         ForwardCallIndicators = {0};
	UserServiceInformation_t        UserServiceInformation = {0};
	UserTeleserviceInformation_t    UserTeleserviceInformation = {0};
	TransmissionMediumRequirement_t TransmissionMediumRequirement = {0};

	enc_res.encoded = -1;

	if (!buffer || !basic || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s] 3rd [%s]\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail", (msg_info)? "ok":"fail");
		goto ext;
	}

	memset(&CallingPartyNumber, 0, sizeof(CallingPartyNumber));
	memset(&OriginalCalledNumber, 0, sizeof(OriginalCalledNumber));
	memset(&GenericNumber, 0, sizeof(GenericNumber));
	memset(&RedirectingNumber, 0, sizeof(RedirectingNumber));

	/* check number format */

	if (asn_SmarTIMessage_Seize (&SmarTIMessage, msg_info, basic, optional, &CallingPartyNumber,
		                         &CallReference, &CallingPartysCategory, &TrunkGroupId, &OriginalCalledNumber,
		                         &GenericNumber, &RedirectingNumber, &RedirectionInformation,
		                         &ForwardCallIndicators, &UserServiceInformation, &UserTeleserviceInformation,
		                         &TransmissionMediumRequirement) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_Seize: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

	ASN_STRUCT_FREE_CONTENTS_ONLY(asn_DEF_SmarTIMessage, &SmarTIMessage);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

static int asn_get_Seize (SmarTIMessage_t *SmarTIMessage, dec_msg_t **dec_msg)
{
	if (!SmarTIMessage || !*dec_msg)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (dec_msg)? "ok":"fail");
		goto ext;
	}

	SeizeBasic_t basic = {0};
	SeizeOptional_t *optional;
	SeizeType_t *Seize;

	Seize = &SmarTIMessage->body.choice.seize;

	*(*dec_msg)->present = SmarTIBody_PR_seize;

	(*dec_msg)->base = (SeizeBasic_t *)calloc(1, sizeof (SeizeBasic_t));
	if (!(*dec_msg)->base)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	(*dec_msg)->options = (SeizeOptional_t *)calloc(1, sizeof (SeizeOptional_t));
	if (!(*dec_msg)->options)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	memcpy (basic.vatsId, Seize->vatsId.buf, Seize->vatsId.size);
	memcpy (basic.applicationId, Seize->applicationId.buf, Seize->applicationId.size);
	memcpy (basic.timestamp, Seize->timestamp.buf, Seize->timestamp.size);

	sscanf ((char *)Seize->cdpn.nai.buf, "%"SCNd8, &basic.CalledPN.nai);
	sscanf ((char *)Seize->cdpn.npi.buf, "%"SCNd8, &basic.CalledPN.npi);
	basic.CalledPN.inn = Seize->cdpn.inn;
	memcpy (basic.CalledPN.strCalled, Seize->cdpn.address.buf, Seize->cdpn.address.size);

	memcpy ((*dec_msg)->base, &basic, sizeof (SeizeBasic_t));

	optional = (*dec_msg)->options;

	if (traceON >= 50)
	{
		printf("\n[Seize] OPTIONS params:\n"
		       "cgpn:                   %s\n"
		       "callRef:                %s\n"
		       "category:               %s\n"
		       "tgId:                   %s\n"
		       "originalCDPN:           %s\n"
		       "genericNumber:          %s\n"
		       "redirectingNumber:      %s\n"
		       "redirectionInformation: %s\n"
		       "fci                     %s\n"
		       "usi                     %s\n"
		       "uti                     %s\n"
		       "tmr                     %s\n"
		       "noCDR                   %s\n"
		       "bridge                  %s\n"
		       "detached                %s\n"
		       "toLog                   %s\n",
		       (Seize->cgpn)? "yes":"no",
		       (Seize->callRef)? "yes":"no",
		       (Seize->category)? "yes":"no",
		       (Seize->tgId)? "yes":"no",
		       (Seize->originalCDPN)? "yes":"no",
		       (Seize->genericNumber)? "yes":"no",
		       (Seize->redirectingNumber)? "yes":"no",
		       (Seize->redirectionInformation)? "yes":"no",
		       (Seize->fci)? "yes":"no",
		       (Seize->usi)? "yes":"no",
		       (Seize->uti)? "yes":"no",
		       (Seize->tmr)? "yes":"no",
		       (Seize->noCDR)? "yes":"no",
		       (Seize->bridge)? "yes":"no",
		       (Seize->detached)? "yes":"no",
		       (Seize->toLog)? "yes":"no");
	}

	if (Seize->cgpn)
	{
		optional->cgpn = (number_t *)calloc(1, sizeof (number_t));
		if (!optional->cgpn)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		sscanf ((char *)Seize->cgpn->nai.buf, "%"SCNd8, &optional->cgpn->type);
		sscanf ((char *)Seize->cgpn->npi.buf, "%"SCNd8, &optional->cgpn->numplan);

		memcpy(optional->cgpn->sym, Seize->cgpn->address.buf, Seize->cgpn->address.size);

		optional->cgpn->present = Seize->cgpn->apri;
		optional->cgpn->pfx = Seize->cgpn->ni;
		optional->cgpn->screen = Seize->cgpn->screening;
	}

	if (Seize->callRef)
	{
		optional->callRef = (int *)calloc(1, sizeof (int));
		if (!optional->callRef)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		sscanf ((char *)Seize->callRef->buf, "%d", optional->callRef);
	}

	if (Seize->category)
	{
		optional->category = (int *)calloc(1, sizeof (int));
		if (!optional->category)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		sscanf ((char *)Seize->category->buf, "%d", optional->category);
	}

	if (Seize->tgId)
	{
		optional->TGID = (long *)calloc(1, sizeof (long));
		if (!optional->TGID)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		memcpy(optional->TGID, Seize->tgId, sizeof *optional->TGID);
	}

	if (Seize->originalCDPN)
	{
		optional->origNum = (number_t *)calloc(1, sizeof (number_t));
		if (!optional->origNum)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		sscanf ((char *)Seize->originalCDPN->nai.buf, "%"SCNd8, &optional->origNum->type);
		sscanf ((char *)Seize->originalCDPN->npi.buf, "%"SCNd8, &optional->origNum->numplan);

		memcpy(optional->origNum->sym, Seize->originalCDPN->address.buf, Seize->originalCDPN->address.size);

		optional->origNum->present = Seize->originalCDPN->apri;
	}

	if (Seize->genericNumber)
	{
		optional->genNum = (number_t *)calloc(1, sizeof (number_t));
		if (!optional->genNum)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		sscanf ((char *)Seize->genericNumber->nai.buf, "%"SCNd8, &optional->genNum->type);
		sscanf ((char *)Seize->genericNumber->npi.buf, "%"SCNd8, &optional->genNum->numplan);

		memcpy(optional->genNum->sym, Seize->genericNumber->address.buf, Seize->genericNumber->address.size);
		//memcpy(optional->genNum->sym, Seize->genericNumber->nqi.buf, Seize->genericNumber->nqi.size); /* ? */

		optional->genNum->present = Seize->genericNumber->apri;
		optional->genNum->pfx = Seize->genericNumber->ni;
		optional->genNum->screen = Seize->genericNumber->screening;
	}

	if (Seize->redirectingNumber)
	{
		optional->redirNum = (number_t *)calloc(1, sizeof (number_t));
		if (!optional->redirNum)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		sscanf ((char *)Seize->redirectingNumber->nai.buf, "%"SCNd8, &optional->redirNum->type);
		sscanf ((char *)Seize->redirectingNumber->npi.buf, "%"SCNd8, &optional->redirNum->numplan);

		memcpy(optional->redirNum->sym, Seize->redirectingNumber->address.buf, Seize->redirectingNumber->address.size);

		optional->redirNum->present = Seize->redirectingNumber->apri;
	}

	if (Seize->redirectionInformation)
	{
		optional->info = (RedirInfo_t *)calloc(1, sizeof (RedirInfo_t));
		if (!optional->info)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}

		optional->info->redirecting = Seize->redirectionInformation->redirecting;
		optional->info->oreason = Seize->redirectionInformation->oreason;
		memcpy (&optional->info->counter, Seize->redirectionInformation->counter.buf, Seize->redirectionInformation->counter.size);
		optional->info->reason = Seize->redirectionInformation->reason;
	}

	if (Seize->fci)
	{
		optional->fci = (char *)calloc(1, Seize->fci->size);
		if (!optional->fci)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}
		memcpy(optional->fci, Seize->fci->buf, Seize->fci->size);
	}

	if (Seize->usi)
	{
		optional->usi = (char *)calloc(1, Seize->usi->size);
		if (!optional->usi)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}
		memcpy(optional->usi, Seize->usi->buf, Seize->usi->size);
	}

	if (Seize->uti)
	{
		optional->uti = (char *)calloc(1, Seize->uti->size);
		if (!optional->uti)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}
		memcpy(optional->uti, Seize->uti->buf, Seize->uti->size);
	}

	if (Seize->tmr)
	{
		optional->tmr = (char *)calloc(1, Seize->tmr->size);
		if (!optional->tmr)
		{
			printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
			goto ext;
		}
		memcpy(optional->tmr, Seize->tmr->buf, Seize->tmr->size);
	}

/*	if (Seize->noCDR)
	{

	}

	if (Seize->bridge)
	{

	}

	if (Seize->detached)
	{

	}

	if (Seize->toLog)
	{

	}*/

	return 0;

ext:
	return -1;
}

/*------------------------------------------------------------------------------------------------------------*/

static void asn_clean_Seize (dec_msg_t **dec_msg)
{
	SeizeBasic_t *basic;
	SeizeOptional_t *optional;

	basic = (SeizeBasic_t *)((*dec_msg)->base);
	optional = (SeizeOptional_t *)((*dec_msg)->options);

	xfree(optional->cgpn)
	xfree(optional->callRef)
	xfree(optional->category)
	xfree(optional->TGID)

	xfree(optional->fci)
	xfree(optional->usi)
	xfree(optional->uti)
	xfree(optional->tmr)

	xfree(optional->origNum)
	xfree(optional->genNum)
	xfree(optional->redirNum)
	xfree(optional->info)

	xfree(optional->noCDR)
	xfree(optional->bridge)
	xfree(optional->detached)
	xfree(optional->toLog)

	xfree(optional);

	xfree(basic);
}

static int
asn_progress_add_Cause (ProgressType_t *Progress, Cause_t *Cause, char *cause)
{
	if (!Progress || !Cause || !cause)
	return 1;

	asn_generate_OCTET_STRING(Cause, cause);

	Progress->cause = Cause;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_OptionalBackwardCallInidicators (ProgressType_t *Progress,
                                                  OptionalBackwardCallInidicators_t *OptionalBackwardCallInidicators, char *obci)
{
	if (!Progress || !OptionalBackwardCallInidicators || !obci)
		return 1;

	asn_generate_OCTET_STRING(OptionalBackwardCallInidicators, obci);

	Progress->obci = OptionalBackwardCallInidicators;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

// static int
// asn_progress_add_GenericNotificationIndicatorList (ProgressType_t *Progress,
//                                                    GenericNotificationIndicatorList_t*GenericNotificationIndicatorList, number_t *redirNumber)
// {
// 	if (!Progress || !)
// 		return 1;


// 	return 0;
// }

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_RedirectionNumber (ProgressType_t *Progress, RedirectionNumber_t *RedirectionNumber, number_t *redirNumber)
{
	if (!Progress || !RedirectionNumber || !redirNumber)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", redirNumber->type);
	sprintf (s_numplan, "%d", redirNumber->numplan);

	asn_generate_OCTET_STRING(&RedirectionNumber->nai, s_type);
	asn_generate_OCTET_STRING(&RedirectionNumber->npi, s_numplan);
	asn_generate_OCTET_STRING(&RedirectionNumber->address, redirNumber->sym);

	//RedirectionNumber->inn = redirNumber->/*?*/

	Progress->redirectionNumber = RedirectionNumber;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_RedirectionNumberRestriction (ProgressType_t *Progress, RedirectionNumberRestriction_t *RedirectionNumberRestriction,
                                               uint8_t *redirRestInd)
{
	if (!Progress || !RedirectionNumberRestriction || !redirRestInd)
		return 1;

	*redirRestInd = !!(*redirRestInd);

	memcpy (RedirectionNumberRestriction, redirRestInd, sizeof *redirRestInd);

	Progress->redirectionRestInd = RedirectionNumberRestriction;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_CallDiversionInformation (ProgressType_t *Progress, CallDiversionInformation_t *CallDiversionInformation,
                                           uint8_t *NotifSubscOptions, uint8_t *RedirReason)
{
	if (!Progress || !CallDiversionInformation || !NotifSubscOptions || !RedirReason)
		return 1;

	*NotifSubscOptions = (*NotifSubscOptions) & 3;
	*RedirReason = (*RedirReason) & 0xF;

	memcpy (&CallDiversionInformation->nso, NotifSubscOptions, sizeof (NotifSubscOptions));
	memcpy (&CallDiversionInformation->redirectingReason, RedirReason, sizeof (RedirReason));

	Progress->callDiversion = CallDiversionInformation;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_CallTransferNumber (ProgressType_t *Progress, CallTransferNumber_t *CallTransferNumber, number_t *callTransNum)
{
	if (!Progress || !CallTransferNumber || !callTransNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", callTransNum->type);
	sprintf (s_numplan, "%d", callTransNum->numplan);

	asn_generate_OCTET_STRING(&CallTransferNumber->nai, s_type);
	asn_generate_OCTET_STRING(&CallTransferNumber->npi, s_numplan);
	CallTransferNumber->screening = callTransNum->screen;
	CallTransferNumber->apri      = 0; /* ! */

	asn_generate_OCTET_STRING(&CallTransferNumber->address, callTransNum->sym);

	Progress->callTransferNumber = CallTransferNumber;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_CollectedInfo (ProgressType_t *Progress, CollectedInfo_t *CollectedInfo, uint8_t *col_toneInfo, char *col_signal)
{
	if (!Progress || !CollectedInfo || !col_toneInfo || !col_signal)
		return 1;

	if (col_toneInfo)
	{
		CollectedInfo->present = CollectedInfo_PR_tone;
		CollectedInfo->choice.tone = *col_toneInfo;
	} else
	if (col_signal)
	{
		CollectedInfo->present = CollectedInfo_PR_signal;
		asn_generate_OCTET_STRING(&CollectedInfo->choice.signal, col_signal);
	}

	Progress->collectedInfo = CollectedInfo;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static int
asn_progress_add_PlayInfo (ProgressType_t *Progress, PlayInfo_t *PlayInfo, uint8_t *pla_toneInfo, char *pla_file)
{
	if (!Progress || !PlayInfo || !pla_toneInfo || !pla_file)
		return 1;

	if (pla_toneInfo)
	{
		PlayInfo->tone = *pla_toneInfo;
		/*ASN_SET_ISPRESENT(PlayInfo, PlayInfo_PR_tone);*/
	} else
	if (pla_file)
	{
		asn_generate_OCTET_STRING(&PlayInfo->file, pla_file);
		/*ASN_SET_ISPRESENT(PlayInfo, PlayInfo_PR_file);*/
	}

	if (pla_toneInfo || pla_file)
		Progress->play = PlayInfo;

	return 0;
}

/*------------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_Progress (SmarTIMessage_t *SmarTIMessage, msg_info_t *msg_info, ProgressBasic_t *basic,
                            ProgressOptional_t *optional, Cause_t *cause, OptionalBackwardCallInidicators_t *obci,
                            /*GenericNotificationIndicatorList_t *gnotification,*/ RedirectionNumber_t *redirectionNumber,
                            RedirectionNumberRestriction_t *redirectionRestInd, CallDiversionInformation_t *callDiversion,
                            CallTransferNumber_t *callTransferNumber, CollectedInfo_t *collectedInfo, PlayInfo_t *play)
{
	if (!SmarTIMessage)
	{
		printf("[%s][%d] arg error: 1st [%s];\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail");
		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	ProgressType_t Progress;

	memset (&Progress, 0, sizeof(Progress));

	Progress.event.event.buf = (uint8_t *)&basic->e_Ind;
	Progress.event.event.size = 3;
	Progress.event.presentation = basic->e_Pres;

	asn_generate_OCTET_STRING(&Progress.timestamp, get_loc_time());

	if (optional)
	{
		asn_progress_add_Cause                            (&Progress, cause, optional->cause);
		asn_progress_add_OptionalBackwardCallInidicators  (&Progress, obci, optional->obci);
		//asn_progress_add_GenericNotificationIndicatorList (&Progress, gnotification, optional->);
		asn_progress_add_RedirectionNumber                (&Progress, redirectionNumber, optional->redirNumber);
		asn_progress_add_RedirectionNumberRestriction     (&Progress, redirectionRestInd, optional->redirRestInd);
		asn_progress_add_CallDiversionInformation         (&Progress, callDiversion, optional->NotifSubscOptions, optional->RedirReason);
		asn_progress_add_CallTransferNumber               (&Progress, callTransferNumber, optional->callTransNum);
		asn_progress_add_CollectedInfo                    (&Progress, collectedInfo, optional->col_toneInfo, optional->col_signal);
		asn_progress_add_PlayInfo                         (&Progress, play, optional->pla_toneInfo, optional->pla_file);
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_progress, (void *)&Progress) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, msg_info->swSID, msg_info->appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_encode_Progress (msg_info_t *msg_info, ProgressBasic_t *basic, ProgressOptional_t *optional, void *buffer, int buffer_size)
{
	asn_enc_rval_t  enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};

	Cause_t                            cause = {0};
	OptionalBackwardCallInidicators_t  obci = {0};
	/*GenericNotificationIndicatorList_t gnotification = {0};*/
	RedirectionNumber_t                redirectionNumber;
	RedirectionNumberRestriction_t     redirectionRestInd = {0};
	CallDiversionInformation_t         callDiversion = {0};
	CallTransferNumber_t               callTransferNumber;
	CollectedInfo_t                    collectedInfo = {0};
	PlayInfo_t                         play = {0};

	enc_res.encoded = -1;

	if (!buffer || !basic || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s] 3rd [%s]\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail", (msg_info)? "ok":"fail");
		goto ext;
	}


	memset (&redirectionNumber, 0, sizeof (redirectionNumber));
	memset (&callTransferNumber, 0, sizeof (callTransferNumber));

	if (asn_SmarTIMessage_Progress (&SmarTIMessage, msg_info, basic, optional, &cause, &obci, /*&gnotification,*/
	                                &redirectionNumber, &redirectionRestInd, &callDiversion, &callTransferNumber,
	                                &collectedInfo, &play) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_Progress: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_decode_Progress (ProgressBasic_t *basic, ProgressOptional_t *optional, void *buffer)
{
	asn_dec_rval_t dec_res = {0};
	SmarTIMessage_t *SmarTIMessage = NULL;
	ProgressType_t *Progress;

	if (!buffer || !basic)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail");
		dec_res.code = RC_FAIL;
		goto ext;
	}


	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	Progress = &SmarTIMessage->body.choice.progress;

	memcpy (basic->timestamp, Progress->timestamp.buf, Progress->timestamp.size);
	memcpy (&basic->e_Ind, Progress->event.event.buf, Progress->event.event.size);
	basic->e_Pres = Progress->event.presentation;

	if (!optional)
		goto ext;

	if (traceON >= 50)
	{
		printf("\n[Progress] OPTIONS params:\n"
		       "cause              %s\n"
		       "obci               %s\n"
		       "gnotification      %s\n"
		       "redirectionNumber  %s\n"
		       "redirectionRestInd %s\n"
		       "callDiversion      %s\n"
		       "callTransferNumber %s\n"
		       "collectedInfo      %s\n"
		       "play               %s\n"
		       "noCDR              %s\n"
		       "bridge             %s\n"
		       "detached           %s\n"
		       "toLog              %s\n"
		       "other              %s\n",
		       (Progress->cause)? "yes":"no",
		       (Progress->obci)? "yes":"no",
		       (Progress->gnotification )? "yes":"no",
		       (Progress->redirectionNumber)? "yes":"no",
		       (Progress->redirectionRestInd)? "yes":"no",
		       (Progress->callDiversion)? "yes":"no",
		       (Progress->callTransferNumber)? "yes":"no",
		       (Progress->collectedInfo)? "yes":"no",
		       (Progress->play)? "yes":"no",
		       (Progress->noCDR)? "yes":"no",
		       (Progress->bridge)? "yes":"no",
		       (Progress->detached)? "yes":"no",
		       (Progress->toLog)? "yes":"no",
		       (Progress->other)? "yes":"no");
	}

	if (Progress->cause)
		memcpy (optional->cause, Progress->cause->buf, Progress->cause->size);

	if (Progress->obci)
		memcpy (optional->obci, Progress->obci->buf, Progress->obci->size);

	// if (Progress->gnotification)
	// {

	// }

	if (Progress->redirectionNumber)
	{
		sscanf ((char *)Progress->redirectionNumber->nai.buf, "%"SCNd8, &optional->redirNumber->type);
		sscanf ((char *)Progress->redirectionNumber->npi.buf, "%"SCNd8, &optional->redirNumber->numplan);

		memcpy(optional->redirNumber->sym, Progress->redirectionNumber->address.buf, Progress->redirectionNumber->address.size);
	}

	if (Progress->redirectionRestInd)
	{
		*optional->redirRestInd = *Progress->redirectionRestInd;
	}

	if (Progress->callDiversion)
	{
		*optional->NotifSubscOptions = Progress->callDiversion->nso;
		*optional->RedirReason = Progress->callDiversion->redirectingReason;
	}

	if (Progress->callTransferNumber)
	{
		sscanf ((char *)Progress->callTransferNumber->nai.buf, "%"SCNd8, &optional->callTransNum->type);
		sscanf ((char *)Progress->callTransferNumber->npi.buf, "%"SCNd8, &optional->callTransNum->numplan);

		memcpy(optional->callTransNum->sym, Progress->callTransferNumber->address.buf, Progress->callTransferNumber->address.size);

		optional->callTransNum->present = Progress->callTransferNumber->apri;
		optional->callTransNum->screen = Progress->callTransferNumber->screening;
	}

	if (Progress->collectedInfo)
	{
		if (Progress->collectedInfo->present == CollectedInfo_PR_tone)
		{
			*optional->col_toneInfo = Progress->collectedInfo->choice.tone;
		} else
		if (Progress->collectedInfo->present == CollectedInfo_PR_signal)
		{
			memcpy(optional->col_signal, Progress->collectedInfo->choice.signal.buf, Progress->collectedInfo->choice.signal.size);
		}
	}

	if (Progress->play)
	{
		// if (Progress->collectedInfo->present == CollectedInfo_PR_tone)
		// {
		// 	*optional->col_toneInfo = Progress->collectedInfo->choice.tone;
		// } else
		// if (Progress->collectedInfo->present == CollectedInfo_PR_signal)
		// {
		// 	memcpy(optional->col_signal, Progress->collectedInfo->choice.signal.buf, Progress->collectedInfo->choice.signal.size);
		// }
	}

	// if (Progress->noCDR)
	// {

	// }

	// if (Progress->bridge)
	// {

	// }

	// if (Progress->detached)
	// {

	// }

	// if (Progress->toLog)
	// {

	// }

	// if (Progress->other)
	// {

	// }


ext:
	switch(dec_res.code)
	{
		case RC_OK:
			if (traceON)
				printf ("%s: RC_OK\n", __func__);
			break;
		case RC_FAIL:  printf ("%s: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("%s: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_BackwardCallIndicators (AnswerType_t *Answer, BackwardCallIndicators_t *BCI, char *bci)
{
	if (!Answer || !BCI || !bci)
		return 1;

	asn_generate_OCTET_STRING(BCI, bci);

	Answer->bci = BCI;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_OptionalBackwardCallInidicators (AnswerType_t *Answer, OptionalBackwardCallInidicators_t *OBCI, char *obci)
{
	if (!Answer || !OBCI || !obci)
		return 1;

	asn_generate_OCTET_STRING(OBCI, obci);

	Answer->obci = OBCI;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

// static int 
// asn_answer_add_GenericNotificationIndicatorList (AnswerType_t *Answer, GenericNotificationIndicatorList_t *GNIL, )
// {
// 	if (!Answer || !GNIL || !)
// 		return 1;

// 	return 0;
// }

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_ConnectedNumber (AnswerType_t *Answer, ConnectedNumber_t *CN, number_t *connNum)
{
	if (!Answer || !CN || !connNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", connNum->type);
	sprintf (s_numplan, "%d", connNum->numplan);

	asn_generate_OCTET_STRING(&CN->nai, s_type);
	asn_generate_OCTET_STRING(&CN->npi, s_numplan);
	asn_generate_OCTET_STRING(&CN->address, connNum->sym);
	CN->screening = connNum->screen;
	CN->apri      = 0; /* ! */

	Answer->connectedNumber = CN;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_GenericNumber (AnswerType_t *Answer, GenericNumber_t *GN, number_t *genNum)
{
	if (!Answer || !GN || !genNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", genNum->type);
	sprintf (s_numplan, "%d", genNum->numplan);

	asn_generate_OCTET_STRING(&GN->nai, s_type);
	asn_generate_OCTET_STRING(&GN->npi, s_numplan);
	asn_generate_OCTET_STRING(&GN->address, genNum->sym);
	asn_generate_OCTET_STRING(&GN->nqi, "");
	GN->screening = genNum->screen;
	GN->apri      = 0; /* ! */
	GN->ni        = 0; /* ! */

	Answer->genericNumber = GN;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_RedirectionNumber (AnswerType_t *Answer, RedirectionNumber_t *RN, number_t *redirNum)
{
	if (!Answer || !RN || !redirNum)
		return 1;

	char s_type[16] = {0};
	char s_numplan[16] = {0};

	sprintf (s_type, "%d", redirNum->type);
	sprintf (s_numplan, "%d", redirNum->numplan);

	asn_generate_OCTET_STRING(&RN->nai, s_type);
	asn_generate_OCTET_STRING(&RN->npi, s_numplan);
	asn_generate_OCTET_STRING(&RN->address, redirNum->sym);

	//RN->inn = redirNumber->/*?*/

	Answer->redirectionNumber = RN;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_RedirectionNumberRestriction (AnswerType_t *Answer, RedirectionNumberRestriction_t *RNR, uint8_t *redirRestInd)
{
	if (!Answer || !RNR || !redirRestInd)
		return 1;

	*redirRestInd = !!(*redirRestInd);

	memcpy (RNR, redirRestInd, sizeof *redirRestInd);

	Answer->redirectionRestInd = RNR;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_PlayInfo (AnswerType_t *Answer, PlayInfo_t *PlayInfo, uint8_t *pla_toneInfo, char *pla_file)
{
	if (!Answer || !PlayInfo || !pla_toneInfo || !pla_file)
		return 1;

	if (pla_toneInfo)
	{
		PlayInfo->tone = *pla_toneInfo;
		/*ASN_SET_ISPRESENT(PlayInfo, PlayInfo_PR_tone);*/
	} else
	if (pla_file)
	{
		asn_generate_OCTET_STRING(&PlayInfo->file, pla_file);
		/*ASN_SET_ISPRESENT(PlayInfo, PlayInfo_PR_file);*/
	}

	if (pla_toneInfo || pla_file)
		Answer->play = PlayInfo;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static int 
asn_answer_add_Detect (AnswerType_t *Answer, Detect_t *Detect, char *detect)
{
	if (!Answer || !Detect || !detect)
		return 1;

	asn_generate_OCTET_STRING(Detect, detect);

	Answer->detect = Detect;

	return 0;
}

/*-----------------------------------------------------------------------------------------------------------*/

static SmarTIMessage_t *
asn_SmarTIMessage_Answer (SmarTIMessage_t *SmarTIMessage, msg_info_t *msg_info, AnswerBasic_t *basic, AnswerOptional_t *optional,
                          BackwardCallIndicators_t *BackwardCallIndicators, OptionalBackwardCallInidicators_t *OptionalBackwardCallInidicators,
                          /*GenericNotificationIndicatorList_t *GenericNotificationIndicatorList,*/ ConnectedNumber_t *ConnectedNumber,
                          GenericNumber_t *GenericNumber, RedirectionNumber_t *RedirectionNumber,
                          RedirectionNumberRestriction_t *RedirectionNumberRestriction, PlayInfo_t *PlayInfo, Detect_t *Detect)
{
	if (!SmarTIMessage || !basic)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]\n", __func__, __LINE__,
		       (SmarTIMessage)? "ok":"fail", (basic)? "ok":"fail");
		goto ext;
	}

	SmarTIBody_t SmarTIBody = {0};
	AnswerType_t Answer;

	memset (&Answer, 0, sizeof (Answer));

	asn_generate_OCTET_STRING(&Answer.timestamp, get_loc_time());


	if (optional)
	{
		asn_answer_add_BackwardCallIndicators (&Answer, BackwardCallIndicators, optional->bci);
		asn_answer_add_OptionalBackwardCallInidicators (&Answer, OptionalBackwardCallInidicators, optional->obci);
		// asn_answer_add_GenericNotificationIndicatorList (&Answer, GenericNotificationIndicatorList, optional->);
		asn_answer_add_ConnectedNumber (&Answer, ConnectedNumber, optional->connNum);
		asn_answer_add_GenericNumber (&Answer, GenericNumber, optional->genNum);
		asn_answer_add_RedirectionNumber (&Answer, RedirectionNumber, optional->redirNum);
		asn_answer_add_RedirectionNumberRestriction (&Answer, RedirectionNumberRestriction, optional->redirRestInd);
		asn_answer_add_PlayInfo (&Answer, PlayInfo, optional->pla_toneInfo, optional->pla_file);
		asn_answer_add_Detect (&Answer, Detect, optional->detect);
	}

	if (asn_SmarTIBody_fill (&SmarTIBody, SmarTIBody_PR_answer, (void *)&Answer) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIBody_add_Release: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (asn_SmarTIMessage_fill (SmarTIMessage, msg_info->swSID, msg_info->appSID, SmarTIBody) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_fill: fail\n", __func__, __LINE__);
		goto ext;
	}

	return SmarTIMessage;

ext:
	return NULL;
}

int asn_encode_Answer (msg_info_t *msg_info, AnswerBasic_t *basic, AnswerOptional_t *optional, void *buffer, int buffer_size)
{
	asn_enc_rval_t  enc_res = {0};
	SmarTIMessage_t SmarTIMessage = {0};

	BackwardCallIndicators_t           BackwardCallIndicators = {0};
	OptionalBackwardCallInidicators_t  OptionalBackwardCallInidicators = {0};
	/*GenericNotificationIndicatorList_t GenericNotificationIndicatorList = {0};*/
	ConnectedNumber_t                  ConnectedNumber;
	GenericNumber_t                    GenericNumber;
	RedirectionNumber_t                RedirectionNumber;
	RedirectionNumberRestriction_t     RedirectionNumberRestriction = {0};
	PlayInfo_t                         PlayInfo = {0};
	Detect_t                           Detect = {0};

	if (!buffer || !basic || !msg_info)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s] 3rd [%s]\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail", (msg_info)? "ok":"fail");
		enc_res.encoded = -1;
		goto ext;
	}


	memset (&ConnectedNumber, 0, sizeof (ConnectedNumber));
	memset (&GenericNumber, 0, sizeof (GenericNumber));
	memset (&RedirectionNumber, 0, sizeof (RedirectionNumber));

	if (asn_SmarTIMessage_Answer (&SmarTIMessage, msg_info, basic, optional, &BackwardCallIndicators,
	                              &OptionalBackwardCallInidicators, /*&GenericNotificationIndicatorList,*/
	                              &ConnectedNumber, &GenericNumber, &RedirectionNumber,
	                              &RedirectionNumberRestriction, &PlayInfo, &Detect) == NULL)
	{
		printf ("[%s][%d] asn_SmarTIMessage_Answer: fail\n", __func__, __LINE__);
		goto ext;
	}

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, &SmarTIMessage);

	enc_res = der_encode_to_buffer(&asn_DEF_SmarTIMessage, &SmarTIMessage, buffer, buffer_size);

ext:
	return enc_res.encoded;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_decode_Answer (AnswerBasic_t *basic, AnswerOptional_t *optional, void *buffer)
{
	asn_dec_rval_t dec_res = {0};
	SmarTIMessage_t *SmarTIMessage = NULL;
	AnswerType_t *Answer;

	if (!buffer || !basic)
	{
		printf("[%s][%d] arg error: 1st [%s]; 2nd [%s]\n", __func__, __LINE__,
		       (buffer)? "ok":"fail", (basic)? "ok":"fail");
		dec_res.code = RC_FAIL;
		goto ext;
	}


	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	Answer = &SmarTIMessage->body.choice.answer;

	memcpy (basic->timestamp, Answer->timestamp.buf, Answer->timestamp.size);

	if (!optional)
		goto ext;

	if (traceON >= 50)
	{
		printf("\n[Answer] OPTIONS params:\n"
		        "bci                %s\n"
		        "obci               %s\n"
		        "gnotification      %s\n"
		        "connectedNumber    %s\n"
		        "genericNumber      %s\n"
		        "redirectionNumber  %s\n"
		        "redirectionRestInd %s\n"
		        "play               %s\n"
		        "detect             %s\n"
		        "noCDR              %s\n"
		        "bridge             %s\n"
		        "detached           %s\n"
		        "toLog              %s\n"
		        "other              %s\n",
		        (Answer->bci)? "yes":"no",
		        (Answer->obci)? "yes":"no",
		        (Answer->gnotification)? "yes":"no",
		        (Answer->connectedNumber)? "yes":"no",
		        (Answer->genericNumber)? "yes":"no",
		        (Answer->redirectionNumber)? "yes":"no",
		        (Answer->redirectionRestInd)? "yes":"no",
		        (Answer->play)? "yes":"no",
		        (Answer->detect)? "yes":"no",
		        (Answer->noCDR)? "yes":"no",
		        (Answer->bridge)? "yes":"no",
		        (Answer->detached)? "yes":"no",
		        (Answer->toLog)? "yes":"no",
		        (Answer->other)? "yes":"no");
	}

	if (Answer->bci)
		memcpy (optional->bci, Answer->bci->buf, Answer->bci->size);

	if (Answer->obci)
		memcpy (optional->obci, Answer->obci->buf, Answer->obci->size);

	// if (Answer->gnotification)
	// {
			/* ? */
	// }

	if (Answer->redirectionNumber)
	{
		sscanf ((char *)Answer->redirectionNumber->nai.buf, "%"SCNd8, &optional->redirNum->type);
		sscanf ((char *)Answer->redirectionNumber->npi.buf, "%"SCNd8, &optional->redirNum->numplan);

		memcpy(optional->redirNum->sym, Answer->redirectionNumber->address.buf, Answer->redirectionNumber->address.size);
	}

	if (Answer->redirectionRestInd)
	{
		*optional->redirRestInd = *Answer->redirectionRestInd;
	}

	if (Answer->genericNumber)
	{
		sscanf ((char *)Answer->genericNumber->nai.buf, "%"SCNd8, &optional->genNum->type);
		sscanf ((char *)Answer->genericNumber->npi.buf, "%"SCNd8, &optional->genNum->numplan);

		memcpy(optional->genNum->sym, Answer->genericNumber->address.buf, Answer->genericNumber->address.size);
		//memcpy(optional->genNum->sym, Answer->genericNumber->nqi.buf, Answer->genericNumber->nqi.size); /* ? */

		optional->genNum->present = Answer->genericNumber->apri;
		optional->genNum->pfx = Answer->genericNumber->ni;
		optional->genNum->screen = Answer->genericNumber->screening;
	}

	if (Answer->connectedNumber)
	{
		sscanf ((char *)Answer->connectedNumber->nai.buf, "%"SCNd8, &optional->connNum->type);
		sscanf ((char *)Answer->connectedNumber->npi.buf, "%"SCNd8, &optional->connNum->numplan);

		memcpy(optional->connNum->sym, Answer->connectedNumber->address.buf, Answer->connectedNumber->address.size);

		optional->connNum->present = Answer->connectedNumber->apri;
		optional->connNum->screen = Answer->connectedNumber->screening;
	}

	if (Answer->play)
	{
		// if (Progress->collectedInfo->present == CollectedInfo_PR_tone)
		// {
		// 	*optional->col_toneInfo = Progress->collectedInfo->choice.tone;
		// } else
		// if (Progress->collectedInfo->present == CollectedInfo_PR_signal)
		// {
		// 	memcpy(optional->col_signal, Progress->collectedInfo->choice.signal.buf, Progress->collectedInfo->choice.signal.size);
		// }
	}

	// if (Progress->noCDR)
	// {

	// }

	// if (Progress->bridge)
	// {

	// }

	// if (Progress->detached)
	// {

	// }

	// if (Progress->toLog)
	// {

	// }

	// if (Progress->other)
	// {

	// }


ext:
	switch(dec_res.code)
	{
		case RC_OK:
			if (traceON)
				printf ("%s: RC_OK\n", __func__);
			break;
		case RC_FAIL:  printf ("%s: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("%s: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

/*------------------------------------------------------------------------------------------------------------*/

int asn_decode_msg (dec_msg_t *dec_msg, void *buffer)
{
	asn_dec_rval_t dec_res = {0};
	SmarTIMessage_t *SmarTIMessage = NULL;

	dec_res.code = RC_FAIL;

	if (!buffer)
	{
		printf("[%s][%d] arg error: 1st [%s];\n", __func__, __LINE__,
		       (buffer)? "ok":"fail");
		goto ext;
	}

	dec_res = ber_decode (0, &asn_DEF_SmarTIMessage, (void **)&SmarTIMessage, buffer, sizeof(SmarTIMessage_t));

	if (!SmarTIMessage)
		goto ext;

	if (traceON >= 90)
		xer_fprint(stdout, &asn_DEF_SmarTIMessage, SmarTIMessage);

	dec_msg->info = (msg_info_t *)calloc(1, sizeof (msg_info_t));
	if (!dec_msg->info)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	dec_msg->present = (int *)calloc(1, sizeof (int));
	if (!dec_msg->present)
	{
		printf("[%s][%d] failed to allocate memory: %s\n", __func__, __LINE__, strerror(errno));
		goto ext;
	}

	dec_msg->info->version = SmarTIMessage->version;
	memcpy (dec_msg->info->swSID, SmarTIMessage->legID.swSessionID.buf, SmarTIMessage->legID.swSessionID.size);
	memcpy (dec_msg->info->appSID, SmarTIMessage->legID.appSessionID.buf, SmarTIMessage->legID.appSessionID.size);

	switch (SmarTIMessage->body.present)
	{
		case SmarTIBody_PR_connectionRequest:
			asn_get_ConnectionRequest (SmarTIMessage, &dec_msg);
			break;
		case SmarTIBody_PR_connectionResponse:
			asn_get_ConnectionResponse (SmarTIMessage, &dec_msg);
			break;
		// case SmarTIBody_PR_connectionReject:
		// 	asn_get_ConnectionReject (SmarTIMessage, present, &dec_msg);
		// 	break;
		case SmarTIBody_PR_connectionUpdateRequest:
			asn_get_ConnectionUpdateRequest (SmarTIMessage, &dec_msg);
			break;
		case SmarTIBody_PR_connectionUpdateResponse:
			asn_get_ConnectionUpdateResponse (SmarTIMessage, &dec_msg);
			break;
		case SmarTIBody_PR_seize:
			asn_get_Seize (SmarTIMessage, &dec_msg);
			break;
		// case SmarTIBody_PR_progress:
		// 	asn_get_Progress (SmarTIMessage, &dec_msg);
		// 	break;
		// case SmarTIBody_PR_answer:
		// 	asn_get_Answer (SmarTIMessage, &dec_msg);
		// 	break;
		// case SmarTIBody_PR_release:
		// 	asn_get_Release (SmarTIMessage, &dec_msg);
		// 	break;
	}

ext:
	ASN_STRUCT_FREE(asn_DEF_SmarTIMessage, SmarTIMessage);

	switch(dec_res.code)
	{
		case RC_OK:
			if (traceON)
				printf ("%s: RC_OK\n", __func__);
			break;
		case RC_FAIL:  printf ("%s: RC_FAIL\n", __func__);  break;
		case RC_WMORE: printf ("%s: RC_WMORE\n", __func__); break;
	}

	return dec_res.code;
}

void asn_clean (dec_msg_t *dec_msg)
{
	if (!dec_msg)
		return;

	switch (*dec_msg->present)
	{
		case SmarTIBody_PR_connectionRequest:        asn_clean_ConnectionRequest (&dec_msg);                break;
		case SmarTIBody_PR_connectionResponse:       asn_clean_ConnectionResponse (&dec_msg);               break;
		/*case SmarTIBody_PR_connectionReject:         asn_clean_ConnectionReject (&dec_msg);                 break;*/
		case SmarTIBody_PR_connectionUpdateRequest:  asn_clean_ConnectionUpdateRequest (&dec_msg);          break;
		case SmarTIBody_PR_connectionUpdateResponse: asn_clean_ConnectionUpdateResponse (&dec_msg); break;
		case SmarTIBody_PR_seize:                    asn_clean_Seize (&dec_msg);                            break;
		// case SmarTIBody_PR_progress:                 asn_clean_Progress (&dec_msg);                         break;
		// case SmarTIBody_PR_answer:                   asn_clean_Answer (&dec_msg);                           break;
		// case SmarTIBody_PR_release:                  asn_clean_Release (&dec_msg);                          break;
	}

	xfree(dec_msg->info);
	xfree(dec_msg->present);
}