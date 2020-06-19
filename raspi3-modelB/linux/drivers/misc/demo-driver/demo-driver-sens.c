


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
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/gpio.h>

// #define	LOCAL_DEBUG_LOG
#define	DEMO_CFG_SW_PU_ENABLE

#define	DEVICE_NAME			("demo_device")
#define	CONTROL_DEVICE_MAX	(1u)

#define	DEMO_DEV_IOCTL_SET_SW_IRQ		(0x0001)
#define	DEMO_DEV_IOCTL_GET_BME280		(0x0100)
#define	DEMO_DEV_IOCTL_SET_ILM_00		(0x0200)
#define	DEMO_DEV_IOCTL_SET_ILM_01		(0x0201)
#define	DEMO_DEV_IOCTL_GET_ILM_00		(0x0210)
#define	DEMO_DEV_IOCTL_GET_ILM_01		(0x0211)

#define	DEMO_DEV_READ_REBOOT			(0x00)
#define	DEMO_DEV_READ_POWOFF			(0x01)

#define BCM2835_CFG_GPIO_SW_BASE		(20)
#define BCM2835_CFG_GPIO_SW_REBOOT		\
		(BCM2835_CFG_GPIO_SW_BASE + DEMO_DEV_READ_REBOOT)
#define BCM2835_CFG_GPIO_SW_POWOFF		\
		(BCM2835_CFG_GPIO_SW_BASE + DEMO_DEV_READ_POWOFF)

#define	BCM2835_CFG_PWM_FSEL_ALT0		(0)
#define	BCM2835_CFG_PWM_FSEL_ALT5		(5)
#define	BCM2835_CFG_PWM0_FSEL_ALT		BCM2835_CFG_PWM_FSEL_ALT0
#define	BCM2835_CFG_PWM1_FSEL_ALT		BCM2835_CFG_PWM_FSEL_ALT0

#define	HSWAP(x)	(x)

// #define	BCM2835_REG_PWM_BASE		(0x7e20c000)
// #define	BCM2835_REG_GP_BASE			(0x7e200000)
// #define	BCM2835_REG_CM_BASE			(0x7e101000)
#define	BCM2835_REG_PWM_BASE			(0x3f20c000)
#define	BCM2835_REG_GP_BASE				(0x3f200000)
#define	BCM2835_REG_CM_BASE				(0x3f101000)

#define	BCM2835_PWM_PERIOD				(1024u)

#define	BME280_I2C_CH					(1)
#define	BME280_SLAVE_ADDRESS			(0x76)

#define	BME280_REG_HUM_MSB				(0xfd)
#define	BME280_REG_HUM_LSB				(0xfe)
#define	BME280_REG_TEMP_MSB				(0xfa)
#define	BME280_REG_TEMP_LSB				(0xfb)
#define	BME280_REG_PRESS_MSB			(0xf7)
#define	BME280_REG_PRESS_LSB			(0xf8)

#define	BME280_REG_CONFIG				(0xf5)
#define	BME280_REG_CTRL_MEAS			(0xf4)
#define	BME280_REG_STATUS				(0xf3)
#define	BME280_REG_CTRL_HUM				(0xf2)

#define	BME280_REG_RESET				(0xe0)
#define	BME280_REG_ID					(0xd0)

#define	BME280_REG_CTRL_MEAS_VAL		((1 << 5) | (1 << 2) | (1))
#define	BME280_REG_CTRL_HUM_VAL			(0x01)

#define	BME280_REG_CTRL_MEAS_MODE_MSK	(0x03)
#define	BME280_REG_STATUS_MERSURING_MSK	(0x08)

typedef	struct demo_drv_ioctl_bme280_info_t {
	uint32_t	temp;
	uint32_t	press;
	uint32_t	hum;
}	demo_drv_ioctl_bme280_info_t;

typedef	struct bcm2835_reg_pwm_t {
	uint32_t	ctl;
	uint32_t	sta;
	uint32_t	dmac;
	uint32_t	reserve1;
	uint32_t	rng1;
	uint32_t	dat1;
	uint32_t	fif1;
	uint32_t	reserve2;
	uint32_t	rng2;
	uint32_t	dat2;
}	bcm2835_reg_pwm_t;

