#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // user-kernel memory access
#include <linux/cdev.h> // char device registration
#include <linux/device.h>
#include <linux/device/class.h> // for class usage

#define DEVICE_NAME "ioctl_debug"
#define CLASS_NAME "ioctl_class"

#define IOCTL_GET_VALUE _IOR('i', 1, int *) // read from kernel space
#define IOCTL_SET_VALUE _IOW('i', 2, int *) // write to kernel space

static int major;
static struct cdev my_cdev;
static struct class *my_class;

static int kernel_val = 42;

static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int user_val;

    switch (cmd) {
        case IOCTL_GET_VALUE:
            if (copy_to_user((int __user *)arg, &kernel_val, sizeof(int)))
                return -EFAULT;
            printk(KERN_INFO "IOCTL: sent value %d to user\n", kernel_val);
            break;

        case IOCTL_SET_VALUE:
            if (copy_from_user(&user_val, (int __user *)arg, sizeof(int)))
                return -EFAULT;
            kernel_val = user_val;
            printk(KERN_INFO "IOCTL: received value %d from user\n", user_val);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

static int __init ioctl_init(void) {
    dev_t dev;
    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    my_class = class_create(CLASS_NAME);
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);

    printk(KERN_INFO "IOCTL device created: /dev/%s (major=%d)\n", DEVICE_NAME, major);
    return 0;
}

static void __exit ioctl_exit(void) {
    device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    printk(KERN_INFO "IOCTL module removed\n");
}

module_init(ioctl_init);
module_exit(ioctl_exit);
MODULE_LICENSE("GPL");
