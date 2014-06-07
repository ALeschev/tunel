/*
 * ivr.c
 *
 *  Created on: 22.05.2014
 *      Author: leschev
 */

#include "ivr_def.h"
#include "ivr.h"
#include "pbx_proc.h"
#include "service_port.h"
#include "service_port_def.h"
#include "handlers.h"
#include "ut_port.h"
#include "mqueue.h"
#include "sip_user.h"

static mqueue_t ivr_proc_que;
static pthread_t ivr_loop_thread;
static uint8_t ivr_running = 0;

static uint8_t ivr_mod_non_response = 0;

typedef struct _ivr_call
{
	tIVRCallData data;
} ivr_call_t;

static ivr_call_t *ivr_call = NULL;

static int ivr_callref_static = 0;

static ut_mutex_t *ivr_port_mutex = NULL;

/*----------------------------------------------------------------------------------------------*/

void *ivr_loop ();
void ivr_close(void);

static void ivr_clean_callers();

/*----------------------------------------------------------------------------------------------*/

// static char *ivr_cmd_str(enum ivr_commands cmd)
// {
// 	switch (cmd)
// 	{
// 		case CMD_SEIZE:         return "CMD_SEIZE";
// 		case CMD_PROGRESS:      return "CMD_PROGRESS";
// 		case CMD_RINGING_F:     return "CMD_RINGING_F";
// 		case CMD_ANSWER_F:      return "CMD_ANSWER_F";
// 		case CMD_HOLD_F:        return "CMD_HOLD_F";
// 		case CMD_RELEASE:       return "CMD_RELEASE";
// 		case CMD_TONEDETECTED:  return "CMD_TONEDETECTED";
// 		case CMD_PLAY_END:      return "CMD_PLAY_END";

// 		case CMD_BRIDGE:        return "CMD_BRIDGE";
// 		case CMD_LOG:           return "CMD_LOG";
// 		case CMD_NOCDR:         return "CMD_NOCDR";
// 		case CMD_RINGING_T:     return "CMD_RINGING_T";
// 		case CMD_ANSWER_T:      return "CMD_ANSWER_T";
// 		case CMD_BUSY:          return "CMD_BUSY";
// 		case CMD_CALL:          return "CMD_CALL";
// 		case CMD_DISCONNECT:    return "CMD_DISCONNECT";
// 		case CMD_SENDDTMF:      return "CMD_SENDDTMF";
// 		case CMD_PLAYBACK:      return "CMD_PLAYBACK";
// 		case CMD_PLAYTONE:      return "CMD_PLAYTONE";
// 		case CMD_STOPPLAYTONE:  return "CMD_STOPPLAYTONE";
// 		case CMD_DETECTTONE:    return "CMD_DETECTTONE";
// 		case CMD_HOLD_T:        return "CMD_HOLD_T";
// 		case CMD_RECORD:        return "CMD_RECORD";
// 		case CMD_STOPRECORD:    return "CMD_STOPRECORD";

// 		case CMD_CLEAN_CALLERS: return "CMD_CLEAN_CALLERS";
// 		case CMD_MODULE_CHECK:  return "CMD_MODULE_CHECK";
// 		case CMD_MODULE_LOST:   return "CMD_MODULE_LOST";

// 		case CMD_END:           return "CMD_END";
// 	}

// 	return "undef";
// }

/*----------------------------------------------------------------------------------------------*/

// static char *ivr_call_state_str(int state)
// {
// 	static char str[20];

// 	switch(state){
// 		case IVR_CALL_NULL_STATE:    return "IVR_CALL_NULL_STATE";
// 		case IVR_CALL_IDLE:          return "IVR_CALL_IDLE";
// 		case IVR_CALL_NEW:           return "IVR_CALL_NEW";
// 		case IVR_CALL_LISTEN_MUSIC:  return "IVR_CALL_LISTEN_MUSIC";
// 		case IVR_CALL_COLLECT_DIG:   return "IVR_CALL_COLLECT_DIG";
// 		case IVR_CALL_TALKING:       return "IVR_CALL_TALKING";
// 	}

// 	sprintf(str, "_undef_%02X_", state);
// 	return str;
// }

/*----------------------------------------------------------------------------------------------*/