typedef	struct bcm2835_reg_gp_t {
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

#pragma	pack(1)
typedef	struct demo_drv_bme280_raw_calib_1st_t {
	uint16_t	dig_T1;			// 0x88 / 0x89
	uint16_t	dig_T2;			// 0x8A / 0x8B
	uint16_t	dig_T3;			// 0x8C / 0x8D
	uint16_t	dig_P1;			// 0x8E / 0x8F
	uint16_t	dig_P2;			// 0x90 / 0x91
	uint16_t	dig_P3;			// 0x92 / 0x93
	uint16_t	dig_P4;			// 0x94 / 0x95
	uint16_t	dig_P5;			// 0x96 / 0x97
	uint16_t	dig_P6;			// 0x98 / 0x99
	uint16_t	dig_P7;			// 0x9A / 0x9B
	uint16_t	dig_P8;			// 0x9C / 0x9D
	uint16_t	dig_P9;			// 0x9E / 0x9F
}	demo_drv_bme280_raw_calib_1st_t;

typedef	struct demo_drv_bme280_raw_calib_2nd_t {
	uint8_t		dig_H1;			// 0xA1
}	demo_drv_bme280_raw_calib_2nd_t;

typedef	struct demo_drv_bme280_raw_calib_3rd_t {
	uint16_t	dig_H2;			// 0xE1 / 0xE2
	uint8_t		dig_H3;			// 0xE3
	uint8_t		dig_H4_E4;		// 0xE4			dig_H4[11:4]
	uint8_t		dig_H4_E5 : 4;	// 0xE5[3:0]	dig_H4[3:0]
	uint8_t		dig_H5_E5 : 4;	// 0xE5[7:4]	dig_H5[3:0]
	uint8_t		dig_H5_E6;		// 0xE6			dig_H5[11:4]
	uint8_t		dig_H6;			// 0xE7
}	demo_drv_bme280_raw_calib_3rd_t;
#pragma	pack()

typedef	struct demo_drv_bme280_calib_t {
	uint16_t	dig_T1;	// 0x88 / 0x89
	int16_t		dig_T2;	// 0x8A / 0x8B
	int16_t		dig_T3;	// 0x8C / 0x8D
	uint16_t	dig_P1;	// 0x8E / 0x8F
	int16_t		dig_P2;	// 0x90 / 0x91
	int16_t		dig_P3;	// 0x92 / 0x93
	int16_t		dig_P4;	// 0x94 / 0x95
	int16_t		dig_P5;	// 0x96 / 0x97
	int16_t		dig_P6;	// 0x98 / 0x99
	int16_t		dig_P7;	// 0x9A / 0x9B
	int16_t		dig_P8;	// 0x9C / 0x9D
	int16_t		dig_P9;	// 0x9E / 0x9F
	uint8_t		dig_H1;	// 0xA1
	int16_t		dig_H2;	// 0xE1 / 0xE2
	uint8_t		dig_H3;	// 0xE3
	int16_t		dig_H4;	// 0xE4 / 0xE5[3:0]
	int16_t		dig_H5;	// 0xE5[7:4] / 0xE6
	int8_t		dig_H6;	// 0xE7
}	demo_drv_bme280_calib_t;

typedef struct demo_dev_devices_t	{
	struct class		*class_inst;
	dev_t				dev_inst;
	struct cdev			cdev_inst;
	struct i2c_client	*i2c;
	struct device		*dev[CONTROL_DEVICE_MAX];
	unsigned			assert_reboot;
	unsigned			assert_powoff;
}	demo_dev_devices_t;

extern void sc1602bs_initialize(void);
extern void sc1602bs_finalize(void);
extern void sc1602bs_indicate_string(char *str);
extern void sc1602bs_clear_display(void);

static int bme280_write_reg(struct i2c_client *client, char addr,
		unsigned char *data, int len);
static int bme280_read_reg(struct i2c_client *client, char addr,
		unsigned char *buff, int *len, int req_len);

static int __init	demo_dev_init_driver(void);
static void __exit	demo_dev_cleanup_driver(void);
static int	demo_dev_driver_open(struct inode *inode,
		struct file *filp);
static int	demo_dev_driver_release(struct inode *inode,
		struct file *filp);
static long	demo_dev_driver_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg);
static ssize_t	demo_dev_driver_read(struct file *filp, char __user *buf,
			   size_t count, loff_t *ppos);
static ssize_t	demo_dev_driver_write(struct file *filp,
		const char __user *buf, size_t count, loff_t *ppos);
static unsigned int demo_dev_driver_poll(struct file* filp, poll_table* wait);

static int	demo_dev_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
static int	demo_dev_i2c_remove(struct i2c_client *client);

static const struct i2c_device_id demo_dev_i2c_idtable[] = {
	{"demo_bme280", 0},
	{}
};


static const struct i2c_board_info demo_dev_i2c_device = {
	I2C_BOARD_INFO("demo_bme280", BME280_SLAVE_ADDRESS),
};

static struct i2c_driver demo_dev_i2c_drv = {
	.driver		= {
		.name	= "demo device driver",
	},
	.probe		= demo_dev_i2c_probe,
	.remove		= demo_dev_i2c_remove,
	.id_table	= demo_dev_i2c_idtable,
};

static const struct file_operations demo_dev_device_fops = {
	.owner			= THIS_MODULE,
	.open			= demo_dev_driver_open,
	.release		= demo_dev_driver_release,
	.unlocked_ioctl	= demo_dev_driver_ioctl,
	.read			= demo_dev_driver_read,
	.write			= demo_dev_driver_write,
	.poll			= demo_dev_driver_poll,
};

static demo_dev_devices_t	demo_drv_devices;

static bcm2835_reg_pwm_t	*demo_drv_pwm_reg;
static bcm2835_reg_gp_t		*demo_drv_gp_reg;
static uint32_t				*bcm2835_reg_cm_base;

static wait_queue_head_t demo_drv_read_q;

