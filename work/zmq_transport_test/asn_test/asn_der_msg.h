#ifndef _ASN_IVR_API_H_
#define _ASN_IVR_API_H_

#include <inttypes.h>

#include "CalledPartyNumber.h"
#include "CallingPartyNumber.h"
#include "CallReference.h"
#include "CallingPartysCategory.h"
#include "TrunkGroupId.h"
#include "OriginalCalledNumber.h"
#include "GenericNumber.h"
#include "RedirectingNumber.h"
#include "RedirectionInformation.h"

enum reject_cause {
	reject_prost = 0,
	reject_hate_you,

	reject_cause_max
};

#define MAX_NUMBER_LEN 30
#define MAX_OCT_LEN 16

typedef struct
{
	short      set:1,
	           pfx:1,
	           present:3,
	           screen:3,
	           cplt:1;
	uint8_t    type;
	uint8_t    category;
	uint8_t    numplan;
	uint8_t    nqi; /*number qualifier indicator  in Generic Number*/
	uint8_t    generic_notify;

	short      ss7_ni; /* nature of address indicator */
	short      ss7_np; /* NI, Num plan, Present restr, Screen ind */

	int        length;
	uint8_t    sym[MAX_NUMBER_LEN+2];
} __attribute__((packed)) number_t;

typedef struct bridge {
	char *leg1;
	char *leg2;
} bridge_t;

typedef struct msg_info
{
	int  version; /*not necessarily to fill. will be rewrited*/
	char swSID[MAX_OCT_LEN];
	char appSID[MAX_OCT_LEN];
} msg_info_t;

/* ---------- Seize message -------- */
typedef struct CdPN {
	uint8_t
	        inn:1,
	        res:7;

	uint8_t npi;
	uint8_t nai;

	char strCalled[MAX_NUMBER_LEN+2];
} CdPN_t;

typedef struct RedirInfo {
	e_RedirectingInd redirecting;
	e_OriginalRedirectionReason oreason;
	e_RedirectingReason reason;
	int counter;
} RedirInfo_t;

typedef struct SeizeBasic {
	char vatsId[MAX_OCT_LEN];
	char applicationId[MAX_OCT_LEN];
	char timestamp[MAX_OCT_LEN]; /*not necessarily to fill. will be rewrited*/
	CdPN_t CalledPN;
} SeizeBasic_t;

typedef struct SeizeOptional {
	number_t *cgpn;
	int *callRef;
	int *category;
	long *TGID;

	char *fci;
	char *usi;
	char *uti;
	char *tmr;

	number_t *origNum;
	number_t *genNum;
	number_t *redirNum;
	RedirInfo_t *info;

	uint8_t *noCDR;
	bridge_t *bridge;
	uint8_t *detached;
	char *toLog;
} SeizeOptional_t;

/* --------------------------------- */

/* -------- Progress message ------- */
typedef struct ProgressBasic {
	char timestamp[MAX_OCT_LEN]; /*not necessarily to fill. will be rewrited*/

	uint8_t e_Ind;  /*event field*/
	uint8_t e_Pres; /*event field*/

} ProgressBasic_t;

typedef struct ProgressOptional {
	char *cause;
	char *obci;

//	gnotification       [3] GenericNotificationIndicatorList OPTIONAL

	number_t *redirNumber;

	uint8_t *redirRestInd;

	uint8_t *NotifSubscOptions; /*callDiversion*/
	uint8_t *RedirReason;       /*callDiversion*/

	number_t *callTransNum;

	uint8_t *col_toneInfo;  /*collectedInfo field*/
	char    *col_signal;    /*collectedInfo field*/

	uint8_t *pla_toneInfo;       /*PlayInfo field*/
	char    *pla_file; /* URL */ /*PlayInfo field*/

	uint8_t *noCDR;
	bridge_t *bridge;
	uint8_t *detached;
	char *toLog;
} ProgressOptional_t;

/* ---------------------------------- */