int ivr_init(void)
{
	int callref;

	// if (ivr_running)
	// 	return -1;

	app_trace(TRACE_INFO, "ivr: init.");

	ivr_call = (ivr_call_t *) ut_malloc_ex ( sizeof(ivr_call_t) * (IVR_CALLREF_MOD_MAX) );

	if(NULL == ivr_call)
	{
		return -1;
	}

	memset(ivr_call, 0, sizeof(ivr_call_t) * (IVR_CALLREF_MOD_MAX));

	for(callref = 0; callref < IVR_CALLREF_MOD_MAX; callref++)
	{
		tIVRCallData *pIVR = &ivr_call[callref].data;

		pIVR->CState = IVR_CALL_NULL_STATE;
	}

	if((ivr_port_mutex = ut_mutex_init()) == NULL)
	{
		app_trace(1, "ivr: mutex create error\r\n");
		ivr_close();

		return -1;
	}

	mqueue_init(&ivr_proc_que, MQUEUESIZE);
	pthread_create(&ivr_loop_thread, NULL, ivr_loop, NULL);
	ivr_running = 1;

	return 0;
}

/*----------------------------------------------------------------------------------------------*/

void ivr_close(void)
{
	// if (!ivr_running)
	// 	return;

	app_trace(TRACE_INFO, "ivr: close.");

	ivr_clean_callers();

	if(ivr_call != NULL) {ut_free_ex (ivr_call); ivr_call = NULL;}

	if(ivr_port_mutex)
	{
		ut_mutex_destroy(ivr_port_mutex);
		ut_free_ex(ivr_port_mutex);
	}

	mqueue_close(&ivr_proc_que);
	pthread_cancel (ivr_loop_thread);
	ivr_running = 0;
}

/*----------------------------------------------------------------------------------------------*/

static tIVRCallData *ivr_call_data(int callref)
{
	if(callref < 0 || callref >= IVR_CALLREF_MOD_MAX)
	{
		app_trace(1, "IVR. call-ref error [%d]", callref);
		return NULL;
	}

	return &ivr_call[callref].data;
}

/*----------------------------------------------------------------------------------------------*/

void ivr_call_init(int callref)
{
	tIVRCallData *pIVR = ivr_call_data(callref);

	pIVR->Callref = callref;

	memset (&pIVR->calling, 0, sizeof (pIVR->calling));
	memset (&pIVR->scr_data, 0, sizeof (pIVR->scr_data));

	pIVR->scr_idx = -1;
}

/*----------------------------------------------------------------------------------------------*/

int ivr_get_next_callref(int in)
{
	int callref = 0;
	int i = 0;
	tIVRCallData *pIVR = NULL;

	ut_mutex_lock(ivr_port_mutex);

	for( i = ivr_callref_static; i < IVR_CALLREF_MOD_MAX; i++)
	{
		callref = i;

		pIVR = ivr_call_data(callref);

		if(pIVR && pIVR->CState == IVR_CALL_NULL_STATE)
		{
			if(ivr_callref_static == i)
			{
				ivr_callref_static = i+1;
			}else{
				ivr_callref_static = i;
			}

			ut_mutex_unlock(ivr_port_mutex);

			return callref;
		}
	}

	for(i = 0; i < ivr_callref_static; i++)
	{
		callref = i;

		pIVR = ivr_call_data(callref);

		if(pIVR && pIVR->CState == IVR_CALL_NULL_STATE)
		{
			ivr_callref_static = i + 1;

			ut_mutex_unlock(ivr_port_mutex);

			return callref;
		}
	}

	ut_mutex_unlock(ivr_port_mutex);

	return -1;
}

/*----------------------------------------------------------------------------------------------*/

static tIVRCallData *ivr_call_create(int *callref, int in)
{
	tIVRCallData *pIVR;

	if(!callref) return NULL;

	*callref = ivr_get_next_callref(in);

	if(*callref < 0 || *callref >= IVR_CALLREF_MOD_MAX)
	{
		app_trace(TRACE_ERR, "IVR. CallCreate: can't find free callref! [%d]", *callref);

		return NULL;
	}

	pIVR = &ivr_call[*callref].data;

	ivr_call_init(*callref);

	return pIVR;
}

/*----------------------------------------------------------------------------------------------*/

static void ivr_set_state(tIVRCallData *pIVR, int state)
{
	if (!pIVR)
		return;

	if((pIVR->CState != state))
	{
		app_trace(TRACE_INFO, "IVR. Callref %04x. New state '%s'", pIVR->Callref, ivr_call_state_str(state));
	}

	pIVR->CState = state;
	pIVR->Timer = sys_tick_count_sec();
}

/*----------------------------------------------------------------------------------------------*/

// void ivr_Idle(int callref)
// {
// //	XPORT xport;
// 	tIVRCallData *pIVR;