/* -------------------------------------------------------------------------- */
static void debug_dump(uint8_t *data, int size, const int line)
{
#ifdef	LOCAL_DEBUG_LOG
	int i;
	uint8_t xbuf[16];

	printk(KERN_DEBUG "(size=%dbytes): l(%d)\n", size, line);
	printk(KERN_DEBUG "    | +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +a +b +c +d +e +f\n");
	printk(KERN_DEBUG "-----------------------------------------------------\n");
	for (i = 0; size > 0; i += sizeof(xbuf), size -= sizeof(xbuf)) {
		memset(xbuf, 0xcc, sizeof(xbuf));
		memcpy(xbuf, &data[i], (size >= (int)sizeof(xbuf))?
				sizeof(xbuf) : size);
		printk(KERN_DEBUG "%04x| %02x %02x %02x %02x %02x %02x %02x %02x"
				" %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
				xbuf[0], xbuf[1], xbuf[2], xbuf[3], xbuf[4],
				xbuf[5], xbuf[6], xbuf[7], xbuf[8], xbuf[9],
				xbuf[10], xbuf[11], xbuf[12], xbuf[13], xbuf[14], xbuf[15]);
	}
	printk(KERN_DEBUG
			"----------------------------------------------------- (end)\n");
#endif	/* LOCAL_DEBUG_LOG */
}

/* -------------------------------------------------------------------------- */
volatile uint32_t *bcm2835_get_register_gpio_data(uint32_t reg_no, uint32_t ctrl)
{
	struct {
		volatile uint32_t *clr;
		volatile uint32_t *set;
	} reg[2] = {
		{
			(volatile uint32_t *)&(demo_drv_gp_reg->clr0),
			(volatile uint32_t *)&(demo_drv_gp_reg->set0)
		},
		{
			(volatile uint32_t *)&(demo_drv_gp_reg->clr1),
			(volatile uint32_t *)&(demo_drv_gp_reg->set1)
		}
	};

	if (reg_no > (sizeof(reg) / sizeof(reg[0]))) {
		return NULL;
	}
	ctrl = !(!ctrl);
	return ((ctrl)? (reg[reg_no].set) : (reg[reg_no].clr));
}
// EXPORT_SYMBOL(bcm2835_get_register_gpio_data);


/* -------------------------------------------------------------------------- */
long bme280_compensate_temp(int32_t adc_T,
		demo_drv_bme280_calib_t *calib, int32_t *t_fine)
{
	long var1, var2, T;
	var1 = ((((adc_T >> 3) - ((long)(calib->dig_T1) << 1))) * ((long)(calib->dig_T2))) >> 11;
	var2 = (((((adc_T >> 4) - ((long)(calib->dig_T1))) * ((adc_T >> 4) -
			((long)(calib->dig_T1)))) >> 12) * ((long)(calib->dig_T3))) >> 14;

	*t_fine = var1 + var2;
	T = ((*t_fine) * 5 + 128) >> 8;
	return T;
}

/* -------------------------------------------------------------------------- */
unsigned long bme280_compensate_press(int32_t adc_P,
		demo_drv_bme280_calib_t *calib, int32_t t_fine)
{
	long var1, var2;
	unsigned long P;
	var1 = (((long)t_fine) >> 1) - (long)64000;
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((long)(calib->dig_P6));
	var2 = var2 + ((var1*((long)(calib->dig_P5))) << 1);
	var2 = (var2 >> 2)+(((long)(calib->dig_P4)) << 16);
	var1 = ((((calib->dig_P3) * (((var1>>2)*(var1>>2)) >> 13)) >>3) + ((((long)(calib->dig_P2)) * var1)>>1))>>18;
	var1 = ((((32768+var1))*((long)(calib->dig_P1)))>>15);
	if (var1 == 0) {
		return 0;
	}
	P = (((unsigned long)(((long)1048576)-adc_P)-(var2>>12)))*3125;
	if (P < 0x80000000) {
		P = (P << 1) / ((unsigned long) var1);
	} else {
		P = (P / (unsigned long)var1) * 2; 
	}
	var1 = (((long)(calib->dig_P9)) * ((long)(((P>>3) * (P>>3))>>13)))>>12;
	var2 = (((long)(P>>2)) * ((long)(calib->dig_P8)))>>13;
	P = (unsigned long)((long)P + ((var1 + var2 + (calib->dig_P7)) >> 4));
	return P;
}

/* -------------------------------------------------------------------------- */
unsigned long bme280_compensate_hum(int32_t adc_H,
		demo_drv_bme280_calib_t *calib, int32_t t_fine)
{
	long v_x1;
	v_x1 = (t_fine - ((long)76800));
	v_x1 = (((((adc_H << 14) -(((long)(calib->dig_H4)) << 20) - (((long)(calib->dig_H5)) * v_x1)) +
			((long)16384)) >> 15) * (((((((v_x1 * ((long)(calib->dig_H6))) >> 10) * 
			(((v_x1 * ((long)(calib->dig_H3))) >> 11) + ((long) 32768))) >> 10) + ((long)2097152)) *
			((long)(calib->dig_H2)) + 8192) >> 14));
	v_x1 = (v_x1 - (((((v_x1 >> 15) * (v_x1 >> 15)) >> 7) * ((long)(calib->dig_H1))) >> 4));
	v_x1 = (v_x1 < 0 ? 0 : v_x1);
	v_x1 = (v_x1 > 419430400 ? 419430400 : v_x1);
	return (unsigned long)(v_x1 >> 12);
}

