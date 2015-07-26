/*
 * LAV by fun
 */

#ifndef __LOG_H__
#define __LOG_H__

#define log_logger(prio, frm, ...)                       \
            do {                                         \
                if (log_check_enable(prio))              \
                {                                        \
                    log_print(prio, frm, ##__VA_ARGS__); \
                }                                        \
            } while(0)

#define log_crit(frm, ...) log_logger(eLOG_CRIT, frm, ##__VA_ARGS__)
#define log_error(frm, ...) log_logger(eLOG_ERR, frm, ##__VA_ARGS__)
#define log_warn(frm, ...) log_logger(eLOG_WARN, frm, ##__VA_ARGS__)
#define log_info(frm, ...) log_logger(eLOG_INFO, frm, ##__VA_ARGS__)
#define log_debug(frm, ...) log_logger(eLOG_CUT, frm, ##__VA_ARGS__)

typedef void (*con_out_func)(char *);

enum e_log_level
{
    eLOG_CRIT = 0,
    eLOG_ERR,
    eLOG_WARN,
    eLOG_INFO,
    eLOG_CUT,
    eLOG_FULL,

    eLOG_MAX
};

static const char *e_log_level_str[eLOG_MAX] = 
{
    "CRIT",
    "ERR",
    "WARN",
    "INFO",
    "CUT",
    "FULL"
};

int log_check_enable(int level);
void log_set_prio(int level);

void log_print(int level, char *format, ...);
void log_register_logger(con_out_func logger);
void log_deregister_logger(void);

#endif /* __LOG_H__ */