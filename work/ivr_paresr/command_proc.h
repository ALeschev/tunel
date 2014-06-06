#ifndef __COMMAND_PROC_H__
#define __COMMAND_PROC_H__

#include <inttypes.h>

#define MAX_COMMAND_LEN 64
#define MAX_ARG_CNT 2
#define MAX_ARG_LEN 16

typedef struct {
	char command[MAX_COMMAND_LEN];
	uint8_t argc;

	void (*proc)(uint8_t argc, char *argv);
} command_t;

void command_init ();
void print_all_commands ();
int check_command (char *command);
command_t *get_command (int idx);

void NoCDR_proc            (uint8_t argc, char *argv);
void ResetCDR_proc         (uint8_t argc, char *argv);
void CDR_proc              (uint8_t argc, char *argv);
void Answer_proc           (uint8_t argc, char *argv);
void Busy_proc             (uint8_t argc, char *argv);
void ChanIsAvail_proc      (uint8_t argc, char *argv);
void Dial_proc             (uint8_t argc, char *argv);
void Hangup_proc           (uint8_t argc, char *argv);
void RetryDial_proc        (uint8_t argc, char *argv);
void Ringing_proc          (uint8_t argc, char *argv);
void SendDTMF_proc         (uint8_t argc, char *argv);
void Wait_proc             (uint8_t argc, char *argv);
void WaitForRing_proc      (uint8_t argc, char *argv);
void WaitMusicOnHold_proc  (uint8_t argc, char *argv);
void LookupCIDName_proc    (uint8_t argc, char *argv);
void SetCallerID_proc      (uint8_t argc, char *argv);
void SetCallerPres_proc    (uint8_t argc, char *argv);
void SetCIDName_proc       (uint8_t argc, char *argv);
void SetCIDNum_proc        (uint8_t argc, char *argv);
void SoftHangup_proc       (uint8_t argc, char *argv);
void Background_proc       (uint8_t argc, char *argv);
void BackgroundDetect_proc (uint8_t argc, char *argv);
void Milliwatt_proc        (uint8_t argc, char *argv);
void SetMusicOnHold_proc   (uint8_t argc, char *argv);
void MusicOnHold_proc      (uint8_t argc, char *argv);
void Playback_proc         (uint8_t argc, char *argv);
void Playtones_proc        (uint8_t argc, char *argv);
void Progress_proc         (uint8_t argc, char *argv);
void StopPlaytones_proc    (uint8_t argc, char *argv);
void Monitor_proc          (uint8_t argc, char *argv);
void Record_proc           (uint8_t argc, char *argv);
void StopMonitor_proc      (uint8_t argc, char *argv);

#endif //__COMMAND_PROC_H__