// //	srv_xport(callref, &xport);

// //	srv_LinkOff(callref);

// //	pbxReleaseComplete(xport, cause & 0x7F, location);

// 	if((pIVR = ivr_CallData(callref)) != NULL)
// 		ivr_set_state(pIVR, IVR_CALL_NULL_STATE);
// }

/*----------------------------------------------------------------------------------------------*/

void ivr_clear_call(int callref)
{
	//srv_Idle(callref, cause, location);
	tIVRCallData *pIVR;

	if((pIVR = ivr_call_data(callref)) != NULL)
		ivr_set_state(pIVR, IVR_CALL_NULL_STATE);
}

/*----------------------------------------------------------------------------------------------*/

static int ivr_send_message (int module, int callref, int command, const char *_data, int size)
{
	char *answer;
	IVRCommands *to_ivr;
	uint8_t *payload;
	int payload_sz;

	payload_sz = sizeof(IVRCommands) + size;
	payload = calloc(1, payload_sz);

	to_ivr = (IVRCommands*)payload;
	to_ivr->ivr_command = command;
	to_ivr->callref = callref;

	memcpy (&payload[sizeof(IVRCommands)], _data, size);

	int res = send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, module, 0, payload_sz, (char *)payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		ut_free_ex(answer);
		answer = NULL;
	}

	ut_free_ex(payload);

	return res;
}

/*----------------------------------------------------------------------------------------------*/

static void ivr_send_command (int module, int command)
{
	char *answer;
	IVRCommands *to_ivr;
	uint8_t *payload;
	int payload_sz;

	payload_sz = sizeof(IVRCommands);
	payload = calloc(1, payload_sz);

	to_ivr = (IVRCommands*)payload;
	to_ivr->ivr_command = command;

	send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, module, 0, payload_sz, (char *)payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		ut_free_ex(answer);
		answer = NULL;
	}

	ut_free_ex(payload);
}

/*----------------------------------------------------------------------------------------------*/

static void ivr_send_caller(int callref, uint8_t *sym, int ivr_idx, int command)
{
	char *answer;
	uint8_t *payload;
	int payload_sz;

	caller_t new_call;
	IVRCommands *to_ivr;

	memset (&new_call, 0, sizeof (caller_t));

	payload_sz = sizeof(IVRCommands) + sizeof (caller_t);
	payload = ut_malloc_ex(payload_sz);

	if(!payload)
	{
		app_trace (TRACE_ERR, "ivr_send_caller: can't allocate memory");
		return;
	}

	to_ivr = (IVRCommands *)payload;
	to_ivr->ivr_command = command;
	to_ivr->callref = callref;

//	new_call.ivr_idx = 123;
	// new_call.state = (command == CMD_ADD_CALLER)? IVR_CALL_NEW : IVR_CALL_NULL_STATE;

	// if (sym)
	// {
	// 	new_call.length = strlen((char *)sym);
	// 	memcpy (new_call.sym, sym, sizeof (new_call.sym));
	// }

	memcpy(&payload[sizeof(IVRCommands)], &new_call, sizeof (caller_t));

	send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, IVRMOD, 0, payload_sz, payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		ut_free_ex(answer);
		answer = NULL;
	}

	ut_free_ex(payload);
}

/*----------------------------------------------------------------------------------------------*/

static void ivrCheckMod()
{
	char *answer;
	IVRCommands *to_ivr;
	uint8_t *payload;
	int payload_sz;

	payload_sz = sizeof(IVRCommands);
	payload = calloc(1, payload_sz);

	to_ivr = (IVRCommands*)payload;
	to_ivr->ivr_command = CMD_MODULE_CHECK;

	send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, IVRMOD, 0, payload_sz, payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		ut_free_ex(answer);
		answer = NULL;
	}
}

/*----------------------------------------------------------------------------------------------*/

static void ivr_clean_callers()
{
	tIVRCallData *pIVR;
	int i;

	ut_mutex_lock(ivr_port_mutex);

	for (i = 0; i < ivr_callref_static; i++)
	{
		if((pIVR = ivr_call_data(i)) == NULL)
		{
			app_trace(TRACE_INFO, "Get call data for [%04x] filed!", i);
			continue;
		}

		pbxRelease(pIVR->owner, NE41_TEMPORARY_FAILURE, 0);

		ivr_set_state(pIVR, /*IVR_CALL_IDLE*/ IVR_CALL_NULL_STATE);
	}

	ut_mutex_unlock(ivr_port_mutex);
}

