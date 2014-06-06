#ifndef IVR_DEF_H
#define IVR_DEF_H

#include <inttypes.h>

#define MAX_NON_RESPONSE 10
#define QUEUE_CHECK_TIME 100000 /*100ms*/

#define IVR_CALLREF_MAX 2048

#define IVR_CALLREF_MOD_MAX IVR_CALLREF_MAX/2

#define MAX_DIG_COLLECT 32

typedef enum ivr_proc_commands {
	CMD_ADD_CALLER = 0,
	CMD_DEL_CALLER,
	CMD_COLLECT_DIG,
	CMD_MAKE_CALL,
	CMD_PLAY_FILE,
	CMD_PLAY_FILE_END,
	CMD_PLAY_FILELOOP,

	CMD_START_RECORD,
	CMD_STOP_RECORD,

	CMD_CLEAN_CALLERS,

	CMD_MODULE_CHECK,
	CMD_MODULE_LOST,

	CMD_END
}eCMD;

enum ivr_call_state {
	IVR_CALL_NULL_STATE = -1,
	IVR_CALL_IDLE,
	IVR_CALL_NEW,
	IVR_CALL_LISTEN_MUSIC,
	IVR_CALL_COLLECT_DIG,
	IVR_CALL_TALKING
};

typedef struct {
	int ivr_idx;
	int state;

	int     length;
	uint8_t sym[MAX_DIG_COLLECT];
} caller_t;

typedef struct {
	char file_name[64];
} melody_t;

typedef struct {
	uint8_t digit;
} digits_t;

typedef struct {
	char to[MAX_DIG_COLLECT];
} call_to;

typedef struct {
	int callref;
	uint8_t ivr_command;
	uint8_t attribute[];
} IVRCommands;

#endif //IVR_DEF_H