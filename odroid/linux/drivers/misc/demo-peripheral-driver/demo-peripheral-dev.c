/**
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/major.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>

// #define __DEMO_PERIPHERAL_DEV_ASYNC

#define DEVICE_NAME			("demo_peripheral_device")

#define IOCTL_WRONLY		(0x1000)
#define IOCTL_RDWR			(0x2000)

#define TARGET_DEVICES		(1)

/* CON10.26: XE.INT16(GPX2.0): EXT_INT16 */
#define SENSOR_IRQ			IRQ_EINT(16)
#define SENSOR_GPIO_PORT	(irq_to_gpio(SENSOR_IRQ))

/* CON10.18: XE.INT11(GPX1.3): EXT_INT11 */
#define SWITCH_IRQ			IRQ_EINT(11)
#define SWITCH_GPIO_PORT	(irq_to_gpio(SWITCH_IRQ))

#define POLL_STATUS_SENSOR	(1)
#define POLL_STATUS_SWITCH	(2)

#define RNDUP(n, b)	((((n) + ((n) + (b))) / (b)) * (b))

/**
 *
 */
#pragma pack(1)
typedef struct command_t {
	uint16_t	type;
	uint16_t	pattern;
	uint32_t	value[7];
} command_t;
#pragma pack()

/**
 *
 */
typedef struct demo_peripheral_dev_t {
	struct class			*class_inst;
	dev_t					dev_inst;
	struct cdev				cdev_inst;
	wait_queue_head_t		wait_queue;
#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
	struct workqueue_struct	*work_queue;
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */
	struct spi_device		*spi;
	struct device			*dev[TARGET_DEVICES];
	uint32_t				poll_status;
}	demo_peripheral_dev_t;

/**
 *
 */
typedef struct demo_peripheral_dev_request_t {
	struct work_struct		wk_st;
	demo_peripheral_dev_t	*dev;
	uint32_t				len;
	uint8_t					*data;
	uint8_t					*buff;
} demo_peripheral_dev_request_t;

static irqreturn_t demo_peripheral_dev_irq_handler(int irq, void *param);
static int demo_peripheral_dev_xfer(demo_peripheral_dev_t *dev,
		void *data, void *buff, uint32_t len);
static int demo_peripheral_dev_match_devt(struct device *dev, void *data);
#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
static void demo_peripheral_dev_post(demo_peripheral_dev_request_t *req);
static void demo_peripheral_dev_work(struct work_struct *wk_st);
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */
static int demo_peripheral_dev_open(struct inode *inode, struct file *filp);
static int demo_peripheral_dev_release(struct inode *inode, struct file *filp);
static long demo_peripheral_dev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg);
static ssize_t demo_peripheral_dev_write(struct file *filp,
		const char __user *buff, size_t count, loff_t *ppos);
static ssize_t demo_peripheral_dev_read(struct file *filp, char __user *buff,
		size_t count, loff_t *ppos);
static unsigned int demo_peripheral_dev_poll(struct file *filp,
		poll_table *wait);
#ifdef CONFIG_PM_SLEEP
static int demo_peripheral_dev_suspend(struct device *dev);
static int demo_peripheral_dev_resume(struct device *dev);
#endif
static int demo_peripheral_dev_probe(struct spi_device *spi);
static int demo_peripheral_dev_remove(struct spi_device *spi);

/**
 *
 */
static const struct file_operations demo_peripheral_dev_fops = {
	.owner			= THIS_MODULE,
	.open			= demo_peripheral_dev_open,
	.release		= demo_peripheral_dev_release,
	.unlocked_ioctl	= demo_peripheral_dev_ioctl,
	.read			= demo_peripheral_dev_read,
	.write			= demo_peripheral_dev_write,
	.poll			= demo_peripheral_dev_poll,
};

/**
 *
 */
static struct class *demo_peripheral_dev_class_inst = NULL;

/**
 *
 */
