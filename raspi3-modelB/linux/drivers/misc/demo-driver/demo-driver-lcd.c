
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
// #include <linux/thread_info.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/export.h>

#define	SC1602BS_DATA_BIT4

#define	SC1602BS_REQ_INITIALIZE					(0)
#define	SC1602BS_REQ_INDICATE					(1)
#define	SC1602BS_REQ_CLEAR						(2)

// remark: if reading from device then db[7:0] needs pull-up.
#define	SC1602BS_RW_READ						(0x01u)
#define	SC1602BS_RW_WRITE						(0x00u)

#define	SC1602BS_RS_RAM							(0x01u)
#define	SC1602BS_RS_REG							(0x00u)

#define	SC1602BS_CTRL_CLEAR_DISPLAY				(0x01u)
#define	SC1602BS_CTRL_RETURN_HOME				(0x03u)
#define	SC1602BS_CTRL_ENTRY_MODE_SET(i_d, s)	(0x04u | ((i_d) << 1) | (s))
#define	SC1602BS_CTRL_DISPLAY_ON(d, c, b)		\
		(0x08u | ((d) << 2) | ((c) << 1) | (b))
#define	SC1602BS_CTRL_FUNCTION_SET(dl, n, f)	\
		(0x20u | ((dl) << 4) | ((n) << 3) | ((f) << 2))
#define	SC1602BS_CTRL_SET_CGRAM_ADDRESS(ac)		(0x40u | (ac))
#define	SC1602BS_CTRL_SET_DDRAM_ADDRESS(ac)		(0x80u | (ac))

typedef struct sc1602bs_request_t {
	struct work_struct	wk_st;
	uint32_t			req_no;
	uint8_t				data[256];
} sc1602bs_request_t;

const uint32_t bcm2835_v_test_gpio = 26u;

const uint32_t sc1602bs_rs_gpio = 5u;
const uint32_t sc1602bs_rw_gpio = 6u;
const uint32_t sc1602bs_es_gpio = 18u;
const uint32_t sc1602bs_db_gpio[] = {
#if !defined(SC1602BS_DATA_BIT4)
	19u, 23u, 24u, 25u,
#endif /* SC1602BS_DATA_BIT4 */
	14u, 15u, 16u, 17u
};

static struct workqueue_struct	*sc1602bs_wq_st = NULL;

static void sc1602bs_main(struct work_struct *wk_st);
static void sc1602bs_post(sc1602bs_request_t *req);
static void sc1602bs_ctrl(uint8_t rs, uint8_t rw, uint8_t db);
static void sc1602bs_port_initialize(void);
static void sc1602bs_device_initialize(void);
static void sc1602bs_send_to_device(uint8_t *data);

/**
 */
void sc1602bs_initialize(void)
{
	sc1602bs_request_t req;

	if (sc1602bs_wq_st) {
		return;
	}

	sc1602bs_wq_st = create_singlethread_workqueue("demo_sc1602bs_workqueue");
	if (!sc1602bs_wq_st) {
		return;
	}

	req.req_no = SC1602BS_REQ_INITIALIZE;
	sc1602bs_post(&req);
}
// EXPORT_SYMBOL(sc1602bs_initialize);

/**
 */
void sc1602bs_finalize(void)
{
	if (!sc1602bs_wq_st) {
		return;
	}
	destroy_workqueue(sc1602bs_wq_st);
}
// EXPORT_SYMBOL(sc1602bs_finalize);

/**
 */
void sc1602bs_indicate_string(char *str)
{
	sc1602bs_request_t req;

	if (!sc1602bs_wq_st) {
		return;
	}

	req.req_no = SC1602BS_REQ_INDICATE;
	strcpy((char *)(req.data), str);
	sc1602bs_post(&req);
}
// EXPORT_SYMBOL(sc1602bs_indicate_string);

/**
 */
void sc1602bs_clear_display(void)
{
	sc1602bs_request_t req;

	if (!sc1602bs_wq_st) {
		return;
	}

	req.req_no = SC1602BS_REQ_CLEAR;
	sc1602bs_post(&req);
}
// EXPORT_SYMBOL(sc1602bs_clear_display);

/**
 */
static void sc1602bs_main(struct work_struct *wk_st)
{
	sc1602bs_request_t req;
	sc1602bs_request_t *preq;

	preq = container_of(wk_st, sc1602bs_request_t, wk_st);
	memcpy(&req, preq, sizeof(sc1602bs_request_t));
	kfree(preq);

	printk(KERN_DEBUG "%s::%s(): l(%d): req=%d\n",
			__FILE__, __PRETTY_FUNCTION__, __LINE__, req.req_no);

	switch (req.req_no) {
	case SC1602BS_REQ_INITIALIZE:
		sc1602bs_port_initialize();
		sc1602bs_device_initialize();
		break;
	case SC1602BS_REQ_INDICATE:
		sc1602bs_send_to_device(req.data);
		break;
	case SC1602BS_REQ_CLEAR:
		sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
				SC1602BS_CTRL_CLEAR_DISPLAY);
		// 1.52 msec wait (1 msec wait in sc1602bs_ctrl)
		msleep(1);
		break;
	default:
		break;
	}
}

/**
 */
