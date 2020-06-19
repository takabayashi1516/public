#
# Makefile for the control device for demo.(bme280/pwm(bcm2837)/sc1602bs)
#

CFILES = demo-driver-sens.c demo-driver-lcd.c

obj-$(CONFIG_DEMO_DEV) += demo-driver.o
demo-driver-objs := $(CFILES:.c=.o)

ifeq ($(CONFIG_DEMO_DEV_DEBUG), y)
EXTRA_CFLAGS += -DDEBUG
endif