static irqreturn_t demo_peripheral_dev_sensor_irq_handler(int irq, void *param)
{
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)param;
	printk(KERN_DEBUG "l(%4d): %s(): irq=%d\n", __LINE__, __PRETTY_FUNCTION__, irq);
	if (irq != SENSOR_IRQ) {
		return IRQ_NONE;
	}
	dev->poll_status = POLL_STATUS_SENSOR;
	wake_up_interruptible(&(dev->wait_queue));
	return IRQ_HANDLED;
}

/**
 *
 */
static irqreturn_t demo_peripheral_dev_switch_irq_handler(int irq, void *param)
{
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)param;
	printk(KERN_DEBUG "l(%4d): %s(): irq=%d\n", __LINE__, __PRETTY_FUNCTION__, irq);
	if (irq != SWITCH_IRQ) {
		return IRQ_NONE;
	}
	dev->poll_status = POLL_STATUS_SWITCH;
	wake_up_interruptible(&(dev->wait_queue));
	return IRQ_HANDLED;
}

/**
 *
 */
static int demo_peripheral_dev_xfer(demo_peripheral_dev_t *dev,
		void *data, void *buff, uint32_t len)
{
	struct spi_message msg;
	struct spi_transfer xfer = {
		.len			= len,
		.cs_change		= 0,
		.speed_hz		= 1000000,
		.bits_per_word	= 8,
		.tx_buf			= data,
		.rx_buf			= buff,
	};
	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	return spi_sync(dev->spi, &msg);
}

/**
 *
 */
static int demo_peripheral_dev_match_devt(struct device *dev, void *data)
{
	const dev_t *devt = data;
	return dev->devt == *devt;
}

/**
 *
 */
static demo_peripheral_dev_t *demo_peripheral_dev_from_devt(struct inode *inode)
{
	struct device *dev;
	dev = class_find_device(demo_peripheral_dev_class_inst, NULL,
			&(inode->i_rdev), demo_peripheral_dev_match_devt);
	if (dev) {
		return dev_get_drvdata(dev);
	}
	return NULL;

}

#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
/**
 *
 */
static void demo_peripheral_dev_post(demo_peripheral_dev_request_t *req)
{
	demo_peripheral_dev_request_t *rq;
	rq = (demo_peripheral_dev_request_t *)kmalloc(sizeof(demo_peripheral_dev_request_t), GFP_KERNEL);
	memcpy(rq, req, sizeof(demo_peripheral_dev_request_t));
	INIT_WORK(&rq->wk_st, demo_peripheral_dev_work);
	queue_work(rq->dev->work_queue, &rq->wk_st);
}

/**
 *
 */
static void demo_peripheral_dev_work(struct work_struct *wk_st)
{
	demo_peripheral_dev_request_t *req;
	req = container_of(wk_st, demo_peripheral_dev_request_t, wk_st);
	demo_peripheral_dev_xfer(req->dev, req->data, req->buff, req->len);
	if (req->buff) {
	}
	if (req->data) {
		kfree(req->data);
	}
	kfree(req);
}
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */

/**
 *
 */
static int demo_peripheral_dev_open(struct inode *inode, struct file *filp)
{
	demo_peripheral_dev_t *dev;
	dev = demo_peripheral_dev_from_devt(inode);
	filp->private_data = dev;
	return(0);
}

/**
 *
 */
static int demo_peripheral_dev_release(struct inode *inode, struct file *filp)
{
//	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)(filp->private_data);
	return(0);
}

/**
 *
 */
static long demo_peripheral_dev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	long result = 0;
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)(filp->private_data);
	command_t *p = NULL;

	printk(KERN_DEBUG "%s::%s: l(%d): cmd=%u\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, cmd);

	p = (command_t *)kzalloc(RNDUP(sizeof(command_t) * 2, 16), GFP_KERNEL);
	result = (long) copy_from_user(p, (void *)arg, sizeof(command_t));
	if (result != 0) {
		printk(KERN_ERR "%s::%s: l(%d): result=%ld\n",
				__FILE__, __PRETTY_FUNCTION__, __LINE__, result);
	}
	result = demo_peripheral_dev_xfer(dev, p, &(p[1]), sizeof(command_t));
	if (cmd == IOCTL_RDWR) {
		result = (long) copy_to_user(&(((command_t *)arg)[1]), &(p[1]),
				sizeof(command_t));
		if (result != 0) {
			printk(KERN_ERR "%s::%s: l(%d): result=%ld\n",
					__FILE__, __PRETTY_FUNCTION__, __LINE__, result);
		}
	}
	kfree(p);

	return(result);
}