/*----------------------------------------------------------------------------------------------*/

int ivrInDigit(XPORT xport, uint8_t digit)
{
	T_XPort *pX;
	TLinkData *ld;
	digits_t digit_to;
	tIVRCallData *pIVR;
	int res = 1;

	if((pX = XPortData(xport)) == NULL)
	{
		app_trace(TRACE_WARN, "pX == NULL");
		return res;
	}

	ld = &pX->link;

	if (!ld->LDF_IVR_CALL)
		return res;

	digit_to.digit = digit;

	int ivr_callref = srv_callref(xport);

	if((pIVR = ivr_call_data(ivr_callref)) == NULL)
	{
		app_trace(TRACE_WARN, "pIVR == NULL");
		return res;
	}

	res = ivr_send_message (IVRMOD, pIVR->Callref, CMD_DETECTTONE, (void*)&digit_to, sizeof (digit_to));

	return res;
}

/*----------------------------------------------------------------------------------------------*/

int ivr_DoRoute(int callref, char* number, XPORT *call_port)
{
	XPORT xport;
	T_XPort *pX;
	TLinkData *ld;
	tIVRCallData *pIVR;

	int res = 1;

	if((pIVR = ivr_call_data(callref)) == NULL)
	{
		app_trace(TRACE_WARN, "pIVR == NULL");
		return res;
	}

	if(srv_MakeNewCall(call_port, &pIVR->calling, -1))
	{
		return -2;
	}

	xport = *call_port;

	if((pX = XPortData(xport)) == NULL)
	{
		res = -3;
		goto do_route_exit;
	}

	ld = &pX->link;

	pX->F_NO_SORM = 1;

	pbxInDial(xport, number, strlen(number), 0);
	pbxDialProc(xport, 1);

	if(pX->F_LINK)
	{
		res = 0;
	}

do_route_exit:
	return res;
}

// static int ivrMakeCall(int callref, uint8_t *cmd_attr)
// {
// 	T_XPort *pX;
// 	XPORT xport, srv_port;
// 	call_to *called = NULL;
// 	tIVRCallData *pIVR;
// 	int res = 1;

// 	if((pIVR = ivr_call_data(callref)) == NULL)
// 	{
// 		app_trace(TRACE_WARN, "pIVR == NULL");
// 		return res;
// 	}

// 	called = (call_to *)cmd_attr;

// 	xport = pIVR->owner;

// 	if((pX = XPortData(xport)) == NULL)
// 	{
// 		app_trace(TRACE_WARN, "pX == NULL");
// 		return res;
// 	}

// 	if(ivr_DoRoute(callref, called->to, &srv_port))
// 	{
// 		/*failed to route*/
// 		return res;
// 	}

// 	res =  srv_pbxSetPeerPort(xport, srv_port);
// 	res += srv_pbxSetPeerPort(srv_port, xport);

// 	res += srv_pbxUpdateVchan(xport);

// 	return res;
// }

/*----------------------------------------------------------------------------------------------*/

void ivr_queue_put (void *element, int prio)
{
	if (!element)
	{
		app_trace(TRACE_INFO, "[%s][%d] element is empty\n", __func__, __LINE__);
		return;
	}

	if(mqueue_put(&ivr_proc_que, element, prio))
	{
		app_trace(TRACE_ERR, "[%s][%d] failed to put cmd into queue!\n", __func__, __LINE__);
		ut_free_ex(element);
	} else{
		//app_trace(TRACE_INFO, "[%s][%d] cmd-queue size [%d]\n", __func__, __LINE__, ivr_proc_que.count);
	}
}

/*----------------------------------------------------------------------------------------------*/

