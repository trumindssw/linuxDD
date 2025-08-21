obj-m += busy_wait_delay.o msleep_delay.o kernel_timer.o \
         hrtimer_example.o tasklet_example.o workqueue_example.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

