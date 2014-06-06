#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h> 
#include <unistd.h>
#include <execinfo.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

#include <stdint.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/socket.h>

#include <linux/types.h>
#include <linux/kdev_t.h>
#include <stdint.h>
#include <dlfcn.h>
#include <asm/byteorder.h>
#include <time.h>
#include <dirent.h>

#include "config_description.h"
#include "header.h"
#include "handlers.h"
#include "sysmodules.h"
#include "object_info.h"
#include "uemf.h"
#include "mqueue.h"

#include "ivr_def.h"
#include "ivr_proc.h"
#include "methods.h"

/*typedef struct _ivr_call
{
	tIVRData data;
} ivr_data_t;

static ivr_data_t *ivr_data = NULL;*/

static mqueue_t configCmdQue;
static volatile int sys_running = 0;
static uint8_t ivr_mod_non_response = 0;

void HandlerSigterm(int x);
int MessageCmdAdd(message_t *message, eCMD cmd);

void app_trace(int level, char *format, ...)
{
	va_list	ap;
	int length, size;
	char str[4096], timestamp[100], *p;
	struct timeval tp;
	struct tm *pt;
	time_t t;

	gettimeofday(&tp, NULL);
	t = (time_t)tp.tv_sec;
	pt = localtime(&t);
	sprintf(timestamp, "% 2d:%02d:%02d.%06d", pt->tm_hour, pt->tm_min, pt->tm_sec, (int)tp.tv_usec);

	switch(level){
		case TRACE_INFO:   p="[INFO] IVR:"; break;
		case TRACE_ERR:    p="[ERR ] IVR:";  break;
		case LOG_WARNING:  p="[WARN] IVR:";  break;
		case -1:           p="[    ] IVR:";  break;
		case TRACE_CON2:   p = " "; break;
		case TRACE_NOTIME: p = " "; break;

		default:	   p="[????] IVR:"; break;
	}

	if(level != TRACE_NOTIME)
	{
		sprintf(str, "%s  %s  ", timestamp, p);
	}else{
		sprintf(str, "%s", p);
	}

	length = strlen(str);
	size = sizeof(str) - length - 10;

	va_start(ap, format);
	length = vsnprintf(&str[length], size, format, ap);
	va_end(ap);

	size = strlen(str);
	while(size && (str[size-1] == '\n')) size--;

	//str[size++] = '\r';
	str[size++] = '\n';
	str[size++] = 0;

	printf("%s", str);

	if (level == TRACE_ERR)
		fputs(str, stderr);
}

void ivr_init(void)
{
//	int callref;

	// if (ivr_running)
	// 	return -1;

/*	app_trace(TRACE_INFO, "ivr_mod: init.");

	ivr_data = (ivr_data_t *) malloc ( sizeof(ivr_data_t) * (IVR_CALLREF_MOD_MAX) );

	if(NULL == ivr_data)
		return;

	memset(ivr_data, 0, sizeof(ivr_data_t) * (IVR_CALLREF_MOD_MAX));

	for(callref = IVR_MOD_CALLREF_FIRST; callref < IVR_MOD_CALLREF_LAST; callref++)
	{
		tIVRData *pIVR = &ivr_data[callref].data;

		pIVR->CState = IVR_CALL_NULL_STATE;
	}
*/

	if (ivr_mutex_init())
		return;

	mqueue_init(&configCmdQue, MQUEUESIZE);
	signal(SIGTERM, HandlerSigterm);

	//Registration
	if(lib_IPC_init(IVRMOD))
	{
		app_trace(TRACE_ERR, "failed to init IPC!");
		return;
	}

	sys_running = 1;

	return;
}

void ivr_close(void)
{
	// if (!ivr_running)
	// 	return;

	ivr_rem_all_caller();

//	if(ivr_data != NULL) {xfree (ivr_data); ivr_data = NULL;}

	ivr_mutex_destroy();

	lib_IPC_deinit();
	mqueue_close(&configCmdQue);

	sys_running = 0;
}

