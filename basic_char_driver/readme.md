## Simple Linux character device driver implemented as a kernel module 

It demonstrates:

Dynamic allocation of major and minor device numbers.

Device file creation in /dev/ using device_create.

Memory allocation using kmalloc with GFP_KERNEL.

Reading and writing from/to user space using copy_to_user and copy_from_user.
