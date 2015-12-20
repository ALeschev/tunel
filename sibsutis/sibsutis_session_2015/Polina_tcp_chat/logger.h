#ifndef __LOGGER_H__
#define __LOGGER_H__

enum e_log_level
{
    eLOG_NONE = -1,

    eLOG_ERR,
    eLOG_WARN,
    eLOG_INFO,

    eLOG_MAX
};

void log_print(int level, char *format, ...);

#endif /* __LOGGER_H__ */