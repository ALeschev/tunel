#include <stdio.h>
#include <unistd.h>
#include "transport/transport.h"

int msg_handler(void)
{
    printf("%s\n", __func__);
    return 0;
}
int error_handler(void)
{
    printf("%s\n", __func__);
    return 0;
}

int main(int argc, char *argv[])
{
    transport_t transport;
    transport_settings_t settings;

    settings.mode = eTRANSPORT_SERVER;
    settings.msg_handler = msg_handler;
    settings.error_handler = error_handler;

    transport_start(&transport, &settings);

    sleep(1);

    transport_stop(&transport);

    return 0;
}