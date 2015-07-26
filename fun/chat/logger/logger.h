/*
 * LAV by fun
 */

#ifndef __LOG_H__
#define __LOG_H__

#define log_info(frm, ...) log_print(eLOG_INFO, frm, ##__VA_ARGS__)
#define log_debug(frm, ...) log_print(eLOG_CUT, frm, ##__VA_ARGS__)
#define log_warn(frm, ...) log_print(eLOG_WARN, frm, ##__VA_ARGS__)
#define log_error(frm, ...) log_print(eLOG_ERR, frm, ##__VA_ARGS__)

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

//extern e_log_level_str;

void log_print(int level, char *format, ...);
void log_register_logger(con_out_func logger);
void log_deregister_logger(void);

#endif /* __LOG_H__ */