void HandlerSigterm(int x)
{
	app_trace(TRACE_WARN,"[%s][%d] term-signal!", __func__, __LINE__);

	sys_running = 0;
}

int MessageCmdAdd(message_t *message, eCMD cmd)
{
	//char *answ;
	int res = -1;

	message_header_t *msg_hdr;
	msg_hdr = &message->header;
	int param = msg_hdr->param;

	stCMD *newCMD = calloc(1 , sizeof(stCMD));
	if (newCMD)
	{
		newCMD->cmd = cmd;
		newCMD->type = (param >> 16)&0xFF;
		newCMD->id = param&0xFFFF;

		memcpy(&newCMD->msg_hdr, msg_hdr, sizeof(message_header_t));

		if (msg_hdr->len)
		{
			newCMD->payload_len = msg_hdr->len;
			newCMD->payload = calloc(1, msg_hdr->len);
			if (newCMD->payload)
			{
				memcpy(newCMD->payload, message->payload, msg_hdr->len);
			}
		}

		if(mqueue_put(&configCmdQue, newCMD, 0))
		{
			xfree(newCMD);

			res = 1;
		}else{
			res = 0;
		}
	}

	return 0;
}

int MessageHandlerUpdate(message_t *message)
{
	IVRCommands *newComm = (IVRCommands *)message->payload;

	if (!newComm)
		return 0;

	return MessageCmdAdd(message, newComm->ivr_command);
}

stCMD *CmdGet()
{
	return (stCMD *)mqueue_tryget(&configCmdQue);
}

static void CmdProc(stCMD * newCMD)
{
	if (newCMD->cmd != CMD_MODULE_CHECK)
		app_trace(TRACE_INFO, "[%s][%d] Proc cmd: '%s' (%d)", __func__, __LINE__, ivr_cmd_str(newCMD->cmd), newCMD->cmd);

	IVRCommands *comnd = (IVRCommands *)newCMD->payload;

	int callref = comnd->callref;

	switch (comnd->ivr_command)
	{
		case CMD_ADD_CALLER:
			ivr_add_caller_proc (callref, (caller_t *)comnd->attribute);
		break;

		case CMD_DEL_CALLER:
			ivr_del_caller_proc (callref);
		break;

		case CMD_COLLECT_DIG:
			ivr_collect_digit_proc (callref, (digits_t *)comnd->attribute);
		break;

		case CMD_PLAY_FILE_END:
			ivr_play_file_stop_proc (callref);
		break;

		case CMD_CLEAN_CALLERS:
			ivr_rem_all_caller();
		break;

		case CMD_MODULE_CHECK:
			ivr_send_command (CHIPMOD, CMD_MODULE_CHECK);
		break;

		default:
			app_trace(TRACE_WARN, "[%s][%d] Unknown cmd: %d", __func__, __LINE__, newCMD->cmd);
		break;
	}
}

static void CmdLoop(void)
{
	uint32_t cmdidx = 0;
	int budget;
	stCMD * newCMD = NULL;

	while(sys_running)
	{
		budget = 20;
		usleep(QUEUE_CHECK_TIME);

		while((--budget > 0) && ((newCMD = CmdGet()) != NULL))
		{
			CmdProc(newCMD);
			xfree(newCMD);

			cmdidx++;
			ivr_mod_non_response = 0;
		}

		if (ivr_mod_non_response >= MAX_NON_RESPONSE)
		{
			app_trace (TRACE_WARN, "MGAPP: lost!");
			ivr_rem_all_caller();
			ivr_mod_non_response = 0;
		} else {
			ivr_mod_non_response++;
		}
	}
}

int main(void)
{
	ivr_init();

	if (!sys_running)
		goto force_exit;

	app_trace(TRACE_INFO, "[%s][%d] Start IVR module", __func__, __LINE__);

	message_handler_add(CMD_IVR, MessageHandlerUpdate);

	CmdLoop();

	ivr_send_command (CHIPMOD, CMD_MODULE_LOST);

force_exit:
	ivr_close();

	app_trace(TRACE_INFO, "[%s][%d] Exit IVR module", __func__, __LINE__);

	return 0;
}