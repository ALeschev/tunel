#ifndef _ICMP_CORE_H_
#define _ICMP_CORE_H_

#define ADDR_BY_PULL 200
#define MAX_ADDR_LEN 64

typedef struct {
    char addr[MAX_ADDR_LEN];
} icmp_addr_t;

typedef struct {
    union {
        struct {
            uint8_t type;
            uint8_t code;
            uint8_t timeout:1,
                    send_error:1,
                    mem_error:1,
                    res:5;
        } err;

        uint32_t total;
    } uerror;

#define err_type uerror.err.type
#define err_code uerror.err.code
#define err_timeout uerror.err.timeout
#define err_send uerror.err.send_error
#define err_mem uerror.err.mem_error
#define err_total uerror.total

    struct timeval delay;

} icmp_result_t;

typedef struct {
    icmp_addr_t *source;
    icmp_addr_t *dest;
    int dest_count;
} icmp_addr_in_t;

typedef void (*icmp_notify_cb_t)(void);
typedef void (*icmp_result_cb_t)(char *dest_address, icmp_result_t *error);

typedef struct {
    // icmp_notify_cb_t started;
    icmp_result_cb_t req_result;
    icmp_notify_cb_t send_finished;
} icmp_callback_t;

int icmp_send (icmp_addr_in_t *addr_in,
               /* added wait reply timeout */
               icmp_callback_t *callbacks);

int icmp_send_periodical (icmp_addr_in_t *addr_in,
                          /* added wait reply timeout */
                          int send_period_ms, /* milliseconds */
                          int send_count,
                          icmp_callback_t *callbacks);

#endif /* _ICMP_CORE_H_ */
