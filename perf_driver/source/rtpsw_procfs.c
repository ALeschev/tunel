#include <linux/proc_fs.h>

struct proc_dir_entry *rtpsw_proc_entry;

int rtpsw_proc_open(struct inode *inode, struct file *file)
{
	return 0;
}

int rtpsw_proc_release(struct inode *inode, struct file *file)
{
	return 0;
}

ssize_t rtpsw_proc_read(struct file *file, char __user *buf,
                        size_t len, loff_t *ppos)
{
	return 0;
}

ssize_t rtpsw_proc_write(struct file *file, const char __user *buf,
                         size_t len, loff_t *ppos)
{
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