/**
 *
 */
static ssize_t demo_peripheral_dev_write(struct file *filp,
		const char __user *buff, size_t count, loff_t *ppos)
{
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)(filp->private_data);
	uint8_t *p;
	unsigned long rest;
	p = kzalloc(RNDUP(count, 16), GFP_KERNEL);
	rest = copy_from_user(p, buff, count);
	if (rest != 0) {
		printk(KERN_ERR "%s::%s: l(%d): rest=%ld\n",
				__FILE__, __PRETTY_FUNCTION__, __LINE__, rest);
	}
	(void) demo_peripheral_dev_xfer(dev, p, NULL, count - rest);
	kfree(p);
	return (count - rest);
}

/**
 *
 */
static ssize_t demo_peripheral_dev_read(struct file *filp, char __user *buff,
			   size_t count, loff_t *ppos)
{
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)(filp->private_data);
	uint8_t *p;
	unsigned long rest;
	p = kzalloc(RNDUP(count * 2, 16), GFP_KERNEL);
	(void) demo_peripheral_dev_xfer(dev, &(p[count]), p, count);
	rest = copy_to_user(buff, p, count);
	if (rest != 0) {
		printk(KERN_ERR "%s::%s: l(%d): rest=%ld\n",
				__FILE__, __PRETTY_FUNCTION__, __LINE__, rest);
	}
	kfree(p);
	*ppos = (count - rest);
	return (count - rest);
}

/**
 *
 */
static unsigned int demo_peripheral_dev_poll(struct file *filp, poll_table *wait)
{
	uint32_t mask = 0;
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)(filp->private_data);
	poll_wait(filp, &(dev->wait_queue), wait);
	if (dev->poll_status) {
		mask = POLLIN | POLLRDNORM;
	}
	dev->poll_status = 0;
	return mask;
}

#ifdef CONFIG_PM_SLEEP
/**
 *
 */
static int demo_peripheral_dev_suspend(struct device *dev)
{
	demo_peripheral_dev_t *pdev = dev_get_drvdata(dev);
	printk(KERN_DEBUG "l(%4d): %s(): dev=%p\n", __LINE__, __PRETTY_FUNCTION__, pdev);
	return 0;
}

/**
 *
 */
