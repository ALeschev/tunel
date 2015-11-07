#include <linux/proc_fs.h>

struct proc_dir_entry *rtpsw_proc_entry;

static ssize_t rtpsw_proc_print_help(char *buff, ssize_t len)
{
	ssize_t result = 0;

	result = snprintf(buff, len,
	                   "rps - reset packets statistics\n");

	return result;
}

static int rtpsw_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int rtpsw_proc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t rtpsw_proc_read(struct file *file, char __user *buf,
                               size_t len, loff_t *ppos)
{
	static ssize_t result = 0;

	if (result == 0)
	{
		if (result < len)
			result += st_pkt_print_to_buff(&buf[result], len - result);

		if (result < len)
			result += rtpsw_proc_print_help(&buf[result], len - result);
	}
	else
	{
		result = 0;
	}

	return result;
}

static ssize_t rtpsw_proc_write(struct file *file, const char __user *buf,
                                size_t len, loff_t *ppos)
{
	rtpsw_info("write: buff '%s'", buf);
	return len;
}

static struct file_operations rtpsw_proc_fops = 
{
	.owner   = THIS_MODULE,
	.open    = rtpsw_proc_open,
	.release = rtpsw_proc_release,
	.read    = rtpsw_proc_read,
	.write   = rtpsw_proc_write
};

static inline int rtpsw_procfs_init(void)
{
	rtpsw_proc_entry = create_proc_entry(RTPSW_DRIVER_NAME, 0, NULL);
	if (!rtpsw_proc_entry)
		return -1;

	rtpsw_proc_entry->proc_fops = &rtpsw_proc_fops;
	rtpsw_proc_entry->proc_iops = NULL;
	rtpsw_proc_entry->mode = S_IFREG | S_IRUGO | S_IWUSR;
	rtpsw_proc_entry->uid = 0;
	rtpsw_proc_entry->gid = 0;
	rtpsw_proc_entry->size = 80;

	rtpsw_info("procfs was inited");

	return 0;
}

static inline void rtpsw_procfs_deinit(void)
{
	remove_proc_entry(RTPSW_DRIVER_NAME, NULL);

	rtpsw_info("procfs was deinited");
}
