#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "transport.h"
#include "transport_core.h"

int transport_core_start(tcore_t **core, transport_settings_t *settings)
{
    tcore_t *core_ptr;

    *core = (tcore_t *)malloc(sizeof (tcore_t));
    if (!*core)
    {
        return -1;
    }

    core_ptr = *core;

    core_ptr->msg_handler = settings->msg_handler;
    core_ptr->error_handler = settings->error_handler;

    sleep(1);

    core_ptr->msg_handler();
    sleep(1);
    core_ptr->error_handler();

    return 0;
}

int transport_start(transport_t *transport, transport_settings_t *settings)
{
    int rc = -1;

    if (!transport || !settings)
        return -99;

    switch (settings->mode)
    {
        case eTRANSPORT_SERVER:
        case eTRANSPORT_CLIENT:
            transport->mode = eTRANSPORT_SERVER;
            rc = transport_core_start((tcore_t **)&transport->core, settings);
        break;

        default:
            return -98;
    }

    return rc;
}

int transport_stop(transport_t *transport)
{

    free(transport->core);

    return 0;
}

int transport_send(transport_t *transport, const char *data, int size)
{

    return 0;
}
