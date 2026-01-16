obj-m := tss_module.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(CC) -O2 -Wall -o tss_app tss_app.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) tss_app
