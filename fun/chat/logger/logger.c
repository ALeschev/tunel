/*
 * LAV by fun
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>

#include "logger.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KGRY  "\x1B[0;30m"
#define KWHT  "\x1B[37m"

#define KLBLU "\x1B[1;34m"
#define KLGRN "\x1B[1;32m"
#define KLCYN "\x1B[1;36m"
#define KLRED "\x1B[1;31m"
#define KLMAG "\x1B[1;35m"
#define KBRWN "\x1B[0;33m"
#define KLYEL "\x1B[1;33m"

#define KBLINK "\x1B[5m"

static int log_level = eLOG_FULL;
static con_out_func con_func = NULL;

void log_print (int level, char *format, ...);

int log_check_enable(int level)
{
    if (log_level < level)
        return 0;

    return 1;
}

void log_set_prio(int level)
{
    if ((eLOG_CRIT <= level) && (level < eLOG_MAX))
    {
        log_print(eLOG_INFO, "Set trace level '%s'", e_log_level_str[level]);

        log_level = level;

        return;
    }

    if (level < eLOG_CRIT)
        log_level = eLOG_CRIT;

    if (level >= eLOG_MAX)
        log_level = eLOG_FULL;

    log_print(eLOG_ERR, "Set trace level error: unknown level '%d' set '%s'",
              level, e_log_level_str[log_level]);
}

void log_print(int level, char *format, ...)
{
    va_list ap;
    int length, size;
    char str[4096] = {0}, *p = "";
    char timestamp[100] = {0};
    struct timeval tp;
    struct tm *pt;
    time_t t;

    gettimeofday(&tp, NULL);
    t = (time_t)tp.tv_sec;
    pt = localtime(&t);
    sprintf(timestamp, " %02d:%02d:%02d.%06lu",
            pt->tm_hour, pt->tm_min, pt->tm_sec, tp.tv_usec);

    switch(level)
    {
        case eLOG_CRIT: p = "[" KBLINK KLRED "CRIT" KNRM "] "; break;
        case eLOG_ERR:  p = "[" KBLINK KLRED "ERR " KNRM "] "; break;
        case eLOG_WARN: p = "[" KLYEL "WARN" KNRM "] "; break;
        case eLOG_INFO: p = "[" KGRN  "INFO" KNRM "] "; break;
        case eLOG_CUT:  p = "[" KLCYN "CUT " KNRM "] "; break;
        case eLOG_FULL: p = "[" KLBLU "FULL" KNRM "] "; break;
        default:        p = "[" KWHT  "????" KNRM "] "; break;
    }

    sprintf(str, "%s %ld %s", timestamp, syscall(SYS_gettid), p);

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

    if (con_func)
    {
        con_func (str);
    }
    else
    {
        printf ("%s", str);
    }
}

void log_register_logger(con_out_func logger)
{
    con_func = logger;
}

void log_deregister_logger(void)
{
    con_func = NULL;
}
