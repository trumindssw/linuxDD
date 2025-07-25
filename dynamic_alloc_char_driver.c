#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "dyn_chrdev"
#define BUFFER_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("keerthana");
MODULE_DESCRIPTION("Character Driver with Dynamic Allocation");

static dev_t dev_num;
static struct cdev dyn_cdev;
static char device_buffer[BUFFER_SIZE];

//File operation to open a device
static int dyn_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "dyn_chrdev: Device opened\n");
    return 0;
}

//File operation to close device
static int dyn_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "dyn_chrdev: Device closed\n");
    return 0;
}

//File operation to read data from kernel 
static ssize_t dyn_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    if (*offset >= BUFFER_SIZE)
        return 0;
    if (*offset + len > BUFFER_SIZE)
        len = BUFFER_SIZE - *offset;

    if (copy_to_user(buf, device_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

//File operation to write data to kernel 
static ssize_t dyn_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    if (*offset >= BUFFER_SIZE)
        return -ENOMEM;
    if (*offset + len > BUFFER_SIZE)
        len = BUFFER_SIZE - *offset;

    if (copy_from_user(device_buffer + *offset, buf, len))
        return -EFAULT;

    *offset += len;
    return len;
}

//File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dyn_open,
    .release = dyn_release,
    .read = dyn_read,
    .write = dyn_write,
};

// ---------- Module Init and Exit ----------

static int __init dyn_chrdev_init(void) {
    int result;

    // Allocate device numbers dynamically
    result = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (result < 0) {
        printk(KERN_ALERT "dyn_chrdev: Failed to allocate major number\n");
        return result;
    }

    printk(KERN_INFO "dyn_chrdev: Registered with major %d, minor %d\n", MAJOR(dev_num), MINOR(dev_num));

    // Initialize and add the cdev structure
    cdev_init(&dyn_cdev, &fops);
    dyn_cdev.owner = THIS_MODULE;

    result = cdev_add(&dyn_cdev, dev_num, 1);
    if (result < 0) {
        unregister_chrdev_region(dev_num, 1);
        printk(KERN_ALERT "dyn_chrdev: Failed to add cdev\n");
        return result;
    }

    return 0;
}

static void __exit dyn_chrdev_exit(void) {
    cdev_del(&dyn_cdev);                      // Remove cdev
    unregister_chrdev_region(dev_num, 1);     // Free device numbers
    printk(KERN_INFO "dyn_chrdev: Unregistered device\n");
}

module_init(dyn_chrdev_init);
module_exit(dyn_chrdev_exit);
