


#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/syscalls.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/poll.h>
#include <linux/gpio.h>

// #define	LOCAL_DEBUG_LOG

#define	DEVICE_NAME			("demo_device")
#define	CONTROL_DEVICE_MAX	(1u)

typedef struct demo_dev_devices_t {
	struct class		*class_inst;
	dev_t				dev_inst;
	struct cdev			cdev_inst;
	struct device		*dev[CONTROL_DEVICE_MAX];
} demo_dev_devices_t;

extern void sc1602bs_initialize(void);
extern void sc1602bs_finalize(void);
extern void sc1602bs_indicate_string(char *str);
extern void sc1602bs_clear_display(void);

static int __init	demo_dev_init_driver(void);
static void __exit	demo_dev_cleanup_driver(void);
static int	demo_dev_driver_open(struct inode *inode,
		struct file *filp);
static int	demo_dev_driver_release(struct inode *inode,
		struct file *filp);
static ssize_t	demo_dev_driver_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos);
static ssize_t	demo_dev_driver_write(struct file *filp,
		const char __user *buf, size_t count, loff_t *ppos);

static const struct file_operations demo_dev_device_fops = {
	.owner			= THIS_MODULE,
	.open			= demo_dev_driver_open,
	.release		= demo_dev_driver_release,
	.read			= demo_dev_driver_read,
	.write			= demo_dev_driver_write,
};

static demo_dev_devices_t	demo_drv_devices;

/* -------------------------------------------------------------------------- */
static int __init	demo_dev_init_driver(void)
{
	int result, minor;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	memset(&demo_drv_devices, 0, sizeof(demo_drv_devices));

	sc1602bs_initialize();

	demo_drv_devices.class_inst = class_create(THIS_MODULE, DEVICE_NAME);

	result = alloc_chrdev_region(&demo_drv_devices.dev_inst,
			0u, CONTROL_DEVICE_MAX, DEVICE_NAME);
	if (unlikely(result < 0))	{
		printk(KERN_ERR "l(%4d): %s %d\n", __LINE__, __PRETTY_FUNCTION__, result);
	}

	cdev_init(&demo_drv_devices.cdev_inst, &demo_dev_device_fops);
	demo_drv_devices.cdev_inst.owner = demo_dev_device_fops.owner;

	result = cdev_add(&demo_drv_devices.cdev_inst, MKDEV(MAJOR(
			demo_drv_devices.dev_inst), 0), CONTROL_DEVICE_MAX);
	if (unlikely(result < 0))	{
		printk(KERN_ERR "l(%4d): %s %d\n", __LINE__, __PRETTY_FUNCTION__, result);
	}

	for (minor = 0; minor < CONTROL_DEVICE_MAX; minor++)	{
		char	filename_fmt[32];
		strcpy(filename_fmt, DEVICE_NAME);
		strcat(filename_fmt, "_%02d");
		demo_drv_devices.dev[minor] = device_create(demo_drv_devices.class_inst,
				(struct device *)NULL, MKDEV(MAJOR(demo_drv_devices.dev_inst),
						minor), &demo_drv_devices, filename_fmt, minor);
		if (unlikely(IS_ERR(demo_drv_devices.dev[minor])))	{
			printk(KERN_ERR "%s::%s(): l(%d): %lx\n", __FILE__,
					__PRETTY_FUNCTION__, __LINE__, (unsigned long)
					demo_drv_devices.dev[minor]);
		}
	}

	return(result);
}

/* -------------------------------------------------------------------------- */
static void __exit	demo_dev_cleanup_driver(void)
{
	int minor;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	sc1602bs_finalize();

	for (minor = 0; minor < CONTROL_DEVICE_MAX; minor++)	{
		device_destroy(demo_drv_devices.class_inst, MKDEV(MAJOR(
				demo_drv_devices.dev_inst), minor));
	}

	cdev_del(&demo_drv_devices.cdev_inst);
	unregister_chrdev_region(demo_drv_devices.dev_inst,
			CONTROL_DEVICE_MAX);

	class_destroy(demo_drv_devices.class_inst);
}

/* -------------------------------------------------------------------------- */
static int	demo_dev_driver_open(struct inode *inode, struct file *filp)
{
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	filp->private_data = &demo_drv_devices;

	return(0);
}

/* -------------------------------------------------------------------------- */
static int	demo_dev_driver_release(struct inode *inode, struct file *filp)
{
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	return(0);
}

/* -------------------------------------------------------------------------- */
static ssize_t	demo_dev_driver_write(struct file *filp,
		const char __user *buf, size_t count, loff_t *ppos)
{
	char *p;
	int result;

	if (!buf) {
		sc1602bs_clear_display();
		return(count);
	}

	p = kzalloc(count + 16, GFP_KERNEL);
	result = copy_from_user(p, buf, count);
	if (result != 0) {
		printk(KERN_ERR "%s::%s: l(%d): result=%d\n",
				__FILE__, __PRETTY_FUNCTION__, __LINE__, result);
	}
	sc1602bs_indicate_string(p);
	kfree(p);

	return(count);
}

/* -------------------------------------------------------------------------- */
static ssize_t	demo_dev_driver_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos)
{
	uint8_t bytes[32];
	int len;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(0x%lx): l(%d): buf=%p. ppos=%x\n",
			__FILE__, __PRETTY_FUNCTION__, (unsigned long)filp, __LINE__,
			buf, (int)*ppos);
#endif	// LOCAL_DEBUG_LOG

	if (*ppos > 0) {
		return 0;
	}

	memset(bytes, 0, sizeof(bytes));
	strcpy(bytes, "dummy\n");
	len = strlen((char *)bytes);
	len -= copy_to_user(buf, bytes, len);
	*ppos = len;
	return len;
}

/* -------------------------------------------------------------------------- */
MODULE_AUTHOR("hoge");
MODULE_DESCRIPTION("device driver for demo.");
MODULE_LICENSE("GPL v2");

module_init(demo_dev_init_driver);
module_exit(demo_dev_cleanup_driver);



