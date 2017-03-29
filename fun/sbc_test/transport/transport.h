#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

typedef enum {
    eTRANSPORT_SERVER = 1,
    eTRANSPORT_CLIENT,
} transport_mode_t;

typedef struct {
    transport_mode_t mode;

    int (*msg_handler)(void);
    int (*error_handler)(void);
} transport_settings_t;

typedef struct {
    transport_mode_t mode;
    void *core;
} transport_t;

int transport_start(transport_t *transport, transport_settings_t *settings);
int transport_stop(transport_t *transport);
int transport_send(transport_t *transport, const char *data, int size);

#endif