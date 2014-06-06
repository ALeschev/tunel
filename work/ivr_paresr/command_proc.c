#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "command_proc.h"

static enum command {
	NoCDR = 0,         /*Указывает Asterisk'у не сохранять CDR запись для вызова.*/
	ResetCDR,          /*Сброс данных CDR.*/
	CDR,               /*Добавить пользовательские данные к существующим в записи CDR.*/

	Answer,            /*Ответ на звонок, если по каналу поступает вызов*/
	Busy,              /*Установить состояние "занято" и ждать окончания соединения*/
	ChanIsAvail,       /*Проверка на доступность канала связи*/
	Dial,              /*Совершить вызов и, в случае успеха, соединить вызываемого с текущим каналом*/
	Hangup,            /*Безусловное разъединение соединения*/
	RetryDial,         /*Совершить вызов заданного екстеншена, повторять при неудачной попытке вызова, вызывающий пользователь
                         может закончить процедуру вызова, нажав кнопку на dtmf клавиатуре.*/
	Ringing,           /*Установить состояние "вызова абонента" (звонка)*/
	SendDTMF,          /*Отправка в канал произвольной последовательности DTMF цифр*/
	Wait,              /*Пауза на заданное время*/
	WaitForRing,       /*Ждать состояния "вызова абонента" (звонка)*/
	WaitMusicOnHold,   /*Ожидание с проигрышом музыки (Music On Hold)*/

	LookupCIDName,     /*Поиск Имени CallerID в локальной базе данных*/
	SetCallerID,       /*Установка CallerID.*/
	SetCallerPres,     /*Изменяет режим публикации для callerid (используя текстовые значения)*/
	SetCIDName,        /*Установка имени в CallerID.*/
	SetCIDNum,         /*Установка только номера в Caller ID (не имени).*/
	SoftHangup,        /*Требует разрыва связи на заданном канале*/

	Background,        /*Проигрывает звуковой файл, в это время могут исполняться другие команды.*/
	BackgroundDetect,  /*Аналогично команде Background с возможностью определения разговора.*/
	Milliwatt,         /*Генерация чистого тона 1000Hz с уровнем 0dbm (mu-law).*/
	SetMusicOnHold,    /*Установка музыки ожидания (Music On Hold) для текущего канала.*/
	MusicOnHold,       /*Проигрыш музыки ожидания (Music On Hold), неопределенно долго.*/
	Playback,          /*Проигрывает звуковой файл.*/
	Playtones,         /*Проигрывает список тонов, в это время могут исполняться другие команды.*/
	Progress,          /*Функция, дающая возможность проигрывания звукового файла вызывающему абоненту до момента ответа на вызов (перевода линии в отвеченное состояние).*/
	StopPlaytones,     /*Останавливает проигрыш списка тонов.*/

	Monitor,           /*Запись телефонного разговора в звуковой файл*/
	Record,            /*Запись телефонного разговора в звуковой файл*/
	StopMonitor,       /*Останов записи телефонного разговора*/

	CMD_LAST
};

static char command_name[][MAX_COMMAND_LEN] = {
	{"NoCDR"},
	{"ResetCDR"},
	{"CDR"},
	{"Answer"},
	{"Busy"},
	{"ChanIsAvail"},
	{"Dial"},
	{"Hangup"},
	{"RetryDial"},
	{"Ringing"},
	{"SendDTMF"},
	{"Wait"},
	{"WaitForRing"},
	{"WaitMusicOnHold"},
	{"LookupCIDName"},
	{"SetCallerID"},
	{"SetCallerPres"},
	{"SetCIDName"},
	{"SetCIDNum"},
	{"SoftHangup"},
	{"Background"},
	{"BackgroundDetect"},
	{"Milliwatt"},
	{"SetMusicOnHold"},
	{"MusicOnHold"},
	{"Playback"},
	{"Playtones"},
	{"Progress"},
	{"StopPlaytones"},
	{"Monitor"},
	{"Record"},
	{"StopMonitor"}
};

static uint8_t command_argc[CMD_LAST] = {
	0, /*NoCDR*/
	1, /*ResetCDR*/
	1, /*CDR*/
	0, /*Answer*/
	0, /*Busy*/
	0, /*ChanIsAvail*/ /*?*/
	4, /*Dial*/ /*?*/
	0, /*Hangup*/
	3, /*RetryDial*/
	0, /*Ringing*/
	2, /*SendDTMF*/
	1, /*Wait*/
	1, /*WaitForRing*/
	1, /*WaitMusicOnHold*/
	1, /*LookupCIDName*/
	1, /*SetCallerID*/
	1, /*SetCallerPres*/
	1, /*SetCIDName*/
	1, /*SetCIDNum*/
	1, /*SoftHangup*/
	1, /*Background*/
	1, /*BackgroundDetect*/
	0, /*Milliwatt*/
	1, /*SetMusicOnHold*/
	0, /*MusicOnHold*/
	2, /*Playback*/
	1, /*Playtones*/
	1, /*Progress*/
	0, /*StopPlaytones*/
	1, /*Monitor*/
	2, /*Record*/
	0, /*StopMonitor*/
};

