#
# Makefile for the control device for demo.(spi)
#

CFILES = demo-peripheral-dev.c

obj-$(CONFIG_DEMO_PERIPHERAL_DEV) += demo-peripheral-driver.o
demo-peripheral-driver-objs := $(CFILES:.c=.o)

ifeq ($(CONFIG_DEMO_PERIPHERAL_DEV_DEBUG), y)
EXTRA_CFLAGS += -DDEBUG
endif