/* ---------- Answer message -------- */
typedef struct AnswerBasic {
	char timestamp[MAX_OCT_LEN]; /*not necessarily to fill. will be rewrited*/
} AnswerBasic_t;

typedef struct AnswerOptional {
	char *bci;
	char *obci;

	//gnotification       [2] GenericNotificationIndicatorList OPTIONAL

	number_t *connNum;
	number_t *genNum;
	number_t *redirNum;

	uint8_t *redirRestInd;

	uint8_t *pla_toneInfo;       /*PlayInfo field*/
	char    *pla_file; /* URL */ /*PlayInfo field*/

	char *detect;

	uint8_t *noCDR;
	bridge_t *bridge;
	uint8_t *detached;
	char *toLog;

} AnswerOptional_t;

/* ---------------------------------- */

/* --------- Release message -------- */
typedef struct ReleaseBasic {
	char timestamp[MAX_OCT_LEN]; /*not necessarily to fill. will be rewrited*/
	char cause[MAX_OCT_LEN];
} ReleaseBasic_t;

typedef struct ReleaseOptional {
	uint8_t *noCDR;
	uint8_t *detached;
	char *toLog;
} ReleaseOptional_t;
/* ---------------------------------- */

typedef struct dec_msg {
	msg_info_t *info;
	int  *present;
	void *base;
	void *options;
} dec_msg_t;

enum msg_present {
	msg_NOTHING,	/* No components present */
	msg_connectionRequest,
	msg_connectionResponse,
	msg_connectionReject,
	msg_connectionUpdateRequest,
	msg_connectionUpdateResponse,
	msg_seize,
	msg_progress,
	msg_answer,
	msg_release
};

void asn_set_trace (int trace);

/* ------- Transports messages ------ */
int asn_encode_ConnectionResponse (msg_info_t *msg_info, long check_time, void *buffer, int buffer_size);
//int asn_decode_ConnectionResponse (msg_info_t *msg_info, int *updateTimeout, void *buffer);

int asn_encode_ConnectionReject (msg_info_t *msg_info, int cause, void *buffer, int buffer_size);

int asn_encode_ConnectionUpdateRequest (msg_info_t *msg_info, int set_timestamp, void *buffer, int buffer_size);
//int asn_decode_ConnectionUpdateRequest (msg_info_t *msg_info, char *timestamp, void *buffer);

int asn_encode_ConnectionUpdateResponse (msg_info_t *msg_info, int set_timestamp, void *buffer, int buffer_size);
//int asn_decode_ConnectionUpdateResponse (msg_info_t *msg_info, char *timestamp, void *buffer);

/* ---------- Seize message --------- */
int asn_encode_Seize (msg_info_t *msg_info, SeizeBasic_t *basic, SeizeOptional_t *optional, void *buffer, int buffer_size);
//int asn_decode_Seize (SeizeBasic_t *basic, SeizeOptional_t *optional, void *buffer);

/* -------- Progress message -------- */
int asn_encode_Progress (msg_info_t *msg_info, ProgressBasic_t *basic, ProgressOptional_t *optional, void *buffer, int buffer_size);
//int asn_decode_Progress (ProgressBasic_t *basic, ProgressOptional_t *optional, void *buffer);

/* --------- Answer message --------- */
int asn_encode_Answer (msg_info_t *msg_info, AnswerBasic_t *basic, AnswerOptional_t *optional, void *buffer, int buffer_size);
//int asn_decode_Answer (AnswerBasic_t *basic, AnswerOptional_t *optional, void *buffer);

/* --------- Release message -------- */
int asn_encode_Release (msg_info_t *msg_info, ReleaseBasic_t *basic, ReleaseOptional_t *optional, void *buffer, int buffer_size);
//int asn_decode_Release (ReleaseBasic_t *basic, ReleaseOptional_t *optional, void *buffer);

int asn_decode_msg (dec_msg_t *dec_msg, void *buffer);

#endif /*_ASN_IVR_API_H_*/
