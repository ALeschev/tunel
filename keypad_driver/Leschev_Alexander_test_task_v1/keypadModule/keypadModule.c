#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <linux/random.h>

#define EMULATION

#define DEVICE_NAME "10_button_keypad"
#define IS_EMPTY 0

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Leschev");

static struct timer_list keypad_timer;
static int device_open_count = 0;
static int new_symbol = IS_EMPTY;
static int old_symbol = IS_EMPTY;
static int key_code[4][3] = {
			{49, 50, 51},
			{52, 53, 54},
			{55, 56, 57},
			{48, 127, 13}
};

static int keypad_open(struct inode *, struct file *);
static int keypad_release(struct inode *, struct file *);
static ssize_t keypad_read(struct file *, char *, size_t, loff_t *);
static unsigned int poll_flag(struct file *, struct poll_table_struct *);
static DECLARE_WAIT_QUEUE_HEAD(qwait);

static struct file_operations fops = {
	.open = keypad_open,
	.release = keypad_release,
	.read = keypad_read,
	.poll = poll_flag
};

#ifdef EMULATION
void emulation_keypad_push_button(unsigned long data)
{
	int i, j;

	if(new_symbol == IS_EMPTY) {
		get_random_bytes(&i, sizeof(int));
		get_random_bytes(&j, sizeof(int));

		/* set range */
		i %= 10; j %= 10;
		i /= 4; j /= 3;
		if(i < 0)
			i = -i;
		if(j < 0)
			j = -j;

		new_symbol = key_code[j][i];
	}

	if(new_symbol == old_symbol)
		new_symbol = IS_EMPTY;

	if(new_symbol != IS_EMPTY)
		wake_up_interruptible(&qwait);
	else
		old_symbol = IS_EMPTY;

	mod_timer(&keypad_timer, jiffies + msecs_to_jiffies(100));
}
#else
void keypad_read_char(unsigned long data)
{
	/* because it makes a easy shift */
	int revers_port_level = 0x20;

	char buff = '\0';
	int mask = 0xE0;
	int level_port = revers_port_level ^ mask;
	int i, j;

	if(new_symbol == IS_EMPTY) {
		for(i=0; i<3; i++) {
			pca9555_write(0x02, level_port);
			pca9555_read(0x01, &buff);
			buff &= 0x0F;

			for(j=0; j<4; j++)
				if(buff & (1 << j))
					new_symbol = key_code[j][i];

			revers_port_level <<= 1;
			level_port = revers_port_level ^ mask;
		}
	}

	if(new_symbol == old_symbol)
		new_symbol = IS_EMPTY;

	if(new_symbol != IS_EMPTY)
		wake_up_interruptible(&qwait);
	else
		old_symbol = IS_EMPTY;

	mod_timer(&keypad_timer, jiffies + msecs_to_jiffies(100));
}
#endif

static struct miscdevice pool_dev = {
	MISC_DYNAMIC_MINOR,
	DEVICE_NAME,
	&fops
};

int init_module(void)
{
	int ret = misc_register(&pool_dev);

	if (ret < 0){
		printk(KERN_ALERT"init_module: Registering the character device failed with %d\n", ret);
		return ret;
	}

	#ifdef EMULATION
	setup_timer(&keypad_timer, emulation_keypad_push_button, 0);
	#else
	/* define 1_0, 1_1, 1_2, 1_3 as input */
	pca9555_write(0x07, 0x0F);
	/* define 0_5, 0_6, 0_7 as output */
	pca9555_write(0x06, 0x1F);

	setup_timer(&keypad_timer, keypad_read, 0);
	#endif

	mod_timer(&keypad_timer, jiffies + msecs_to_jiffies(100));

return 0;
}
void cleanup_module(void)
{
	misc_deregister(&pool_dev);
	del_timer(&keypad_timer);
}

static int keypad_open(struct inode *inode, struct file *file)
{
	if (device_open_count)
		return -EBUSY;

	device_open_count++;
	try_module_get(THIS_MODULE);

return 0;
}

static int keypad_release(struct inode *inode, struct file *file)
{
	device_open_count--;
	module_put(THIS_MODULE);

return 0;
}

static ssize_t keypad_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	if((new_symbol == IS_EMPTY) && (filp->f_flags & O_NONBLOCK))
		return -EAGAIN;
	else
		interruptible_sleep_on(&qwait);

	if(new_symbol != IS_EMPTY){
		if(put_user(new_symbol, buffer))
			return -EFAULT;

		old_symbol = new_symbol;
		new_symbol = IS_EMPTY;

		return 1;
	}

return 0;
}

static unsigned int poll_flag(struct file *file, struct poll_table_struct *poll)
{
	int flag = 0;

	poll_wait(file, &qwait, poll);
	if(new_symbol != IS_EMPTY)
		flag |= POLLIN | POLLRDNORM;

return flag;
}
