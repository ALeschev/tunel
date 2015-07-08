/*
 * LAV by fun
 */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>

#include "log.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void log_print (int level, char *format, ...)
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
        case eLOG_INFO:  p = "[" KGRN"INFO "KNRM "] "; break;
        case eLOG_DEBUG: p = "[" KMAG"DEBUG"KNRM "] "; break;
        case eLOG_WARN:  p = "[" KYEL"WARN "KNRM "] "; break;
        case eLOG_ERR:   p = "[" KRED"ERR  "KNRM "] "; break;
        default:         p = "[" KWHT"?????"KNRM "] "; break;
    }

    sprintf(str, "%s %d %s", timestamp, syscall(SYS_gettid), p);

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

    printf ("%s", str);
}
