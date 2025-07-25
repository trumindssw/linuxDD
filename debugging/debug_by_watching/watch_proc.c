#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>

#define DEVICE_NAME "watch_drv"
#define CLASS_NAME "watch_class"
#define PROC_NAME   "watch_info"

static int value = 100;

static dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%d\n", value);

    if (*ppos > 0) return 0;
    if (copy_to_user(buf, buffer, len)) return -EFAULT;

    *ppos = len;
    return len;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];

    if (count > sizeof(buffer) - 1) return -EINVAL;
    if (copy_from_user(buffer, buf, count)) return -EFAULT;

    buffer[count] = '\0';
    sscanf(buffer, "%d", &value);

    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
};

static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "Proc value: %d\n", value);

    if (*ppos > 0) return 0;
    if (copy_to_user(buf, buffer, len)) return -EFAULT;

    *ppos = len;
    return len;
}

static ssize_t proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];

    if (count > sizeof(buffer) - 1) return -EINVAL;
    if (copy_from_user(buffer, buf, count)) return -EFAULT;

    buffer[count] = '\0';
    sscanf(buffer, "%d", &value);

    return count;
}

static struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init watch_init(void) {
    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    my_class = class_create(CLASS_NAME);
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);

    proc_create(PROC_NAME, 0666, NULL, &proc_fops);

    printk(KERN_INFO "Watch driver loaded: /dev/%s, /proc/%s\n", DEVICE_NAME, PROC_NAME);
    return 0;
}

static void __exit watch_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    device_destroy(my_class, dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Watch driver unloaded\n");
}

module_init(watch_init);
module_exit(watch_exit);
MODULE_LICENSE("GPL");
