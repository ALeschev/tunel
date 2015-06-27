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
#include "rtpsw_port_sets.c"
#include "rtpsw_procfs.c"

static struct nf_hook_ops nf_incoming;
static struct task_struct *at_thread[4];

static int rtpsw_open(struct inode *inode, struct file *file)
{
	// nf_register_hook(&nf_incoming);

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

static int rtpsw_proc(void *arg)
{
	while(!kthread_should_stop())
	{
		msleep(100);
		schedule();
	}

	rtpsw_info("exit");

	return 0;
}

static void rtpsw_proc_thread_init(void)
{
	int i;

	for (i = 0; i < MAX_PROC_THREAD; i++)
	{
		at_thread[i] = kthread_run(rtpsw_proc, NULL, "rtp_%d", i);
		if (!at_thread[i])
		{
			rtpsw_error("rtp thread %d: init failed", i);
			return;
		}
		rtpsw_info("rtp thread %d: started", i);
	}
}

static void rtpsw_proc_thread_deinit(void)
{
	int i;

	for (i = 0; i < MAX_PROC_THREAD; i++)
	{
		rtpsw_info("thread %d stopped: %p state %d",
		           i, at_thread[i], at_thread[i]->state);
		/* valid and runnable */
		// if (at_thread[i] && (at_thread[i]->state == 0))
		// {
		// 	kthread_stop(at_thread[i]);
		// 	rtpsw_info("thread %d stopped", i);
		// }
	}
}

static int __init rtpsw_init(void)
{
	int res = -1;

	st_init();

	rtpsw_proc_thread_init();

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
	// rtpsw_proc_thread_deinit();

	nf_unregister_hook(&nf_incoming);

	rtpsw_procfs_deinit();

	unregister_chrdev(RTPSW_MAJOR_NUM, RTPSW_DRIVER_NAME);

	rtpsw_info("deinit success");
}

module_init(rtpsw_init);
module_exit(rtpsw_deinit);