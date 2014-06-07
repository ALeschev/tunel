#ifndef IVR_PROC_H
#define IVR_PROC_H

//#include <stdint.h>
#include "messages.h"
#include "config_description.h"
#include "ivr_def.h"

#define xfree(ptr) if(ptr){free(ptr); ptr=NULL;}

typedef struct stCMD
{
	ivr_cmd cmd;
	uint32_t type;
	uint32_t id;
	message_header_t msg_hdr;
	uint32_t payload_len;
	void *payload;
} stCMD;

void app_trace(int level, char *format, ...);
void register_log(void *proc);
void unregister_log(void);
int get_ptid(void);

void StartTrace(void);
void StopTrace(void);

int ivrSetTrace(int lvl);
int ivrTrace();
int Trace(void);

#endif //IVR_PROC_H