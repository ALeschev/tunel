#ifndef __LOG_H__
#define __LOG_H__

#define log_info(frm, ...) log_print(eLOG_INFO, frm, ##__VA_ARGS__)
#define log_debug(frm, ...) log_print(eLOG_DEBUG, frm, ##__VA_ARGS__)
#define log_warn(frm, ...) log_print(eLOG_WARN, frm, ##__VA_ARGS__)
#define log_error(frm, ...) log_print(eLOG_ERR, frm, ##__VA_ARGS__)

enum e_log_level
{
	eLOG_INFO,
	eLOG_DEBUG,
	eLOG_WARN,
	eLOG_ERR,
};

void log_print(int level, char *format, ...);

#endif /* __LOG_H__ */