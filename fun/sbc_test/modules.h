#ifndef __SBC_MODULES_H__
#define __SBC_MODULES_H__

typedef enum
{
    eSBC_MODTYPE_NONE = 0,

    eSBC_MODTYPE_DISPATCHER,
    eSBC_MODTYPE_MANAGER,
    eSBC_MODTYPE_WORKER,

    eSBC_MODTYPE_MAX
} sbc_module_type_t;

#endif