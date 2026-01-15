obj-m += hrtimer.o
obj-m += workqueues.o
obj-m += spinlock.o
obj-m += mutex.o
obj-m += tasklets.o

KDIR = /lib/modules/$(shell uname -r)/build
PWD  = $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
