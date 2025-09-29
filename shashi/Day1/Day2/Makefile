obj-m += simple_char_driver.o dyn_char_driver.o char_ab_driver.o mulit_device_driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

