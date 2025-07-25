#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>//file operations
#include <linux/uaccess.h>
#include <linux/cdev.h>//character device
#include <linux/slab.h> // for kmalloc and kfree

#define DEVICE_NAME "debug_char"
#define BUF_SIZE 1024

static char *kernel_buffer; // now a pointer instead of fixed array
static int major;
static struct cdev my_cdev;
static struct class *cls;

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO " Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    int bytes = min(len, (size_t)BUF_SIZE);

    if (copy_to_user(buf, kernel_buffer, bytes))
    {
        printk(KERN_ERR "copy_to_user failed\n");
        return -EFAULT;
    }

    printk(KERN_INFO " Sent to user: %s\n", kernel_buffer);
    return bytes;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    int bytes = min(len, (size_t)BUF_SIZE);

    if (copy_from_user(kernel_buffer, buf, bytes))
    {
        printk(KERN_ERR " copy_from_user failed\n");
        return -EFAULT;
    }

    printk(KERN_INFO " Received from user: %s\n", kernel_buffer);
    return bytes;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

static int __init my_init(void)
{
    dev_t dev;

    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0)
    {
        printk(KERN_ALERT " Failed to allocate major number\n");
        return -1;
    }

    major = MAJOR(dev);// to extract major number
    printk(KERN_INFO " Major number: %d\n", major);

    //  Allocate memory dynamically
    kernel_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!kernel_buffer)
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT " Failed to allocate memory\n");
        return -ENOMEM;
    }

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    cls = class_create("debug_class");// to create a class
    device_create(cls, NULL, dev, NULL, DEVICE_NAME);//to create a device

    printk(KERN_INFO " Module loaded successfully\n");
    return 0;
}

// exit
static void __exit my_exit(void)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);

    //  Free dynamically allocated memory
    kfree(kernel_buffer);

    printk(KERN_INFO "Module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prasanthi");
MODULE_DESCRIPTION("Char device with dynamic memory allocation");

