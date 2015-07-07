#include <stdio.h>
#include <time.h>
#include <sys/syscall.h>

#include "log.h"

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
        case eLOG_INFO:  p = "[INFO ] "; break;
        case eLOG_DEBUG: p = "[DEBUG] "; break;
        case eLOG_WARN:  p = "[WARN ] "; break;
        case eLOG_ERR:   p = "[ERR  ] "; break;
        default:         p = "[?????] "; break;
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

    printf ("%s", str);
}
