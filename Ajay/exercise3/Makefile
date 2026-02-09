ifneq ($(KERNELRELEASE),)
obj-m += mydrv.o
else
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all: mydrv.ko user_app

mydrv.ko: mydrv.c
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user_app: user_app.c
	$(CC) -O2 -Wall -Wextra -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) user_app

.PHONY: all clean
endif
