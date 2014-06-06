#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_description.h"
#include "sysmodules.h"
#include "methods.h"
#include "messages.h"
#include "header.h"
#include "handlers.h"
#include "ivr_proc.h"
#include "ivr_caller_tree.h"

static int ivr_callref_static = IVR_CALLREF_MOD_MAX;
static pthread_mutex_t ivr_data_mutex;

char *ivr_cmd_str(enum ivr_proc_commands cmd)
{
	switch (cmd)
	{
		case CMD_ADD_CALLER:    return "CMD_ADD_CALLER";
		case CMD_DEL_CALLER:    return "CMD_DEL_CALLER";
		case CMD_COLLECT_DIG:   return "CMD_COLLECT_DIG";
		case CMD_MAKE_CALL:     return "CMD_MAKE_CALL";
		case CMD_PLAY_FILE:     return "CMD_PLAY_FILE";
		case CMD_PLAY_FILE_END: return "CMD_PLAY_FILE_END";
		case CMD_PLAY_FILELOOP: return "CMD_PLAY_FILELOOP";
		case CMD_START_RECORD:  return "CMD_START_RECORD";
		case CMD_STOP_RECORD:   return "CMD_STOP_RECORD";
		case CMD_CLEAN_CALLERS: return "CMD_CLEAN_CALLERS";
		case CMD_MODULE_CHECK:  return "CMD_MODULE_CHECK";
		case CMD_MODULE_LOST:   return "CMD_MODULE_LOST";

		case CMD_END:           return "CMD_END";
	}

	return "undef";
}

/*----------------------------------------------------------------------------------------------*/

char *ivr_state_str(int state)
{
	static char str[20];

	switch(state){
		case IVR_CALL_NULL_STATE:    return "IVR_CALL_NULL_STATE";
		case IVR_CALL_IDLE:          return "IVR_CALL_IDLE";
		case IVR_CALL_NEW:           return "IVR_CALL_NEW";
		case IVR_CALL_LISTEN_MUSIC:  return "IVR_CALL_LISTEN_MUSIC";
		case IVR_CALL_COLLECT_DIG:   return "IVR_CALL_COLLECT_DIG";
		case IVR_CALL_TALKING:       return "IVR_CALL_TALKING";
	}

	sprintf(str, "_undef_%02X_", state);
	return str;
}

int ivr_mutex_init()
{
	if((pthread_mutex_init(&ivr_data_mutex, NULL)))
	{
		app_trace(TRACE_ERR, "ivr: mutex create error\r\n");
		return 1;
	}

	return 0;
}