/* -------------------------------------------------------------------------- */
static int bme280_write_reg(struct i2c_client *client, char addr,
		unsigned char *data, int len)
{
	int result;
	unsigned char *p;
	p = (unsigned char *)kmalloc(len + 1, GFP_KERNEL);
	if (!p) {
		return -ENOMEM;
	}
	p[0] = addr;
	memcpy(&p[1], data, len);
	len++;
	result = i2c_master_send(client, p, len);
	if (unlikely(result != len)) {
		dev_err(&client->dev, "sent %d bytes of %d total\n",
				len, result);
		result = -EIO;
		goto free_kmem;
	}

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d): success\n",
			__FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	result = 0;

free_kmem:
	kfree(p);
	return result;
}

/* -------------------------------------------------------------------------- */
static int bme280_read_reg(struct i2c_client *client, char addr,
		unsigned char *buff, int *len, int req_len)
{
	int result;

	result = i2c_master_send(client, &addr, 1);
	if (unlikely(result != 1)) {
		dev_err(&client->dev, "sent addr result=%d\n",
				result);
		result = -EIO;
		return result;
	}

	result = i2c_master_recv(client, buff, req_len);
	if (unlikely(result != req_len)) {
		dev_err(&client->dev, "wanted %d bytes, got %d\n",
				req_len, result);
		result = -EIO;
		return result;
	}
	*len = req_len;
	result = 0;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d): success\n",
			__FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	return result;
}

/* -------------------------------------------------------------------------- */
static int	bme280_read_calib(struct i2c_client *client,
		demo_drv_bme280_calib_t *calib)
{
	int len;
	int result;

	demo_drv_bme280_raw_calib_1st_t raw1;
	demo_drv_bme280_raw_calib_2nd_t raw2;
	demo_drv_bme280_raw_calib_3rd_t raw3;

	result = bme280_read_reg(client, 0x88, (unsigned char *)&raw1,
			&len, sizeof(raw1));
	if (result < 0) {
		return result;
	}
	debug_dump((uint8_t *)&raw1, sizeof(raw1), __LINE__);

	result = bme280_read_reg(client, 0xa1, (unsigned char *)&raw2,
			&len, sizeof(raw2));
	if (result < 0) {
		return result;
	}
	debug_dump((uint8_t *)&raw2, sizeof(raw2), __LINE__);

	result = bme280_read_reg(client, 0xe1, (unsigned char *)&raw3,
			&len, sizeof(raw3));
	if (result < 0) {
		return result;
	}
	debug_dump((uint8_t *)&raw3, sizeof(raw3), __LINE__);

	calib->dig_T1 = HSWAP(raw1.dig_T1);
	calib->dig_T2 = (int16_t)HSWAP(raw1.dig_T2);
	calib->dig_T3 = (int16_t)HSWAP(raw1.dig_T3);
	calib->dig_P1 = HSWAP(raw1.dig_P1);
	calib->dig_P2 = (int16_t)HSWAP(raw1.dig_P2);
	calib->dig_P3 = (int16_t)HSWAP(raw1.dig_P3);
	calib->dig_P4 = (int16_t)HSWAP(raw1.dig_P4);
	calib->dig_P5 = (int16_t)HSWAP(raw1.dig_P5);
	calib->dig_P6 = (int16_t)HSWAP(raw1.dig_P6);
	calib->dig_P7 = (int16_t)HSWAP(raw1.dig_P7);
	calib->dig_P8 = (int16_t)HSWAP(raw1.dig_P8);
	calib->dig_P9 = (int16_t)HSWAP(raw1.dig_P9);

	calib->dig_H1 = raw2.dig_H1;

	calib->dig_H2 = (int16_t)HSWAP(raw3.dig_H2);
	calib->dig_H3 = raw3.dig_H3;
	calib->dig_H4 = (int16_t)(((raw3.dig_H4_E4) << 4) | (raw3.dig_H4_E5));
	calib->dig_H5 = (int16_t)(((raw3.dig_H5_E6) << 4) | (raw3.dig_H5_E5));
	calib->dig_H6 = (int8_t)raw3.dig_H6;

	debug_dump((uint8_t *)calib, sizeof(*calib), __LINE__);

	return result;
}

