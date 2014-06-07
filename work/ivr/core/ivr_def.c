#include "ivr_def.h"

char *ivr_cmd_str(enum ivr_commands cmd)
{
	switch (cmd)
	{
		case CMD_SEIZE:         return "CMD_SEIZE";
		case CMD_PROGRESS:      return "CMD_PROGRESS";
		case CMD_RINGING_F:     return "CMD_RINGING_F";
		case CMD_ANSWER_F:      return "CMD_ANSWER_F";
		case CMD_HOLD_F:        return "CMD_HOLD_F";
		case CMD_RELEASE:       return "CMD_RELEASE";
		case CMD_TONEDETECTED:  return "CMD_TONEDETECTED";
		case CMD_PLAY_END:      return "CMD_PLAY_END";

		case CMD_BRIDGE:        return "CMD_BRIDGE";
		case CMD_LOG:           return "CMD_LOG";
		case CMD_NOCDR:         return "CMD_NOCDR";
		case CMD_RINGING_T:     return "CMD_RINGING_T";
		case CMD_ANSWER_T:      return "CMD_ANSWER_T";
		case CMD_BUSY:          return "CMD_BUSY";
		case CMD_CALL:          return "CMD_CALL";
		case CMD_DISCONNECT:    return "CMD_DISCONNECT";
		case CMD_SENDDTMF:      return "CMD_SENDDTMF";
		case CMD_PLAYBACK:      return "CMD_PLAYBACK";
		case CMD_PLAYTONE:      return "CMD_PLAYTONE";
		case CMD_STOPPLAYTONE:  return "CMD_STOPPLAYTONE";
		case CMD_DETECTTONE:    return "CMD_DETECTTONE";
		case CMD_HOLD_T:        return "CMD_HOLD_T";
		case CMD_RECORD:        return "CMD_RECORD";
		case CMD_STOPRECORD:    return "CMD_STOPRECORD";

		case CMD_CLEAN_CALLERS: return "CMD_CLEAN_CALLERS";
		case CMD_MODULE_CHECK:  return "CMD_MODULE_CHECK";
		case CMD_MODULE_LOST:   return "CMD_MODULE_LOST";

		case CMD_END:           return "CMD_END";
	}

	return "undef";
}

char *ivr_call_state_str(int state)
{
	switch(state)
	{
		case IVR_CALL_NULL_STATE:    return "IVR_CALL_NULL_STATE";
		case IVR_CALL_IDLE:          return "IVR_CALL_IDLE";
		case IVR_CALL_NEW:           return "IVR_CALL_NEW";
		case IVR_CALL_LISTEN_MUSIC:  return "IVR_CALL_LISTEN_MUSIC";
		case IVR_CALL_COLLECT_DIG:   return "IVR_CALL_COLLECT_DIG";
		case IVR_CALL_TALKING:       return "IVR_CALL_TALKING";
	}

	return "undef";
}