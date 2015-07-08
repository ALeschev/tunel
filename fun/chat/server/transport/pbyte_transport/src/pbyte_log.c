#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "pbyte_log.h"
#include "pbyte_msg_api.h"

inline void pb_log_set_logger (pb_logger_t *p_logger,
                           void (*logger)(int prio, char *format, ...))
{
	if (!p_logger || !logger)
		return;

	p_logger->logger = logger;
}

/*----------------------------------------------------------------------------*/

inline void pb_log_set_prio (pb_logger_t *p_logger, int prio)
{
	if (p_logger)
		p_logger->logger_prio = prio;
}

/*----------------------------------------------------------------------------*/

inline int pb_log_check_enable (pb_logger_t *p_logger, int level)
{
	if (!p_logger || !p_logger->logger)
		return 0;

	if (p_logger->logger_prio >= level)
		return 1;

	return 0;
}

/*----------------------------------------------------------------------------*/

void pb_trace (pb_logger_t *p_logger, int level, char *format, ...)
{
	va_list	ap;
	int length, size;
	char str[4096] = {0}, *p = "";

	if (!p_logger)
		return;

	// switch(level)
	// {
	// 	case PBYTE_CRIT: p = "[CRIT] "; break;
	// 	case PBYTE_ERR:  p = "[ERR ] "; break;
	// 	case PBYTE_WARN: p = "[WARN] "; break;
	// 	case PBYTE_INFO: p = "[INFO] "; break;
	// 	case PBYTE_CUT:  p = "[CUT ] "; break;
	// 	case PBYTE_FULL: p = "[FULL] "; break;

	// 	default:         p = ""; break;
	// }

	sprintf(str, "%s %s", p_logger->prefix, p);

	length = strlen(str);
	size = sizeof(str) - length - 10;
	va_start(ap, format);
	length = vsnprintf(&str[length], size, format, ap);
	va_end(ap);

	size = strlen(str);
	while(size && (str[size-1] == '\n')) size--;
	//str[size++] = '\r';
	// str[size++] = '\n';
	str[size++] = 0;

	if (p_logger->logger)
		p_logger->logger(level, "%s", str);
	// printf ("%s", str);
}

/*----------------------------------------------------------------------------*/

#ifdef TIME_DEBUG

int pb_clockgettime(struct timeval* tv)
{
	int ret = -1;
	struct timespec ts;

	if (!tv)
		return ret;

	ret = clock_gettime(CLOCK_MONOTONIC, &ts);

	if (ret == 0)
	{
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;

		if (ts.tv_nsec % 1000 >= 500)
		{
			if (++tv->tv_usec == 1000000)
			{
				tv->tv_usec = 0;
				++tv->tv_sec;
			}
		}
	}

	return ret;
}

const char *pb_pb_msg_type_to_str(pb_msg_type_t type);

void pb_print_msg_time(pb_logger_t *p_logger, char *descript, pb_msg_t *pb_msg)
{
	struct timeval diff;
	int log_level = PBYTE_INFO;
	char identity_str[256] = {0};

	if (!pb_msg)
		return;

	if (pb_msg->message_params.msg_type != TRANSFER)
		return;

	pb_clockgettime(&pb_msg->finish);

	timersub(&pb_msg->finish, &pb_msg->start, &diff);

	if ((diff.tv_sec > 1) && diff.tv_usec)
	{
		log_level = PBYTE_WARN;
	}

	pb_pb_identity_to_str(&pb_msg->identity, 1, identity_str, sizeof (identity_str));

	if (pb_log_check_enable(p_logger, log_level))
	{
		pb_trace (p_logger, log_level, "%s Time debug: '%s': '%s' (%ld.%06ld sec)",
		          (descript)? descript:"",
		          identity_str,
		          pb_pb_msg_type_to_str(pb_msg->message_params.msg_type),
		          diff.tv_sec, diff.tv_usec);
	}
}

#endif

/*----------------------------------------------------------------------------*/
