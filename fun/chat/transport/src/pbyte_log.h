#ifndef _PBYTE_LOG_H_
#define _PBYTE_LOG_H_

#include "logger.h"

#define pb_log(base, prio, format, ...) \
              do { \
                if (base && pb_log_check_enable(&base->logger, prio))\
                {\
                  pb_trace(&base->logger, prio, format, ##__VA_ARGS__); \
                }\
              }while(0)

#define pb_logger(logger, prio, format, ...) \
              do { \
                if (logger && pb_log_check_enable(logger, prio))\
                {\
                  pb_trace(logger, prio, format, ##__VA_ARGS__); \
                }\
              }while(0)

// enum PBYTE_LOG_PRIOS { PBYTE_CRIT = 0,
//                        PBYTE_ERR,
//                        PBYTE_WARN,
//                        PBYTE_INFO,
//                        PBYTE_CUT,
//                        PBYTE_FULL};

enum PBYTE_LOG_PRIOS
{
    PBYTE_CRIT = eLOG_CRIT,
    PBYTE_ERR  = eLOG_ERR,
    PBYTE_WARN = eLOG_WARN,
    PBYTE_INFO = eLOG_INFO,
    PBYTE_CUT  = eLOG_CUT,
    PBYTE_FULL = eLOG_FULL
};

typedef struct {
  char prefix[16];
  int logger_prio;
  void (*logger)(int prio, char *format, ...);
} pb_logger_t;

inline void pb_log_set_logger (pb_logger_t *p_logger,  void (*logger)(int prio, char *format, ...));
inline void pb_log_set_prio (pb_logger_t *p_logger, int prio);
inline int pb_log_check_enable (pb_logger_t *p_logger, int level);

void pb_trace (pb_logger_t *p_logger, int level, char *format, ...);

#ifdef TIME_DEBUG
int pb_clockgettime(struct timeval* tv);
void pb_print_msg_time(pb_logger_t *p_logger, char *descript, pb_msg_t *pb_msg);
#endif

#endif
