#ifndef IVR_DEF_H
#define IVR_DEF_H

#include <inttypes.h>

#define MAX_NON_RESPONSE 10
#define QUEUE_CHECK_TIME 100000 /*100ms*/

#define IVR_CALLREF_MAX 2048

#define IVR_CALLREF_MOD_MAX IVR_CALLREF_MAX/2

#define MAX_DIG_COLLECT 32

typedef enum ivr_commands {
	/*APP->IVR*/
	CMD_SEIZE = 0,
	CMD_PROGRESS,
	CMD_RINGING_F,
	CMD_ANSWER_F,
	CMD_HOLD_F,
	CMD_RELEASE,
	CMD_TONEDETECTED,

	CMD_PLAY_END,

	/*IVR->APP*/
	CMD_BRIDGE,       /*Соединение двух произвольных каналов.*/
	CMD_LOG,          /*Вносит произвольный текст в файл(ы) лога сервера Asterisk.*/

	CMD_NOCDR,        /*Указывает Asterisk'у не сохранять CDR запись для вызова.*/

	CMD_RINGING_T,      /*Установить состояние "вызова абонента" (звонка)*/
	CMD_ANSWER_T,       /*Ответ на звонок, если по каналу поступает вызов*/
	CMD_BUSY,         /*Установить состояние "занято" и ждать окончания соединения*/

	CMD_CALL,         /*Совершить вызов и, в случае успеха, */
	CMD_DISCONNECT,   /*Безусловное разъединение соединения*/

	CMD_SENDDTMF,     /*Отправка в канал произвольной последовательности DTMF цифр*/

	CMD_PLAYBACK,     /*Проигрывает звуковой файл. (+ готовность обработать донабор)*/
	CMD_PLAYTONE,     /*Проигрывает список тонов (1..N), в это время могут исполняться другие команды.*/
	CMD_STOPPLAYTONE, /*Останавливает проигрыш списка тонов.*/
	CMD_DETECTTONE,   /*Запуск детектора тонов DTMF, Modem, Fax, Flash*/
	CMD_HOLD_T,       /*Выдача Hold по сигнализации*/

	CMD_RECORD,       /*Запись телефонного разговора в звуковой файл*/
	CMD_STOPRECORD,   /*Останов записи телефонного разговора*/

	CMD_CLEAN_CALLERS,

	/*general*/
	CMD_MODULE_CHECK,
	CMD_MODULE_LOST,

	CMD_END
} ivr_cmd;

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

char *ivr_cmd_str(enum ivr_commands cmd);
char *ivr_call_state_str(int state);

#endif //IVR_DEF_H