/* -------------------------------------------------------------------------- */
static int	bme280_read_info(struct i2c_client *client,
		demo_drv_ioctl_bme280_info_t *data)
{
	uint8_t bytes[32];
	int result;
	int len;
	demo_drv_bme280_calib_t calib;
	int32_t t_fine;

	bytes[0] = BME280_REG_CTRL_MEAS_VAL;
	result = bme280_write_reg(client, BME280_REG_CTRL_MEAS,
			(unsigned char *)bytes, 1);
	if (result < 0) {
		return result;
	}

	do {
		bytes[0] = 0xff;
		result = bme280_read_reg(client, BME280_REG_STATUS,
				(unsigned char *)&bytes[0], &len, 1);
		if (result < 0) {
			return result;
		}
	} while ((bytes[0] & BME280_REG_STATUS_MERSURING_MSK));

	result = bme280_read_calib(client, &calib);
	if (result < 0) {
		return result;
	}

	/* 0xFA temp_msb[7:0]
	     Contains the MSB part ut[19:12] of the raw temperature
	     measurement output data.
	   0xFB temp_lsb[7:0]
	     Contains the LSB part ut[11:4] of the raw temperature
	     measurement output data.
	   0xFC (bit 7, 6, 5, 4) temp_xlsb[3:0]
	     Contains the XLSB part ut[3:0] of the raw temperature
	     measurement output data. Contents depend on pressure resolution. */
	result = bme280_read_reg(client, BME280_REG_TEMP_MSB, bytes, &len, 3);
	if (result < 0) {
		return result;
	}
	data->temp = ((((uint32_t)(bytes[0])) << 12) |
			(((uint32_t)(bytes[1])) << 4) | (((uint32_t)(bytes[2])) >> 4));
	debug_dump(bytes, 3, __LINE__);
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "T_raw=%d(%08x)\n", data->temp, data->temp);
#endif	// LOCAL_DEBUG_LOG

	/* 0xF7 press_msb[7:0]
	     Contains the MSB part up[19:12] of the raw pressure
	     measurement output data.
	   0xF8 press_lsb[7:0]
	     Contains the LSB part up[11:4] of the raw pressure
	     measurement output data.
	   0xF9 (bit 7, 6, 5, 4) press_xlsb[3:0]
	     Contains the XLSB part up[3:0] of the raw pressure
	     measurement output data. Contents depend on temperature resolution. */
	result = bme280_read_reg(client, BME280_REG_PRESS_MSB, bytes, &len, 3);
	if (result < 0) {
		return result;
	}
	data->press = ((((uint32_t)(bytes[0])) << 12) |
			(((uint32_t)(bytes[1])) << 4) | (((uint32_t)(bytes[2])) >> 4));
	debug_dump(bytes, 3, __LINE__);
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "P_raw=%d(%08x)\n", data->press, data->press);
#endif	// LOCAL_DEBUG_LOG

	/* 0xFD hum_msb[7:0]
	     Contains the MSB part uh[15:8] of the raw humidity
	     measurement output data.
	   0xFE temp_lsb[7:0]
	     Contains the LSB part uh[7:0] of the raw humidity
	     measurement output data. */
	result = bme280_read_reg(client, BME280_REG_HUM_MSB, bytes, &len,
			sizeof(unsigned short));
	if (result < 0) {
		return result;
	}
	data->hum = ((((uint32_t)(bytes[0])) << 8) | ((uint32_t)(bytes[1])));
	debug_dump(bytes, 2, __LINE__);
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "H_raw=%d(%08x)\n", data->hum, data->hum);
#endif	// LOCAL_DEBUG_LOG

	data->temp = bme280_compensate_temp(data->temp, &calib, &t_fine);
	data->press = bme280_compensate_press(data->press, &calib, t_fine);
	data->hum = bme280_compensate_hum(data->hum, &calib, t_fine);

	return result;
}

/* -------------------------------------------------------------------------- */
static void bcm2835_set_pwm(int ch, uint32_t width)
{
	if (BCM2835_PWM_PERIOD < width) {
		width = BCM2835_PWM_PERIOD;
	}

	switch (ch) {
	case 0:
	{
#if (BCM2835_CFG_PWM0_FSEL_ALT == BCM2835_CFG_PWM_FSEL_ALT5)
		if (width) {
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (8 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x02) << (8 * 3));	// GPIO18 alt5
			demo_drv_pwm_reg->rng1 = BCM2835_PWM_PERIOD;
			demo_drv_pwm_reg->dat1 = width;
			demo_drv_pwm_reg->ctl &= ~(0x000000ff);
			demo_drv_pwm_reg->ctl |= 0x00000081;
		} else {
			demo_drv_gp_reg->clr0 = (1u << 18);
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (8 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x01) << (8 * 3));	// GPIO18 gpo
			demo_drv_pwm_reg->ctl &= ~(0x000000ff);
			demo_drv_pwm_reg->dat1 = width;
		}
#else	/* if (BCM2835_CFG_PWM0_FSEL_ALT == BCM2835_CFG_PWM_FSEL_ALT0) */
		if (width) {
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (2 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x04) << (2 * 3));	// GPIO12 alt0
			demo_drv_pwm_reg->rng1 = BCM2835_PWM_PERIOD;
			demo_drv_pwm_reg->dat1 = width;
			demo_drv_pwm_reg->ctl &= ~(0x000000ff);
			demo_drv_pwm_reg->ctl |= 0x00000081;
		} else {
			demo_drv_gp_reg->clr0 = (1u << 12);
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (2 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x01) << (2 * 3));	// GPIO12 gpo
			demo_drv_pwm_reg->ctl &= ~(0x000000ff);
			demo_drv_pwm_reg->dat1 = width;
		}
#endif	/* BCM2835_CFG_PWM0_FSEL_ALT */
		break;
	}
	case 1:
	{
#if (BCM2835_CFG_PWM1_FSEL_ALT == BCM2835_CFG_PWM_FSEL_ALT5)
		if (width) {
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (9 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x02) << (9 * 3));	// GPIO19 alt5
			demo_drv_pwm_reg->rng2 = BCM2835_PWM_PERIOD;
			demo_drv_pwm_reg->dat2 = width;
			demo_drv_pwm_reg->ctl &= ~(0x000000ff << 8);
			demo_drv_pwm_reg->ctl |= (0x00000081 << 8);
		} else {
			demo_drv_gp_reg->clr0 = (1u << 19);
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (9 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x01) << (9 * 3));	// GPIO19 gpo
			demo_drv_pwm_reg->ctl &= ~(0x000000ff << 8);
			demo_drv_pwm_reg->dat2 = width;
		}
