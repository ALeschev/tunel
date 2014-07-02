#include <stdio.h>
#include <string.h>

#include "asn_der_msg.h"

int main(int argc, char const *argv[])
{
	char buffer[2048] = {0};

	int trace = 0;

	asn_set_trace (trace);

#if 0 /* ConnectionResponseType TEST*/
{
	if (asn_encode_ConnectionResponse ("15", "17", 165, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char APPSID[64] = {0};
		char SWSID[64] = {0};
		int timeout = -1;

		asn_decode_ConnectionResponse (SWSID, APPSID, &timeout, buffer);

		printf("SWSID %s\n", SWSID);
		printf("APPSID %s\n", APPSID);
		printf("timeout %d\n", timeout);
	}
}
#endif

#if 0 /* ConnectionUpdateReques TEST*/
{
	if (asn_encode_ConnectionUpdateRequest ("15", "17", 1, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char APPSID[64] = {0};
		char SWSID[64] = {0};
		char timestamp[128] = {0};

		asn_decode_ConnectionUpdateRequest (SWSID, APPSID, timestamp, buffer);

		printf("SWSID %s\n", SWSID);
		printf("APPSID %s\n", APPSID);

		if (strlen(timestamp))
			printf("timestamp %s\n", timestamp);
		else
			printf("timestamp not set\n");
	}
}
#endif

#if 0 /* ConnectionUpdateResponse TEST*/
{
	if (asn_encode_ConnectionUpdateResponse ("15", "17", 1, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		char APPSID[64] = {0};
		char SWSID[64] = {0};
		char timestamp[128] = {0};

		asn_decode_ConnectionUpdateResponse (SWSID, APPSID, timestamp, buffer);

		printf("SWSID %s\n", SWSID);
		printf("APPSID %s\n", APPSID);

		if (strlen(timestamp))
			printf("timestamp %s\n", timestamp);
		else
			printf("timestamp not set\n");
	}
}
#endif

#if 0 /* ConnectionRejectType TEST*/
{
	if (asn_encode_ConnectionReject ("15", "17", reject_hate_you, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		// int timeout = -1;

		// asn_decode_ConnectionResponse (&timeout, buffer);

		// printf("timeout %d\n", timeout);
	}
}
#endif

#if 1 /* SeizeType TEST*/
{
	SeizeBasic_t basic = {0};
	SeizeOptional_t optional = {0};
	RedirInfo_t RInfo = {0};
	msg_info_t msg_info;

	char fci[] = "1";
	char usi[] = "2";
	char uti[] = "3";
	char tmr[] = "4";

	CdPN_t Called = {0};
	long TGID = 17;
	int redirCount = 5;
	int callref = 5555;
	int category = 1;
	number_t cgpn = {0};
	number_t origNum = {0};
	number_t genNum = {0};

	Called.inn = 1;
	Called.npi = 23;
	Called.nai = 8;
	strcpy(Called.strCalled, "4446854");

	strcpy (cgpn.sym, "111111");
	cgpn.type = 10;
	cgpn.numplan = 1;
	cgpn.nqi = 7;
	cgpn.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	cgpn.present = 1;

	strcpy (origNum.sym, "222222");
	origNum.type = 11;
	origNum.numplan = 1;
	origNum.nqi = 8;
	origNum.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	origNum.present = 2;

	strcpy (genNum.sym, "333333");
	genNum.type = 12;
	genNum.numplan = 1;
	genNum.nqi = 9;
	genNum.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	genNum.present = 3;

	RInfo.redirecting = RedirectingInd_callRerouted;
	RInfo.oreason = OriginalRedirectionReason_userBusy;
	RInfo.counter = redirCount;
	RInfo.reason = RedirectingReason_noReply;

	strncpy (msg_info.swSID, "4598", MAX_OCT_LEN);
	strncpy (msg_info.appSID, "49", MAX_OCT_LEN);

	strncpy (basic.vatsId, "443", MAX_OCT_LEN);
	strncpy (basic.applicationId, "132", MAX_OCT_LEN);
	memcpy (&basic.CalledPN, &Called, sizeof (Called));

	optional.cgpn = &cgpn;
	optional.callRef = &callref;
	optional.category = &category;
	optional.TGID = &TGID;
	optional.origNum = &origNum;
	optional.genNum = &genNum;
	optional.info = &RInfo;

	optional.fci = fci;
	optional.usi = usi;
	optional.uti = uti;
	optional.tmr = tmr;

#ifdef MEM_DEBUG
	int i;
	for (i = 0; i < 999; i++)
#endif
	{
		if (asn_encode_Seize (&msg_info, &basic, &optional, buffer, sizeof(buffer)) < 0)
		{
			printf ("encoded failed!\n");
			goto ext;
		} else {
			dec_msg_t dec_msg;

			if (asn_decode_msg (&dec_msg, buffer) < 0)
			{
				printf("decode failed\n");
				goto ext;
			}

	/*		switch (*dec_msg.present)
			{
				case msg_connectionRequest: printf("msg_connectionRequest\n"); break;
				case msg_connectionResponse: printf("msg_connectionResponse\n"); break;
				case msg_connectionReject: printf("msg_connectionReject\n"); break;
				case msg_connectionUpdateRequest: printf("msg_connectionUpdateRequest\n"); break;
				case msg_connectionUpdateResponse: printf("msg_connectionUpdateResponse\n"); break;
				case msg_seize: printf("msg_seize\n"); break;
				case msg_progress: printf("msg_progress\n"); break;
				case msg_answer: printf("msg_answer\n"); break;
				case msg_release: printf("msg_release\n"); break;
			}

			SeizeBasic_t *SeizeBasic;
			SeizeOptional_t * SeizeOptional;

			SeizeBasic = (SeizeBasic_t *)dec_msg.base;
			SeizeOptional = (SeizeOptional_t *)dec_msg.options;

			printf("\ndec_msg.info->swSID %s\n"
			         "dec_msg.info->appSID %s\n"
			         "dec_msg.base->vatsId %s\n"
			         "dec_msg.base->applicationId %s\n"
			         "dec_msg.base->CalledPN.inn %d\n"
			         "dec_msg.base->CalledPN.npi %d\n"
			         "dec_msg.base->CalledPN.nai %d\n"
			         "dec_msg.base->CalledPN.strCalled %s\n"
			         "dec_msg.base->timestamp %s\n",
			         dec_msg.info->swSID,
			         dec_msg.info->appSID,
			         SeizeBasic->vatsId,
			         SeizeBasic->applicationId,
			         SeizeBasic->CalledPN.inn,
			         SeizeBasic->CalledPN.npi,
			         SeizeBasic->CalledPN.nai,
			         SeizeBasic->CalledPN.strCalled,
			         SeizeBasic->timestamp);

			if (SeizeOptional->cgpn)
				printf("\nSeizeOptional->cgpn->sym %s\n"
				         "SeizeOptional->cgpn->type %d\n"
				         "SeizeOptional->cgpn->numplan %d\n"
				         "SeizeOptional->cgpn->nqi %d\n"
				         "SeizeOptional->cgpn->screen %d\n"
				         "SeizeOptional->cgpn->present %d\n",
				          SeizeOptional->cgpn->sym,
				          SeizeOptional->cgpn->type,
				          SeizeOptional->cgpn->numplan,
				          SeizeOptional->cgpn->nqi,
				          SeizeOptional->cgpn->screen,
				          SeizeOptional->cgpn->present);

			if (SeizeOptional->callRef)
				printf("\nSeizeOptional->callRef %d\n", *SeizeOptional->callRef);

			if (SeizeOptional->category)
				printf("\nSeizeOptional->category %d\n", *SeizeOptional->category);

			if (SeizeOptional->TGID)
				printf("\nSeizeOptional->TGID %ld\n", *SeizeOptional->TGID);

			if (SeizeOptional->origNum)
				printf("\nSeizeOptional->origNum->sym %s\n"
				         "SeizeOptional->origNum->type %d\n"
				         "SeizeOptional->origNum->numplan %d\n"
				         "SeizeOptional->origNum->nqi %d\n"
				         "SeizeOptional->origNum->screen %d\n"
				         "SeizeOptional->origNum->present %d\n",
				          SeizeOptional->origNum->sym,
				          SeizeOptional->origNum->type,
				          SeizeOptional->origNum->numplan,
				          SeizeOptional->origNum->nqi,
				          SeizeOptional->origNum->screen,
				          SeizeOptional->origNum->present);

			if (SeizeOptional->genNum)
				printf("\nSeizeOptional->genNum->sym %s\n"
				         "SeizeOptional->genNum->type %d\n"
				         "SeizeOptional->genNum->numplan %d\n"
				         "SeizeOptional->genNum->nqi %d\n"
				         "SeizeOptional->genNum->screen %d\n"
				         "SeizeOptional->genNum->present %d\n",
				          SeizeOptional->genNum->sym,
				          SeizeOptional->genNum->type,
				          SeizeOptional->genNum->numplan,
				          SeizeOptional->genNum->nqi,
				          SeizeOptional->genNum->screen,
				          SeizeOptional->genNum->present);

			if (SeizeOptional->info)
				printf("\nSeizeOptional->info->redirecting %d\n"
				         "SeizeOptional->info->oreason %d\n"
				         "SeizeOptional->info->counter %d\n"
				         "SeizeOptional->info->reason %d\n", 
				          SeizeOptional->info->redirecting,
				          SeizeOptional->info->oreason,
				          SeizeOptional->info->counter,
				          SeizeOptional->info->reason);

			if (SeizeOptional->fci)
				printf ("\nSeizeOptional->>fci %s\n", SeizeOptional->fci);

			if (SeizeOptional->usi)
				printf ("\nSeizeOptional->>usi %s\n", SeizeOptional->usi);

			if (SeizeOptional->uti)
				printf ("\nSeizeOptional->>uti %s\n", SeizeOptional->uti);

			if (SeizeOptional->tmr)
				printf ("\nSeizeOptional->>tmr %s\n", SeizeOptional->tmr);*/

			asn_clean(&dec_msg);
		}
	}
}
#endif

#if 0 /* ProgressType TEST*/
{

	char swSID[MAX_OCT_LEN] = "888";
	char appSID[MAX_OCT_LEN] = "999";

	ProgressBasic_t basic = {0};
	ProgressOptional_t optional = {0};

	strcpy (basic.swSID, swSID);
	strcpy (basic.appSID, appSID);

	basic.e_Ind = 2;
	basic.e_Pres = 1;

	char cause[] = "just for fun";
	char obci[] = "no";
	number_t RN;
	number_t CTN;
	uint8_t RRI = 1;
	uint8_t NSO = 5;
	uint8_t RR = 16;
	uint8_t toneN = 5;
	//char toneC[] = "4";

	strcpy (RN.sym, "111111");
	RN.type = 10;
	RN.numplan = 1;
	RN.nqi = 7;
	RN.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	RN.present = 1;

	strcpy (CTN.sym, "222222");
	CTN.type = 11;
	CTN.numplan = 1;
	CTN.nqi = 8;
	CTN.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	CTN.present = 2;

	optional.cause = cause;
	optional.obci = obci;

	optional.redirNumber = &RN;
	optional.redirRestInd = &RRI;
	optional.NotifSubscOptions = &NSO;
	optional.RedirReason = &RR;
	optional.callTransNum = &CTN;
	optional.col_toneInfo = &toneN;
	//optional.col_signal = toneC;

	if (asn_encode_Progress (trace, &basic, &optional, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		memset (cause, 0, sizeof (cause));
		memset (obci, 0, sizeof (obci));
		memset (&RN, 0, sizeof (RN));
		memset (&CTN, 0, sizeof (CTN));
		//memset (toneC, 0, sizeof (toneC));
		RRI = 0;
		NSO = 0;
		RR = 0;
		toneN = 0;

		if (asn_decode_Progress (trace, &basic, &optional, buffer) < 0)
		{
			printf ("asn_decode_Progress failed!\n");
			goto ext;
		}

		printf("\nbasic.swSID %s\n"
         "basic.appSID %s\n"
         "basic.e_Ind %d\n"
         "basic.e_Pres %d\n"
         "basic.timestamp %s\n",
         basic.swSID,
         basic.appSID,
         basic.e_Ind,
         basic.e_Pres,
         basic.timestamp);

		if (optional.redirNumber)
			printf("\noptional.redirNumber->sym %s\n"
			         "optional.redirNumber->type %d\n"
			         "optional.redirNumber->numplan %d\n"
			         "optional.redirNumber->nqi %d\n"
			         "optional.redirNumber->screen %d\n"
			         "optional.redirNumber->present %d\n",
			          optional.redirNumber->sym,
			          optional.redirNumber->type,
			          optional.redirNumber->numplan,
			          optional.redirNumber->nqi,
			          optional.redirNumber->screen,
			          optional.redirNumber->present);

		if (optional.callTransNum)
			printf("\noptional.callTransNum->sym %s\n"
			         "optional.callTransNum->type %d\n"
			         "optional.callTransNum->numplan %d\n"
			         "optional.callTransNum->nqi %d\n"
			         "optional.callTransNum->screen %d\n"
			         "optional.callTransNum->present %d\n",
			          optional.callTransNum->sym,
			          optional.callTransNum->type,
			          optional.callTransNum->numplan,
			          optional.callTransNum->nqi,
			          optional.callTransNum->screen,
			          optional.callTransNum->present);

		if (optional.cause)
			printf ("\noptional.cause: %s\n", optional.cause);

		if (optional.obci)
			printf ("\noptional.obci: %s\n", optional.obci);

		if (optional.col_signal)
			printf ("\noptional.col_signal: %s\n", optional.col_signal);

		if (optional.col_toneInfo)
			printf ("\noptional.col_toneInfo: " "%"SCNd8"\n", *optional.col_toneInfo);

		if (optional.redirRestInd)
			printf ("\noptional.redirRestInd: " "%"SCNd8"\n", *optional.redirRestInd);

		if (optional.NotifSubscOptions)
			printf ("\noptional.NotifSubscOptions: " "%"SCNd8"\n", *optional.NotifSubscOptions);

		if (optional.RedirReason)
			printf ("\noptional.RedirReason: " "%"SCNd8"\n", *optional.RedirReason);

	}

}
#endif

#if 0 /* AnswerType TEST*/
{
	char swSID[MAX_OCT_LEN] = "444";
	char appSID[MAX_OCT_LEN] = "2222";

	AnswerBasic_t basic = {0};
	AnswerOptional_t optional = {0};

	strcpy (basic.swSID, swSID);
	strcpy (basic.appSID, appSID);

	char bci[] = "bci";
	char obci[] = "obci";
	char detect[] = "detect";
	number_t connNum;
	number_t genNum;
	number_t redirNum;
	uint8_t RRI = 1;

	strcpy (connNum.sym, "111111");
	connNum.type = 10;
	connNum.numplan = 1;
	connNum.nqi = 7;
	connNum.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	connNum.present = 1;

	strcpy (genNum.sym, "222222");
	genNum.type = 11;
	genNum.numplan = 1;
	genNum.nqi = 8;
	genNum.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	genNum.present = 2;

	strcpy (redirNum.sym, "3333333");
	redirNum.type = 12;
	redirNum.numplan = 2;
	redirNum.nqi = 9;
	redirNum.screen = ScreeningInd_userProvidedVerifiedAndFailed;
	redirNum.present = 3;

	optional.bci = bci;
	optional.obci = obci;

	optional.detect = detect;

	optional.connNum = &connNum;
	optional.genNum = &genNum;
	optional.redirNum = &redirNum;
	optional.redirRestInd = &RRI;


	if (asn_encode_Answer (trace, &basic, &optional, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		memset (bci, 0, sizeof (bci));
		memset (obci, 0, sizeof (obci));
		memset (&connNum, 0, sizeof (connNum));
		memset (&genNum, 0, sizeof (genNum));
		memset (&redirNum, 0, sizeof (redirNum));
		RRI = 0;

		if (asn_decode_Answer (trace, &basic, &optional, buffer) < 0)
		{
			printf ("asn_decode_Progress failed!\n");
			goto ext;
		}

		printf("\nbasic.swSID %s\n"
	           "basic.appSID %s\n"
	           "basic.timestamp %s\n",
	           basic.swSID,
	           basic.appSID,
	           basic.timestamp);

		if (optional.bci)
					printf ("\noptional.bci: %s\n", optional.bci);

		if (optional.obci)
					printf ("\noptional.obci: %s\n", optional.obci);

		if (optional.connNum)
			printf("\noptional.connNum->sym %s\n"
			         "optional.connNum->type %d\n"
			         "optional.connNum->numplan %d\n"
			         "optional.connNum->nqi %d\n"
			         "optional.connNum->screen %d\n"
			         "optional.connNum->present %d\n",
			          optional.connNum->sym,
			          optional.connNum->type,
			          optional.connNum->numplan,
			          optional.connNum->nqi,
			          optional.connNum->screen,
			          optional.connNum->present);

		if (optional.genNum)
			printf("\noptional.genNum->sym %s\n"
			         "optional.genNum->type %d\n"
			         "optional.genNum->numplan %d\n"
			         "optional.genNum->nqi %d\n"
			         "optional.genNum->screen %d\n"
			         "optional.genNum->present %d\n",
			          optional.genNum->sym,
			          optional.genNum->type,
			          optional.genNum->numplan,
			          optional.genNum->nqi,
			          optional.genNum->screen,
			          optional.genNum->present);

		if (optional.redirNum)
			printf("\noptional.redirNum->sym %s\n"
			         "optional.redirNum->type %d\n"
			         "optional.redirNum->numplan %d\n"
			         "optional.redirNum->nqi %d\n"
			         "optional.redirNum->screen %d\n"
			         "optional.redirNum->present %d\n",
			          optional.redirNum->sym,
			          optional.redirNum->type,
			          optional.redirNum->numplan,
			          optional.redirNum->nqi,
			          optional.redirNum->screen,
			          optional.redirNum->present);

		if (optional.detect)
					printf ("\noptional.detect: %s\n", optional.detect);
	}

}
#endif


#if 0 /* ReleaseType TEST*/
{
	char swSID[MAX_OCT_LEN] = "444";
	char appSID[MAX_OCT_LEN] = "2222";

	ReleaseBasic_t basic = {0};
	ReleaseOptional_t optional = {0};

	char str_cause[128] = "prost))";

	strcpy (basic.swSID, swSID);
	strcpy (basic.appSID, appSID);

	memcpy (basic.cause, str_cause, sizeof (basic.cause));

	if (asn_encode_Release (trace, &basic, &optional, buffer, sizeof(buffer)) < 0)
	{
		printf ("encoded failed!\n");
		goto ext;
	} else {
		printf ("encoded\n");

		memset (swSID, 0, sizeof (swSID));
		memset (appSID, 0, sizeof (appSID));
		memset (str_cause, 0, sizeof (str_cause));

		if (asn_decode_Release (trace, &basic, &optional, buffer) < 0)
		{
			printf("asn_decode_Release failed\n");
			goto ext;
		}

		printf("\nbasic.swSID   %s\n"
	           "basic.appSID    %s\n"
	           "basic.cause     %s\n"
	           "basic.timestamp %s\n",
	            basic.swSID,
	            basic.appSID,
	            basic.cause,
	            basic.timestamp);
	}
}
#endif

ext:
	return 0;
}