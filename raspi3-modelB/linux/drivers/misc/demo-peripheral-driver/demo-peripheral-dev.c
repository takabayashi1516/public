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

#define BCM2835_REG_GP_BASE	(0x3f200000)

#define SENSOR_GPIO_PORT	(22)
#define SENSOR_GPFSEL_BIT	(3 * (SENSOR_GPIO_PORT - 20))

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
typedef struct bcm2835_reg_gp_t {
	uint32_t	fsel0;
	uint32_t	fsel1;
	uint32_t	fsel2;
	uint32_t	fsel3;
	uint32_t	fsel4;
	uint32_t	fsel5;
	uint32_t	reserve1;
	uint32_t	set0;
	uint32_t	set1;
	uint32_t	reserve2;
	uint32_t	clr0;
	uint32_t	clr1;
	uint32_t	reserve3;
	uint32_t	lev0;
	uint32_t	lev1;
	uint32_t	reserve4;
	uint32_t	eds0;
	uint32_t	eds1;
	uint32_t	reserve5;
	uint32_t	ren0;
	uint32_t	ren1;
	uint32_t	reserve6;
	uint32_t	fen0;
	uint32_t	fen1;
	uint32_t	reserve7;
	uint32_t	hen0;
	uint32_t	hen1;
	uint32_t	reserve8;
	uint32_t	len0;
	uint32_t	len1;
	uint32_t	reserve9;
	uint32_t	aren0;
	uint32_t	aren1;
	uint32_t	reserve10;
	uint32_t	afen0;
	uint32_t	afen1;
	uint32_t	reserve11;
	uint32_t	pud;
	uint32_t	pudclk0;
	uint32_t	pudclk1;
}	bcm2835_reg_gp_t;

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
	bcm2835_reg_gp_t		*gp_reg;
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
static irqreturn_t demo_peripheral_dev_irq_handler(int irq, void *param)
{
	demo_peripheral_dev_t *dev = (demo_peripheral_dev_t *)param;
//	printk(KERN_DEBUG "l(%4d): %s(): irq=%d\n", __LINE__, __PRETTY_FUNCTION__, irq);
	if (irq != gpio_to_irq(SENSOR_GPIO_PORT)) {
		return IRQ_NONE;
	}
	dev->poll_status = 1;
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
	__poll_t mask = 0;
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
	return 0;
}

/**
 *
 */
static int demo_peripheral_dev_resume(struct device *dev)
{
	demo_peripheral_dev_t *pdev = dev_get_drvdata(dev);
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

	dev = (demo_peripheral_dev_t *)kzalloc(RNDUP(sizeof(demo_peripheral_dev_t), 16), GFP_KERNEL);
	dev->spi = spi;

	init_waitqueue_head(&(dev->wait_queue));

#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
	dev->work_queue = create_singlethread_workqueue("demo_peripheral_dev_work_queue");
	if (!dev->work_queue) {
		return -EBADF;
	}
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */

	dev->gp_reg = (bcm2835_reg_gp_t *)ioremap_nocache(BCM2835_REG_GP_BASE, PAGE_SIZE);
	// gpio22: gpi
	dev->gp_reg->fsel2 &= ~(0x07u << SENSOR_GPFSEL_BIT);
	// gpio22: fall edge detect off
	dev->gp_reg->fen0 &= ~(1u << SENSOR_GPIO_PORT);
	// gpio22: raise edge detect on
	dev->gp_reg->ren0 |= (1u << SENSOR_GPIO_PORT);

	// gpio22 pull-down
	dev->gp_reg->pud = 0x00000001u;
	msleep(1);
	dev->gp_reg->pudclk0 = (0x01u << SENSOR_GPIO_PORT);
	msleep(1);
	dev->gp_reg->pud = 0x00000000u;
	dev->gp_reg->pudclk0 = 0x00000000u;
	msleep(1);

	result = request_irq(gpio_to_irq(SENSOR_GPIO_PORT),
			demo_peripheral_dev_irq_handler, IRQF_SHARED,
			"demo_peripheral_dev_irq_handler", dev);
	if (result != 0)	{
		printk(KERN_ERR "l(%4d): %s() %d\n", __LINE__, __PRETTY_FUNCTION__, result);
		return -EACCES;
	}
	enable_irq(gpio_to_irq(SENSOR_GPIO_PORT));

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
	return 0;
}

/**
 *
 */
static int demo_peripheral_dev_remove(struct spi_device *spi)
{
	int minor;
	demo_peripheral_dev_t *dev = spi_get_drvdata(spi);

	disable_irq(gpio_to_irq(SENSOR_GPIO_PORT));
	free_irq(gpio_to_irq(SENSOR_GPIO_PORT), dev);

	for (minor = 0; minor < TARGET_DEVICES; minor++)	{
		device_destroy(dev->class_inst, MKDEV(MAJOR(dev->dev_inst), minor));
	}
	cdev_del(&(dev->cdev_inst));
	unregister_chrdev_region(dev->dev_inst, TARGET_DEVICES);
	class_destroy(dev->class_inst);

#ifdef __DEMO_PERIPHERAL_DEV_ASYNC
	destroy_workqueue(dev->work_queue);
#endif /* __DEMO_PERIPHERAL_DEV_ASYNC */
	iounmap(dev->gp_reg);
	kfree(dev);

	return 0;
}

/**
 *
 */
static const struct of_device_id demo_peripheral_dev_of_ids[] = {
	{ .compatible = "demo-spidev" },
	{}
};
MODULE_DEVICE_TABLE(of, demo_peripheral_dev_of_ids);

/**
 *
 */
static const struct spi_device_id demo_peripheral_dev_ids[] = {
    {"demo-spidev", 0},
    {},
};
MODULE_DEVICE_TABLE(spi, demo_peripheral_dev_ids);

/**
 *
 */
static struct spi_driver demo_peripheral_dev_driver = {
	.driver = {
		.name			= "demo-peripheral-device",
		.pm				= &demo_peripheral_dev_pm_ops,
		.of_match_table = of_match_ptr(demo_peripheral_dev_of_ids),
	},
	.id_table	= demo_peripheral_dev_ids,
	.probe		= demo_peripheral_dev_probe,
	.remove		= demo_peripheral_dev_remove,
};

module_spi_driver(demo_peripheral_dev_driver);

MODULE_AUTHOR("hoge");
MODULE_DESCRIPTION("peripheral device driver for demo.");
MODULE_LICENSE("GPL v2");