#else	/* if (BCM2835_CFG_PWM1_FSEL_ALT == BCM2835_CFG_PWM_FSEL_ALT0) */
		if (width) {
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (3 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x04) << (3 * 3));	// GPIO13 alt0
			demo_drv_pwm_reg->rng2 = BCM2835_PWM_PERIOD;
			demo_drv_pwm_reg->dat2 = width;
			demo_drv_pwm_reg->ctl &= ~(0x000000ff << 8);
			demo_drv_pwm_reg->ctl |= (0x00000081 << 8);
		} else {
			demo_drv_gp_reg->clr0 = (1u << 13);
			demo_drv_gp_reg->fsel1 &= ~((0x07) << (3 * 3));
			demo_drv_gp_reg->fsel1 |= ((0x01) << (3 * 3));	// GPIO13 gpo
			demo_drv_pwm_reg->ctl &= ~(0x000000ff << 8);
			demo_drv_pwm_reg->dat2 = width;
		}
#endif	/* BCM2835_CFG_PWM1_FSEL_ALT */
		break;
	}
	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */
static uint32_t bcm2835_get_width_pwm(int ch)
{
	switch (ch) {
	case 0:
		return demo_drv_pwm_reg->dat1;
	case 1:
		return demo_drv_pwm_reg->dat2;
	default:
		break;
	}
	return ~0u;
}

/* -------------------------------------------------------------------------- */
static irqreturn_t demo_dev_sw_irq_handler(int irq, void *param)
{
	irqreturn_t result = IRQ_NONE;
	demo_dev_devices_t	*dev = (demo_dev_devices_t *)param;

	printk(KERN_DEBUG "l(%4d): %s(): irq=%d\n", __LINE__, __PRETTY_FUNCTION__, irq);

	if (irq == gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT)) {
		dev->assert_reboot = 1u;
		result = IRQ_HANDLED;
	}
	if (irq == gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF)) {
		dev->assert_powoff = 1u;
		result = IRQ_HANDLED;
	}

	if (result == IRQ_NONE) {
		return(result);
	}

//	disable_irq_nosync(irq);
	wake_up_interruptible(&demo_drv_read_q);

	return(result);
}

/* -------------------------------------------------------------------------- */
static int	demo_dev_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int	result = 0;
	int len;
	unsigned char data;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s: SLA(0x%02x) id(%d)\n", __PRETTY_FUNCTION__,
			client->addr, (int)id->driver_data);
#endif	// LOCAL_DEBUG_LOG

	i2c_set_clientdata(client, &demo_drv_devices);

	/* device initialize */
	data = 0xb6;
	bme280_write_reg(client, BME280_REG_RESET, &data, sizeof(data));

	result = bme280_read_reg(client, BME280_REG_ID, &data, &len, sizeof(data));
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d): ID=%02x\n",
			__FILE__, __PRETTY_FUNCTION__, __LINE__, data);
#endif	// LOCAL_DEBUG_LOG

	data = BME280_REG_CTRL_HUM_VAL;
	bme280_write_reg(client, BME280_REG_CTRL_HUM, &data, sizeof(data));

	data = BME280_REG_CTRL_MEAS_VAL;
	bme280_write_reg(client, BME280_REG_CTRL_MEAS, &data, sizeof(data));

	return (result);
}

/* -------------------------------------------------------------------------- */
static int	demo_dev_i2c_remove(struct i2c_client *client)
{
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s: SLA(0x%02x)\n", __PRETTY_FUNCTION__, client->addr);
#endif	// LOCAL_DEBUG_LOG
	return(0);
}

