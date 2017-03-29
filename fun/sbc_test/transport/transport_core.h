#ifndef __TRANSPORT_CORE_H__
#define __TRANSPORT_CORE_H__

#include "transport.h"

typedef struct
{
    int (*msg_handler)(void);
    int (*error_handler)(void);
} tcore_t;

#endif