int ivr_mutex_destroy()
{
	return pthread_mutex_destroy (&ivr_data_mutex);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_show_caller_tree()
{
	app_trace(TRACE_INFO, "\n---------------SHOW TREE START---------------\n");
	tree_print();
	app_trace(TRACE_INFO, "\n----------------SHOW TREE END----------------\n");
}

/*----------------------------------------------------------------------------------------------*/

void ivr_add_caller (int callref, caller_t *new_call)
{
	if (!new_call)
		return;

	tree_add_caller(callref, new_call);

	// printf ("---caller %04x added---\n", new_call->callref);
	// show_caller_tree();
	// printf ("-----------------------\n");
}

/*----------------------------------------------------------------------------------------------*/

int ivr_get_caller_count ()
{
	return tree_get_caller_cnt ();
}

/*----------------------------------------------------------------------------------------------*/

void ivr_del_caller (int callref)
{
	tree_del_caller (callref);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_set_caller_state (int callref, int state)
{
	int prev_state = tree_get_state (callref);

	if (prev_state != state)
	{
		app_trace (TRACE_INFO, "[%s][%d] caller %04x changed state: %s -> %s",
		           __func__, __LINE__, callref, ivr_state_str(prev_state), ivr_state_str(state));
	}

	tree_set_state (callref, state);
}

/*----------------------------------------------------------------------------------------------*/

int ivr_get_caller_state (int callref)
{
	return tree_get_state(callref);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_need_digit (int callref, int needed)
{
	if (tree_set_gitits_need (callref, needed))
		return;

	ivr_set_caller_state (callref, IVR_CALL_COLLECT_DIG);
}

/*----------------------------------------------------------------------------------------------*/

int ivr_set_digit_to_caller (int callref, uint8_t dig)
{
	int res = tree_set_digit (callref, dig);

	// switch (res)
	// {
	// 	case 0: app_trace (TRACE_INFO, "[%s][%d] callerf %04x: this was last one.", __func__, __LINE__, callref); break;
	// 	case 1: app_trace (TRACE_INFO, "[%s][%d] callerf %04x: need moooore digits!", __func__, __LINE__, callref); break;
	// 	case 2: app_trace (TRACE_INFO, "[%s][%d] callref %04x: not found\n", __func__, __LINE__, callref);         break;
	// }

	return res;
}

/*----------------------------------------------------------------------------------------------*/

void ivr_rem_all_caller(void)
{
	if (ivr_get_caller_count())
	{
		app_trace(TRACE_WARN, "Clean all callers!");
		tree_clean_all();
	}
}

/*----------------------------------------------------------------------------------------------*/

int ivr_get_next_callref (void)
{
	int callref = 0;
	int i = 0;
	int state = -1;

	pthread_mutex_lock(&ivr_data_mutex);

	for( i = ivr_callref_static; i < IVR_CALLREF_MAX; i++)
	{
		callref = i;

		state = ivr_get_caller_state(callref);

		if(state == -2)
		{
			if(ivr_callref_static == i)
			{
				ivr_callref_static = i+1;
			}else{
				ivr_callref_static = i;
			}

			pthread_mutex_unlock(&ivr_data_mutex);

			return callref;
		}
	}

	for(i = IVR_CALLREF_MOD_MAX; i < ivr_callref_static; i++)
	{
		callref = i;

		state = ivr_get_caller_state(callref);

		if(state == -2)
		{
			ivr_callref_static = i + 1;

			pthread_mutex_unlock(&ivr_data_mutex);

			return callref;
		}
	}

	pthread_mutex_unlock(&ivr_data_mutex);

	return -1;
}

/*----------------------------------------------------------------------------------------------*/

void ivr_send_message (int module, int command, const char *_data, int size)
{
	char *answer;
	IVRCommands *to_ivr;
	uint8_t *payload;
	int payload_sz;

	payload_sz = sizeof(IVRCommands) + size;
	payload = calloc(1, payload_sz);

	to_ivr = (IVRCommands*)payload;
	to_ivr->ivr_command = command;

	memcpy (&payload[sizeof(IVRCommands)], _data, size);

	send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, module, 0, payload_sz, (char *)payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		xfree(answer);
		answer = NULL;
	}

	xfree(payload);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_send_command (int module, int command)
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
		xfree(answer);
		answer = NULL;
	}

	xfree(payload);
}

/*----------------------------------------------------------------------------------------------*/

int ivr_validate_digit_ok (int callref, char *base)
{
	if (!base)
		return 0;

	char s_digits[MAX_DIG_COLLECT];
	strcpy (s_digits, tree_get_digits (callref));

	return !memcmp(s_digits, base, strlen(base));
}

/*----------------------------------------------------------------------------------------------*/

void ivr_add_caller_proc (int callref, caller_t *new_call)
{
	if (!new_call)
		return;

	app_trace(TRACE_INFO, "[%d] callref %04x (%s[%d]) added", new_call->ivr_idx, callref, (char *)new_call->sym, new_call->length);

	ivr_add_caller (callref, new_call);

	// char *sym = tree_get_caller_sym (callref);

	// if (sym)
	// 	app_trace (TRACE_INFO, "caller %04x have sym: %s[%d]", callref, sym, new_call->length);
	// else
	// 	app_trace(TRACE_WARN, "can't get sym: caller %04x");

	ivr_set_caller_state(callref, IVR_CALL_NEW);

	ivr_need_digit (callref, 5);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_del_caller_proc (int callref)
{
	ivr_del_caller (callref);

	if (ivr_get_caller_count() == 0)
		ivr_send_command (CHIPMOD, CMD_CLEAN_CALLERS);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_palay_file_proc (int callref, int start)
{
	char *answer;
	uint8_t *payload;
	int payload_sz;

	IVRCommands *to_ivr;
	melody_t for_play;

	if (start < 0 || start > 1)
	{
		app_trace(TRACE_INFO, "[%s][%d] invalid 'start' param", __func__, __LINE__);
		return;
	}

	payload_sz = sizeof(IVRCommands) + sizeof (melody_t);// * start;
	payload = calloc(1, payload_sz);

	if(!payload)
	{
		app_trace (TRACE_ERR, "[%s][%d] can't allocate memory", __func__, __LINE__);
		return;
	}

	to_ivr = (IVRCommands*)payload;
	to_ivr->ivr_command = CMD_PLAY_FILE;
	to_ivr->callref = callref;

	if (start)
		strcpy(for_play.file_name, "wait.wav");
	else
		memset(for_play.file_name, 0, sizeof(for_play.file_name));

	memcpy(&payload[sizeof(IVRCommands)], &for_play, sizeof (melody_t));

	send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, CHIPMOD, 0, payload_sz, (char *)payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		xfree(answer);
		answer = NULL;
	}

	ivr_set_caller_state (callref, IVR_CALL_LISTEN_MUSIC);

	xfree (payload);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_play_file_stop_proc (int callref)
{
	ivr_set_caller_state (callref, IVR_CALL_NEW);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_make_call_proc (int callref, char *call_num)
{
	if (!call_num)
		return;

	char *answer;
	uint8_t *payload;
	int payload_sz;

	IVRCommands *to_ivr;
	call_to called;

	if (ivr_get_caller_state(callref) == IVR_CALL_TALKING)
		return;

	payload_sz = sizeof(IVRCommands) + sizeof (call_to);
	payload = calloc(1, payload_sz);

	to_ivr = (IVRCommands*)payload;
	to_ivr->ivr_command = CMD_MAKE_CALL;
	to_ivr->callref = callref;

	strncpy (called.to, call_num, MAX_DIG_COLLECT);

	memcpy(&payload[sizeof(IVRCommands)], &called, sizeof (call_to));

	send_message(CMD_IVR, 0, 0, MESSAGE_TYPE_EVENT, CHIPMOD, 0, payload_sz, (char *)payload, &answer, CONFMOD_REPLY_TO);

	if (answer) {
		xfree(answer);
		answer = NULL;
	}

	ivr_set_caller_state (callref, IVR_CALL_TALKING);

	xfree (payload);
}

/*----------------------------------------------------------------------------------------------*/

void ivr_collect_digit_proc (int callref, digits_t *dig)
{
	if (!dig)
		return;

	if (ivr_set_digit_to_caller (callref, dig->digit) == 0)
	{
		// if (validate_digit_ok (callref, "55251"))
		// 	palay_melody (callref, 1);

		// if (validate_digit_ok (callref, "55250"))
		// 	palay_melody (callref, 0);

		if (ivr_validate_digit_ok (callref, "41015"))
		{
			ivr_make_call_proc (callref, "41015");
		} else {
			ivr_need_digit (callref, 5);
		}
	}
}