/* -------------------------------------------------------------------------- */
static int __init	demo_dev_init_driver(void)
{
	int result, minor;
	struct i2c_adapter *adapt;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	memset(&demo_drv_devices, 0, sizeof(demo_drv_devices));

	demo_drv_pwm_reg	= (bcm2835_reg_pwm_t *)ioremap_nocache(
			BCM2835_REG_PWM_BASE, PAGE_SIZE);
	demo_drv_gp_reg		= (bcm2835_reg_gp_t *)ioremap_nocache(
			BCM2835_REG_GP_BASE, PAGE_SIZE);
	bcm2835_reg_cm_base	= (uint32_t *)ioremap_nocache(
			BCM2835_REG_CM_BASE, PAGE_SIZE);

	// gpio20: gpi
	demo_drv_gp_reg->fsel2 &= ~(0x07u << (3 * DEMO_DEV_READ_REBOOT));
	// gpio20: fall edge detect on
	demo_drv_gp_reg->fen0 |= (1u << BCM2835_CFG_GPIO_SW_REBOOT);
#ifndef DEMO_CFG_SW_PU_ENABLE
	// gpio20: raise edge detect on
	demo_drv_gp_reg->ren0 |= (1u << BCM2835_CFG_GPIO_SW_REBOOT);
#endif /* DEMO_CFG_SW_PU_ENABLE */

	// gpio21: gpi
	demo_drv_gp_reg->fsel2 &= ~(0x07u << (3 * DEMO_DEV_READ_POWOFF));
	// gpio21: fall edge detect on
	demo_drv_gp_reg->fen0 |= (1u << BCM2835_CFG_GPIO_SW_POWOFF);
#ifndef DEMO_CFG_SW_PU_ENABLE
	// gpio21: raise edge detect on
	demo_drv_gp_reg->ren0 |= (1u << BCM2835_CFG_GPIO_SW_POWOFF);
#endif /* DEMO_CFG_SW_PU_ENABLE */

#ifdef DEMO_CFG_SW_PU_ENABLE
	// gpio20 / gpio21 pull-up
	demo_drv_gp_reg->pud = 0x00000002u;
	msleep(1);
	demo_drv_gp_reg->pudclk0 = (0x03u << 20);
	msleep(1);
	demo_drv_gp_reg->pud = 0x00000000u;
	demo_drv_gp_reg->pudclk0 = 0x00000000u;
	msleep(1);
#endif /* DEMO_CFG_SW_PU_ENABLE */

	printk(KERN_DEBUG "gplev0=0x%08x\n", demo_drv_gp_reg->lev0);

	sc1602bs_initialize();

	result = request_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT),
			demo_dev_sw_irq_handler, IRQF_SHARED, "demo_drv_irq_reboot_gpio",
			&demo_drv_devices);
	if (result != 0)	{
		printk(KERN_ERR "l(%4d): %s() %d\n",
			__LINE__, __PRETTY_FUNCTION__, result);
	}

	result = request_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF),
			demo_dev_sw_irq_handler, IRQF_SHARED, "demo_drv_irq_powoff_gpio",
			&demo_drv_devices);
	if (result != 0)	{
		printk(KERN_ERR "l(%4d): %s() %d\n",
			__LINE__, __PRETTY_FUNCTION__, result);
	}

	enable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT));
	enable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF));

	bcm2835_reg_cm_base[40] = 0x5a000000 | (1u << 5);
	msleep(1);
	while ((bcm2835_reg_cm_base[40] & (1u << 7))) {}

	bcm2835_reg_cm_base[41] = 0x5a000000 | (32u << 12);
	bcm2835_reg_cm_base[40] = 0x5a000000 | 0x001;
	bcm2835_reg_cm_base[40] = 0x5a000000 | (1u << 4) | 0x001;

	msleep(1);
	while (!(bcm2835_reg_cm_base[40] & (1u << 7))) {}

	demo_drv_pwm_reg->rng1 = BCM2835_PWM_PERIOD;
	demo_drv_pwm_reg->rng2 = BCM2835_PWM_PERIOD;

	bcm2835_set_pwm(1, 100);

	demo_drv_devices.class_inst = class_create(THIS_MODULE, DEVICE_NAME);

	result = alloc_chrdev_region(&demo_drv_devices.dev_inst,
			0u, CONTROL_DEVICE_MAX, DEVICE_NAME);
	if (unlikely(result < 0))	{
		printk(KERN_ERR "l(%4d): %s %d\n", __LINE__, __PRETTY_FUNCTION__, result);
	}

	init_waitqueue_head(&demo_drv_read_q);

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
						minor), minor, filename_fmt, minor);
		if (unlikely(IS_ERR(demo_drv_devices.dev[minor])))	{
			printk(KERN_ERR "%s::%s(): l(%d): %lx\n", __FILE__,
					__PRETTY_FUNCTION__, __LINE__, (unsigned long)
					demo_drv_devices.dev[minor]);
		}
	}

	result = i2c_add_driver(&demo_dev_i2c_drv);
	if (result != 0)	{
		printk(KERN_ERR "%s(): l(%d): %d\n", __PRETTY_FUNCTION__, __LINE__, result);
		return(result);
	}

	adapt = i2c_get_adapter(BME280_I2C_CH);
	if (!adapt)	{
		printk(KERN_ERR "%s(): l(%d)\n", __PRETTY_FUNCTION__, __LINE__);
		result = -ENOENT;
		return(result);
	}

	demo_drv_devices.i2c = i2c_new_device(adapt, &demo_dev_i2c_device);
	if (!demo_drv_devices.i2c)	{
		printk(KERN_ERR "%s(): l(%d)\n", __PRETTY_FUNCTION__, __LINE__);
		result = -EIO;
		goto exit_proc;
	}

exit_proc:
	i2c_put_adapter(adapt);

	return(result);
}

