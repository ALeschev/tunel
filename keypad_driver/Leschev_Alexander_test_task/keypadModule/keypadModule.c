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

struct timer_list keypadTimer;
static int deviceOpenCount = 0;
static int newSymbol = IS_EMPTY;
static int prevSymbol = IS_EMPTY;
static int keyCode[4][3] = {
			{49, 50, 51},
			{52, 53, 54},
			{55, 56, 57},
			{48, 127, 13}
};

static int keypadOpen(struct inode *, struct file *);
static int keypadRelease(struct inode *, struct file *);
static ssize_t keypadRead(struct file *, char *, size_t, loff_t *);
static unsigned int pollFlag(struct file *, struct poll_table_struct *);
static DECLARE_WAIT_QUEUE_HEAD(qwait);

static struct file_operations fops = {
	.open = keypadOpen,
	.release = keypadRelease,
	.read = keypadRead,
	.poll = pollFlag
};

void keypadReadChar(unsigned long data)
{
	int i, j;
	#ifndef EMULATION
	char buff = '\0';
	int reversPortLevel = 0x20; /* because it makes a easy shift */
	int mask = 0xE0;
	int levelPort = reversPortLevel ^ mask;
	#endif

	if(newSymbol == IS_EMPTY){ /* if the previous character is not read, ignore the press */
		#ifndef EMULATION
		for(i=0; i<3; i++){
			pca9555_write(0x02, levelPort); /* set 0_5 - 0_7 HIGH level */
			pca9555_read(0x01, &buff);
			buff &= 0x0F;
			for(j=0; j<4; j++) /* get 1_0 - 1_3 line level; if pressed a few buttons, a set-off is the last */
				if(buff & (1 << j))
					newSymbol = keyCode[j][i];
		reversPortLevel <<= 1;
		levelPort = reversPortLevel ^ mask;
		}
		#else
		get_random_bytes(&i, sizeof(int));
		get_random_bytes(&j, sizeof(int));
		i %= 10; j %= 10;
		i %= 3; j %= 4;
		if((i >= 0 && i < 3) && (j >= 0 && j < 4))
			newSymbol = keyCode[j][i];
/*
		printk(KERN_ALERT"newSymbol: %d\t KEYPAD_STATUS: %s\n",newSymbol, (newSymbol)? "BUTTON_PUSHED":"BUTTON_NOT_PUSHED");
*/
		#endif
	}
	if(newSymbol == prevSymbol) 
		newSymbol = IS_EMPTY;
	if(newSymbol != IS_EMPTY)
		wake_up_interruptible(&qwait);
	else
		prevSymbol = IS_EMPTY;

	mod_timer(&keypadTimer, jiffies + msecs_to_jiffies(100));
}
/*----------------------------------------------------------*/
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

	#ifndef EMULATION
	pca9555_write(0x07, 0x0F); /* define 1_0, 1_1, 1_2, 1_3 as input */
	pca9555_write(0x06, 0x1F); /* define 0_5, 0_6, 0_7 as output */
	#endif

	setup_timer(&keypadTimer, keypadReadChar, 0);
	mod_timer(&keypadTimer, jiffies + msecs_to_jiffies(100));

return 0;
}
void cleanup_module(void)
{
	misc_deregister(&pool_dev);
	del_timer(&keypadTimer);
}

/*---------------------------------------------------------*/
static int keypadOpen(struct inode *inode, struct file *file)
{
	if (deviceOpenCount)
		return -EBUSY;
	deviceOpenCount++;
	try_module_get(THIS_MODULE);

return 0;
}

static int keypadRelease(struct inode *inode, struct file *file)
{
	deviceOpenCount--;
	module_put(THIS_MODULE);

return 0;
}

static ssize_t keypadRead(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	if((newSymbol == IS_EMPTY) && (filp->f_flags & O_NONBLOCK))
		return -EAGAIN;
	else
		interruptible_sleep_on(&qwait);

	if(newSymbol != IS_EMPTY){ 
		if(put_user(newSymbol, buffer))
			return -EFAULT;
		prevSymbol = newSymbol;
		newSymbol = IS_EMPTY;
		return 1;
	}
	else
		return 0;
}

static unsigned int pollFlag(struct file *file, struct poll_table_struct *poll)
{
	int flag = 0;
	poll_wait(file, &qwait, poll);
	if(newSymbol != IS_EMPTY)
		flag |= POLLIN | POLLRDNORM;

return flag;
}