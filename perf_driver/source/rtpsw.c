#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <linux/kthread.h>

#include "rtpsw.h"
#include "hook.c"
#include "rtpsw_procfs.c"

static struct nf_hook_ops nf_incoming;

static int rtpsw_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int rtpsw_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t rtpsw_read(struct file *file, char __user *buf,
                          size_t len, loff_t *off)
{
	ssize_t ret_size = 0;

	return ret_size;
}

static ssize_t rtpsw_write(struct file *file, const char __user *buf,
                           size_t len, loff_t *off)
{
	ssize_t ret_size = 0;

	return ret_size;
}

static struct file_operations rtpsw_fops = 
{
	.owner   = THIS_MODULE,
	.open    = rtpsw_open,
	.release = rtpsw_release,
	.read    = rtpsw_read,
	.write   = rtpsw_write
};

static struct nf_hook_ops nf_incoming =
{
	.hook     = rtpsw_main_hook,
	.pf       = PF_INET,
	.hooknum  = NF_INET_PRE_ROUTING,
	.priority = NF_IP_PRI_FIRST
};

static int __init rtpsw_init(void)
{
	int res = -1;

	st_init();

	res = register_chrdev(RTPSW_MAJOR_NUM, RTPSW_DRIVER_NAME, &rtpsw_fops);
	if (res < 0)
	{
		rtpsw_error("failed to register device");
		return res;
	}

	res = rtpsw_procfs_init();
	if (res < 0)
	{
		rtpsw_error("failed to init procfs");
		return res;
	}

	rtpsw_warn("hook in init func!");
	nf_register_hook(&nf_incoming);

	rtpsw_info("init success ");

	return 0;
}

static void __exit rtpsw_deinit(void)
{
	nf_unregister_hook(&nf_incoming);

	rtpsw_procfs_deinit();

	st_deinit();

	unregister_chrdev(RTPSW_MAJOR_NUM, RTPSW_DRIVER_NAME);

	rtpsw_info("deinit success");
}

module_init(rtpsw_init);
module_exit(rtpsw_deinit);