static void *command_proc[CMD_LAST] = {
	&NoCDR_proc,
	&ResetCDR_proc,
	&CDR_proc,
	&Answer_proc,
	&Busy_proc,
	&ChanIsAvail_proc,
	&Dial_proc,
	&Hangup_proc,
	&RetryDial_proc,
	&Ringing_proc,
	&SendDTMF_proc,
	&Wait_proc,
	&WaitForRing_proc,
	&WaitMusicOnHold_proc,
	&LookupCIDName_proc,
	&SetCallerID_proc,
	&SetCallerPres_proc,
	&SetCIDName_proc,
	&SetCIDNum_proc,
	&SoftHangup_proc,
	&Background_proc,
	&BackgroundDetect_proc,
	&Milliwatt_proc,
	&SetMusicOnHold_proc,
	&MusicOnHold_proc,
	&Playback_proc,
	&Playtones_proc,
	&Progress_proc,
	&StopPlaytones_proc,
	&Monitor_proc,
	&Record_proc,
	&StopMonitor_proc,
};

static command_t command_list[CMD_LAST];

void command_init ()
{
	int i;

	memset (command_list, 0, sizeof (command_t));

	for (i = 0; i < CMD_LAST; i++)
	{
		strncpy (command_list[i].command, command_name[i], MAX_COMMAND_LEN);
		command_list[i].argc = command_argc[i];
		command_list[i].proc = command_proc[i];
	}
}

/*-----------------------------------------------------------------*/

void print_all_commands ()
{
	int i;

	for (i = 0; i < CMD_LAST; i++)
		printf ("%s (%d)\n", command_name[i], command_argc[i]);
}

/*-----------------------------------------------------------------*/

int check_command (char *command)
{
	int i;
	int idx = -1;

	if (!command)
		return idx;

	printf ("try find command: %s\n", command);

	for (i = 0; i < CMD_LAST; i++)
	{
		if (!strcmp (command_name[i], command))
		{
			idx = i;
			break;
		}
	}

	return idx;
}

/*-----------------------------------------------------------------*/

command_t *get_command (int idx)
{
	if (idx < 0 || idx >= CMD_LAST)
		return NULL;

	return &command_list[idx];
}

/*-----------------------------------------------------------------*/

char **check_args (uint8_t argc, char *argv, int *args_c)
{
	static char *args[MAX_ARG_LEN];
	int i;

	char *str1, *token;
	char *saveptr1;

	for (i = 0, str1 = argv; i < argc; i++, str1 = NULL)
	{
		token = strtok_r(str1, " ", &saveptr1);

		if (token == NULL)
			break;

		args[i] = token;
	}

	if (argc != i)
	{
		*args_c = -1;
		return NULL;
	}

	*args_c = i;

	return args;
}

/*-----------------------------------------------------------------*/

void args_proc (const char *f_name, uint8_t argc, char *argv)
{
	char **arg_s;
	int args;

	if (!f_name)
		return;

	arg_s = check_args (argc, argv, &args);

	if (args < 0)
	{
		printf ("arg error\n");
		return;
	}

	int i;
	for (i = 0; i < argc; i++)
	{
		if (arg_s[i] && strlen(arg_s[i]))
			printf ("[%d][%s][%s]: arg %d: %s\n", __LINE__, f_name, __func__, i, arg_s[i]);
	}
}

/*-----------------------------------------------------------------*/

void NoCDR_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void ResetCDR_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void CDR_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Answer_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Busy_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void ChanIsAvail_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Dial_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Hangup_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void RetryDial_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Ringing_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SendDTMF_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Wait_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void WaitForRing_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void WaitMusicOnHold_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void LookupCIDName_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SetCallerID_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SetCallerPres_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SetCIDName_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SetCIDNum_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SoftHangup_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Background_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void BackgroundDetect_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Milliwatt_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void SetMusicOnHold_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void MusicOnHold_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Playback_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Playtones_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Progress_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void StopPlaytones_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Monitor_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void Record_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/

void StopMonitor_proc (uint8_t argc, char *argv)
{
	printf("[%d][%s]: argc %d: %s\n", __LINE__, __func__, argc, argv);

	args_proc (__func__, argc, argv);
}

/*-----------------------------------------------------------------*/