static void ivr_proc (IVRCommands *command)
{
	if (command->ivr_command != CMD_MODULE_CHECK)
		app_trace(TRACE_INFO, "[%s][%d] Proc cmd: '%s' (%d)", __func__, __LINE__, ivr_cmd_str(command->ivr_command), command->ivr_command);

//	int callref = command->callref;

	switch (command->ivr_command)
	{
#if 0
		case CMD_ADD_CALLER:
		break;

		case CMD_DEL_CALLER:
		break;

		case CMD_MAKE_CALL:
		{
			// tIVRCallData *pIVR;

			// if((pIVR = ivr_call_data(callref)) == NULL)
			// {
			// 	app_trace(TRACE_WARN, "pIVR == NULL");
			// 	return;
			// }

			// if (ivrMakeCall(callref, command->attribute) == 0)
			// {
			// 	ivr_set_state(pIVR, IVR_CALL_TALKING);
			// }
		}
		break;

		case CMD_PLAY_FILE:
		{
			// tIVRCallData *pIVR;
			// melody_t *for_play = (melody_t *)command->attribute;

			// int start = !!strlen(for_play->file_name);

			// if((pIVR = ivr_call_data(callref)) == NULL)
			// {
			// 	app_trace(TRACE_WARN, "pIVR == NULL");
			// 	return;
			// }

			// pbxTrunkHold(pIVR->owner, start, 0);

			// ivr_set_state(pIVR, IVR_CALL_LISTEN_MUSIC);
		}
		break;

		case CMD_PLAY_FILELOOP:
		break;

		case CMD_START_RECORD:
		break;

		case CMD_STOP_RECORD:
		break;
#endif
		case CMD_BRIDGE:
		break;
		case CMD_LOG:
		break;
		case CMD_NOCDR:
		break;
		case CMD_RINGING_T:
		break;
		case CMD_ANSWER_T:
		break;
		case CMD_BUSY:
		break;
		case CMD_CALL:
		break;
		case CMD_DISCONNECT:
		break;
		case CMD_SENDDTMF:
		break;
		case CMD_PLAYBACK:
		break;
		case CMD_PLAYTONE:
		break;
		case CMD_STOPPLAYTONE:
		break;
		case CMD_DETECTTONE:
		break;
		case CMD_HOLD_T:
		break;
		case CMD_RECORD:
		break;
		case CMD_STOPRECORD:
		break;
		case CMD_CLEAN_CALLERS:
			ivr_clean_callers();
		break;

		case CMD_MODULE_CHECK:
			/*answer from IVR module. do nothing?*/
		break;

		case CMD_MODULE_LOST:
			app_trace (TRACE_WARN, "IVR. IVR module lost!");
		break;

		default:
			app_trace(TRACE_WARN, "[%s][%d] Unknown command: %d", __func__, __LINE__, command->ivr_command);
		break;
	}
}

/*----------------------------------------------------------------------------------------------*/

static IVRCommands *ivr_command_get()
{
	return (IVRCommands *)mqueue_tryget(&ivr_proc_que);
}

/*----------------------------------------------------------------------------------------------*/

void *ivr_loop ()
{
	uint32_t cmdidx = 0;
	int budget;
	IVRCommands *new_command = NULL;

	while(ivr_running)
	{
		budget = 20;
		usleep(QUEUE_CHECK_TIME);

		while((--budget > 0) && ((new_command = ivr_command_get()) != NULL))
		{
			ivr_proc(new_command);
			ut_free_ex(new_command);

			cmdidx++;

			ivr_mod_non_response = 0;
		}

		ivrCheckMod();

		if (ivr_mod_non_response == MAX_NON_RESPONSE)
		{
			ivr_send_command(CHIPMOD, CMD_MODULE_LOST);
		} else {
			ivr_mod_non_response++;
		}
	}

	return NULL;
}

/*----------------------------------------------------------------------------------------------*/

int ivrProcessEnter(XPORT xport, int ivr_idx)
{
	XPORT srv_port;
	T_XPort *pX;
	TLinkData *ld;
	tIVRCallData *pIVR;
	int ivr_callref;

	int res = 0;

	if (!ivr_running)
		return res;

	if((pIVR = ivr_call_create(&ivr_callref, 0)) == NULL)
	{
		app_trace(TRACE_WARN, "pIVR == NULL");
		return res;
	}

	if((pX = XPortData(xport)) == NULL)
	{
		app_trace(TRACE_WARN, "pX == NULL");
		return res;
	}
	ld = &pX->link;
	ld->LDF_IVR_CALL = 1;

	res = srv_pbxSeize(&srv_port, xport, -1);
	if(res)
	{
		app_trace(TRACE_ERR, "Failed to terminate call to service");
		return res;
	}

	pIVR->owner = srv_port;
	pIVR->scr_idx = ivr_idx;

	memcpy(&pIVR->calling, &ld->calling, sizeof(ld->calling));

	//pbxAlert(callref, 1);
	pbxStopDial(srv_port, 1);

	ivr_send_caller (ivr_callref, pIVR->calling.sym, ivr_idx, CMD_SEIZE);

	srv_pbxSetIVRRef(srv_port, ivr_callref);

	ivr_set_state(pIVR, IVR_CALL_NEW);

	return res;
}

void ivrProcessStop(int callref)
{
	ivr_send_caller (callref, 0, 0, CMD_RELEASE);

	ivr_clear_call (callref);
}