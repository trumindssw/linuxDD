#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/device.h>

#define DEVICE_NAME "simplechar"
#define BUF_SIZE 1024

static char kernel_buffer[BUF_SIZE];
static int major;
static struct cdev simple_cdev;
static struct class *simple_class;

static ssize_t simple_read(struct file *filep, char __user *buf, size_t len, loff_t *offset) {
    size_t to_copy = min(len, (size_t)BUF_SIZE);
    if (copy_to_user(buf, kernel_buffer, to_copy)) {
        return -EFAULT;
    }
    pr_info("simplechar: Data sent to user\n");
    return to_copy;
}

static ssize_t simple_write(struct file *filep, const char __user *buf, size_t len, loff_t *offset) {
    size_t to_copy = min(len, (size_t)BUF_SIZE);
    if (copy_from_user(kernel_buffer, buf, to_copy)) {
        return -EFAULT;
    }
    pr_info("simplechar: Data received from user\n");
    return to_copy;
}

static int my_open(struct inode *inode, struct file *filep) {
    pr_info("simplechar: Device opened\n");
    return 0;
}

static int simple_release(struct inode *inode, struct file *filep) {
    pr_info("simplechar: Device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = simple_read,
    .write = simple_write,
    .open = my_open,
    .release = simple_release
};

static int __init simple_init(void) {
    dev_t dev;
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    major = MAJOR(dev);
    cdev_init(&simple_cdev, &fops);
    ret = cdev_add(&simple_cdev, dev, 1);
    if (ret < 0)
        return ret;

    simple_class = class_create(DEVICE_NAME);
    device_create(simple_class, NULL, dev, NULL, DEVICE_NAME);

    pr_info("simplechar: Module loaded, Major %d\n", major);
    return 0;
}

static void __exit simple_exit(void) {
    device_destroy(simple_class, MKDEV(major, 0));
    class_destroy(simple_class);
    cdev_del(&simple_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
    pr_info("simplechar: Module unloaded\n");
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajaypal Singh");
MODULE_DESCRIPTION("Simple char device using copy_from_user and copy_to_user");