static void sc1602bs_post(sc1602bs_request_t *req)
{
	sc1602bs_request_t *rq;
	rq = (sc1602bs_request_t *)kmalloc(sizeof(sc1602bs_request_t), GFP_KERNEL);
	memcpy(rq, req, sizeof(sc1602bs_request_t));
	INIT_WORK(&rq->wk_st, sc1602bs_main);
	queue_work(sc1602bs_wq_st, &rq->wk_st);
}

/**
 */
static void sc1602bs_ctrl(uint8_t rs, uint8_t rw, uint8_t db)
{
	uint32_t i;

	gpio_set_value(sc1602bs_rw_gpio, rw? 1u : 0u);
	gpio_set_value(sc1602bs_rs_gpio, rs? 1u : 0u);

	if ((sizeof(sc1602bs_db_gpio) / sizeof(sc1602bs_db_gpio[0])) == 8u) {
		for (i = 0; i < sizeof(sc1602bs_db_gpio) / sizeof(sc1602bs_db_gpio[0]); i++) {
			gpio_set_value(sc1602bs_db_gpio[i], (db & (1u << i))? 1u : 0u);
		}
		gpio_set_value(sc1602bs_es_gpio, 1u);
		// 140 nsec wait (tpw)
//		msleep(1);
		gpio_set_value(sc1602bs_es_gpio, 0u);
	} else {
		for (i = 0; i < sizeof(sc1602bs_db_gpio) / sizeof(sc1602bs_db_gpio[0]); i++) {
			gpio_set_value(sc1602bs_db_gpio[i], ((db >> 4) & (1u << i))? 1u : 0u);
		}
		gpio_set_value(sc1602bs_es_gpio, 1u);
		// 140 nsec wait (tpw)
//		msleep(1);
		gpio_set_value(sc1602bs_es_gpio, 0u);
		for (i = 0; i < sizeof(sc1602bs_db_gpio) / sizeof(sc1602bs_db_gpio[0]); i++) {
			gpio_set_value(sc1602bs_db_gpio[i], ((db & 0x0fu) & (1u << i))? 1u : 0u);
		}
		gpio_set_value(sc1602bs_es_gpio, 1u);
		// 140 nsec wait (tpw)
//		msleep(1);
		gpio_set_value(sc1602bs_es_gpio, 0u);
	}
	// (1200 - 140) nsec wait (tc)
	msleep(1);
}

/**
 */
static void sc1602bs_port_initialize(void)
{
	uint32_t i;

	gpio_direction_output(bcm2835_v_test_gpio, 1u);

	gpio_direction_output(sc1602bs_rw_gpio, 0u);
	gpio_direction_output(sc1602bs_rs_gpio, 0u);
	gpio_direction_output(sc1602bs_es_gpio, 0u);
	for (i = 0; i < (sizeof(sc1602bs_db_gpio) / sizeof(sc1602bs_db_gpio[0])); i++) {
		gpio_direction_output(sc1602bs_db_gpio[i], 0u);
	}
}

/**
 */
static void sc1602bs_device_initialize(void)
{
	// 40 msec wait
	msleep(40);
#if !defined(SC1602BS_DATA_BIT4)
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_FUNCTION_SET(1, 1, 0));
	// 37 usec wait (1 msec wait in sc1602bs_ctrl)
//	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
//			SC1602BS_CTRL_FUNCTION_SET(1, 1, 0));
#else /* defined(SC1602BS_DATA_BIT4) */
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_FUNCTION_SET(1, 1, 0));
	msleep(5);
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_FUNCTION_SET(0, 1, 0));
#endif /* SC1602BS_DATA_BIT4 */
	// 37 usec wait (1 msec wait in sc1602bs_ctrl)
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_DISPLAY_ON(0, 0, 0));
	// 37 usec wait (1 msec wait in sc1602bs_ctrl)
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_CLEAR_DISPLAY);
	// 1.52 msec wait (1 msec wait in sc1602bs_ctrl)
	msleep(1);
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_RETURN_HOME);
	// 1.52 msec wait (1 msec wait in sc1602bs_ctrl)
	msleep(1);
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_ENTRY_MODE_SET(1, 0));
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_DISPLAY_ON(1, 0, 0));
}

/**
 */
static void sc1602bs_send_to_device(uint8_t *data)
{
	uint32_t i;
#if 0
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_CLEAR_DISPLAY);
	// 1.52 msec wait (1 msec wait in sc1602bs_ctrl)
	msleep(1);
#endif
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_RETURN_HOME);
	// 1.52 msec wait (1 msec wait in sc1602bs_ctrl)
	msleep(1);

	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_SET_CGRAM_ADDRESS(0));
	sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
			SC1602BS_CTRL_SET_DDRAM_ADDRESS(0));
	for (i = 0; data[i]; i++) {
		if ((data[i] == (uint8_t)'\n') || (data[i] == (uint8_t)'\r')) {
			sc1602bs_ctrl(SC1602BS_RS_REG, SC1602BS_RW_WRITE,
					SC1602BS_CTRL_SET_DDRAM_ADDRESS(0x40));
			continue;
		}
		sc1602bs_ctrl(SC1602BS_RS_RAM, SC1602BS_RW_WRITE, data[i]);
	}
}