static int demo_peripheral_dev_resume(struct device *dev)
{
	demo_peripheral_dev_t *pdev = dev_get_drvdata(dev);
	printk(KERN_DEBUG "l(%4d): %s(): dev=%p\n", __LINE__, __PRETTY_FUNCTION__, pdev);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(demo_peripheral_dev_pm_ops, demo_peripheral_dev_suspend, demo_peripheral_dev_resume);

/**
 *
 */
static int demo_peripheral_dev_probe(struct spi_device *spi)
{
	demo_peripheral_dev_t *dev = NULL;
	int result = 0;
	int minor;

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

	dev = (demo_peripheral_dev_t *)kzalloc(RNDUP(sizeof(demo_peripheral_dev_t), 16), GFP_KERNEL);
	dev->spi = spi;

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

	init_waitqueue_head(&(dev->wait_queue));

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
	dev->work_queue = create_singlethread_workqueue("demo_peripheral_dev_work_queue");
	if (!dev->work_queue) {
		return -EBADF;
	}
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

	result = request_irq(SENSOR_IRQ,
			demo_peripheral_dev_sensor_irq_handler, IRQF_SHARED | IRQF_TRIGGER_RISING,
			"demo_peripheral_dev_sensor_irq_handler", dev);
	if (result != 0)	{
		printk(KERN_ERR "l(%4d): %s() result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
		return result;
	}
	enable_irq(SENSOR_IRQ);

	result = request_irq(SWITCH_IRQ,
			demo_peripheral_dev_switch_irq_handler, IRQF_SHARED | IRQF_TRIGGER_RISING,
			"demo_peripheral_dev_switch_irq_handler", dev);
	if (result != 0)	{
		printk(KERN_ERR "l(%4d): %s() result=%d\n", __LINE__, __PRETTY_FUNCTION__, result);
		return result;
	}
	enable_irq(SWITCH_IRQ);

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

	if (!demo_peripheral_dev_class_inst) {
		demo_peripheral_dev_class_inst = class_create(THIS_MODULE, DEVICE_NAME);
	}
	dev->class_inst = demo_peripheral_dev_class_inst;

	result = alloc_chrdev_region(&(dev->dev_inst),
			0u, TARGET_DEVICES, DEVICE_NAME);
	if (unlikely(result < 0))	{
		printk(KERN_ERR "l(%4d): %s %d\n", __LINE__, __PRETTY_FUNCTION__, result);
	}

	cdev_init(&(dev->cdev_inst), &demo_peripheral_dev_fops);
	dev->cdev_inst.owner = demo_peripheral_dev_fops.owner;

	result = cdev_add(&(dev->cdev_inst), MKDEV(MAJOR(
			dev->dev_inst), 0), TARGET_DEVICES);
	if (unlikely(result < 0))	{
		printk(KERN_ERR "l(%4d): %s %d\n", __LINE__, __PRETTY_FUNCTION__, result);
	}

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

	for (minor = 0; minor < TARGET_DEVICES; minor++)	{
		char	filename_fmt[64];
		strcpy(filename_fmt, DEVICE_NAME);
		strcat(filename_fmt, "_%02d");
		dev->dev[minor] = device_create(dev->class_inst,
				(struct device *)NULL, MKDEV(MAJOR(dev->dev_inst),
						minor), dev, filename_fmt, minor);
		if (unlikely(IS_ERR(dev->dev[minor])))	{
			printk(KERN_ERR "%s::%s(): l(%d): %lx\n", __FILE__,
					__PRETTY_FUNCTION__, __LINE__, (unsigned long)
					dev->dev[minor]);
		}
	}
	spi_set_drvdata(spi, dev);

	printk(KERN_DEBUG "l(%4d): %s()\n", __LINE__, __PRETTY_FUNCTION__);

	return 0;
}

/**
 *
 */
static int demo_peripheral_dev_remove(struct spi_device *spi)
{
	int minor;
	demo_peripheral_dev_t *dev = spi_get_drvdata(spi);

	disable_irq(SENSOR_IRQ);
	free_irq(SENSOR_IRQ, dev);

	disable_irq(SWITCH_IRQ);
	free_irq(SWITCH_IRQ, dev);

	for (minor = 0; minor < TARGET_DEVICES; minor++)	{
		device_destroy(dev->class_inst, MKDEV(MAJOR(dev->dev_inst), minor));
	}
	cdev_del(&(dev->cdev_inst));
	unregister_chrdev_region(dev->dev_inst, TARGET_DEVICES);
	class_destroy(dev->class_inst);

#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
	destroy_workqueue(dev->work_queue);
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */
	kfree(dev);

	return 0;
}

/**
 *
 */
static struct spi_driver demo_peripheral_dev_driver = {
	.driver = {
		.name	= "ioboard-spi",
		.bus	= &spi_bus_type,
		.pm		= &demo_peripheral_dev_pm_ops,
		.owner	= THIS_MODULE,
	},
	.probe		= demo_peripheral_dev_probe,
	.remove		= demo_peripheral_dev_remove,
};

module_spi_driver(demo_peripheral_dev_driver);

MODULE_AUTHOR("dts-insight");
MODULE_DESCRIPTION("peripheral device driver for demo.");
MODULE_LICENSE("GPL v2");
