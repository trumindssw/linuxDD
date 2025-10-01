obj-m += avg.o

KBUILD_EXTRA_SYMBOLS := /home/imdad/Desktop/day1/add/Module.symvers

KDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