/* -------------------------------------------------------------------------- */
static void __exit	demo_dev_cleanup_driver(void)
{
	int minor;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(): l(%d)\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	// gpio20: foll edge detect off
	demo_drv_gp_reg->fen0 &= ~(1u << BCM2835_CFG_GPIO_SW_REBOOT);
#ifndef DEMO_CFG_SW_PU_ENABLE
	// gpio20: raise edge detect off
	demo_drv_gp_reg->ren0 &= ~(1u << BCM2835_CFG_GPIO_SW_REBOOT);
#endif /* DEMO_CFG_SW_PU_ENABLE */

	// gpio21: foll edge detect off
	demo_drv_gp_reg->fen0 &= ~(1u << BCM2835_CFG_GPIO_SW_POWOFF);
#ifndef DEMO_CFG_SW_PU_ENABLE
	// gpio21: raise edge detect off
	demo_drv_gp_reg->ren0 &= ~(1u << BCM2835_CFG_GPIO_SW_POWOFF);
#endif /* DEMO_CFG_SW_PU_ENABLE */

	disable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT));
	disable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF));
	free_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT), &demo_drv_devices);
	free_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF), &demo_drv_devices);

	sc1602bs_finalize();

	iounmap(demo_drv_pwm_reg);
	iounmap(demo_drv_gp_reg);
	iounmap(bcm2835_reg_cm_base);

	i2c_unregister_device(demo_drv_devices.i2c);
	i2c_del_driver(&demo_dev_i2c_drv);

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
static long	demo_dev_driver_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	long result = 0;
#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(0x%lx, %d, 0x%lx): l(%d)\n",
			__FILE__, __PRETTY_FUNCTION__, (unsigned long)filp,
					cmd, arg, __LINE__);
#endif	// LOCAL_DEBUG_LOG

	switch (cmd) {
	case DEMO_DEV_IOCTL_SET_SW_IRQ:
	{
		if (arg) {
			enable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT));
			enable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF));
		} else {
			disable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT));
			disable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF));
		}
		break;
	}
	case DEMO_DEV_IOCTL_GET_BME280:
	{
		demo_drv_ioctl_bme280_info_t info;
		result = bme280_read_info(
				((demo_dev_devices_t *)(filp->private_data))->i2c, &info);
		if (result < 0) {
			break;
		}
		result = copy_to_user((void *)arg, &info, sizeof(info));
		break;
	}

	case DEMO_DEV_IOCTL_SET_ILM_00:
	{
		bcm2835_set_pwm(0, arg);
		break;
	}
	case DEMO_DEV_IOCTL_SET_ILM_01:
	{
		bcm2835_set_pwm(1, arg);
		break;
	}

	case DEMO_DEV_IOCTL_GET_ILM_00:
	{
		uint32_t val = bcm2835_get_width_pwm(0);
		result = copy_to_user((void *)arg, &val, sizeof(val));
		break;
	}
	case DEMO_DEV_IOCTL_GET_ILM_01:
	{
		uint32_t val = bcm2835_get_width_pwm(1);
		result = copy_to_user((void *)arg, &val, sizeof(val));
		break;
	}

	default:
		break;
	}

	return(result);
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
	demo_drv_ioctl_bme280_info_t data;
	uint8_t bytes[128];
	int len;

#ifdef	LOCAL_DEBUG_LOG
	printk(KERN_DEBUG "%s::%s(0x%lx): l(%d): buf=%p. ppos=%x\n",
			__FILE__, __PRETTY_FUNCTION__, (unsigned long)filp, __LINE__,
			buf, (int)*ppos);
#endif	// LOCAL_DEBUG_LOG

	if (*ppos > 0) {
		return 0;
	}

	if (((demo_dev_devices_t *)(filp->private_data))->assert_reboot) {
//		enable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_REBOOT));
		((demo_dev_devices_t *)(filp->private_data))->assert_reboot = 0u;
		*bytes = DEMO_DEV_READ_REBOOT;
		len = 1;
		len -= copy_to_user(buf, bytes, len);
		*ppos = len;
		return len;
	}

	if (((demo_dev_devices_t *)(filp->private_data))->assert_powoff) {
//		enable_irq(gpio_to_irq(BCM2835_CFG_GPIO_SW_POWOFF));
		((demo_dev_devices_t *)(filp->private_data))->assert_powoff = 0u;
		*bytes = DEMO_DEV_READ_POWOFF;
		len = 1;
		len -= copy_to_user(buf, bytes, len);
		*ppos = len;
		return len;
	}

	memset(&data, 0, sizeof(data));
	if (bme280_read_info(((demo_dev_devices_t *)(filp->private_data))->i2c,
			&data) < 0) {
		return 0;
	}

	sprintf((char *)bytes, "T%d,P%d,\nH%d",
			data.temp, data.press, data.hum);
	sc1602bs_indicate_string((char *)bytes);

	sprintf((char *)bytes, "T=%d, P=%d, H=%d\n",
			data.temp, data.press, data.hum);

	len = strlen((char *)bytes);
	len -= copy_to_user(buf, bytes, len);
	*ppos = len;
	return len;
}

/* -------------------------------------------------------------------------- */
static unsigned int	demo_dev_driver_poll(struct file *filp, poll_table *wait)
{
	unsigned retmask = 0;

	poll_wait(filp, &demo_drv_read_q, wait);
	if (((demo_dev_devices_t *)(filp->private_data))->assert_reboot) {
		retmask |= (POLLIN | POLLRDNORM);
	}
	if (((demo_dev_devices_t *)(filp->private_data))->assert_powoff) {
		retmask |= (POLLIN | POLLRDNORM);
	}
	return retmask;
}

/* -------------------------------------------------------------------------- */
MODULE_AUTHOR("hoge");
MODULE_DESCRIPTION("device driver for demo.");
MODULE_LICENSE("GPL v2");

module_init(demo_dev_init_driver);
module_exit(demo_dev_cleanup_driver);



