#ifndef __RTPSW_DRIVER_H__
#define __RTPSW_DRIVER_H__

#define RTPSW_MAJOR_NUM 2706
#define RTPSW_DRIVER_NAME "rtpsw"

#define MAX_PROC_THREAD 4

#define rtpsw_info(frm, ...) printk(KERN_INFO "RTP INFO: "frm"\n", ##__VA_ARGS__)
#define rtpsw_warn(frm, ...) printk(KERN_WARNING "RTP WARN: "frm"\n", ##__VA_ARGS__)
#define rtpsw_error(frm, ...) printk(KERN_ERR "RTP ERR : "frm"\n", ##__VA_ARGS__)

#endif /* __RTPSW_DRIVER_H__ */
