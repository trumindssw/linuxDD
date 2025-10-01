#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "MyCharDevice"
#define NUM_DEVICES 4        // We’ll create 4 devices: 0–3
#define BUF_SIZE 1024        // Each device has its own buffer

static int major;
static struct cdev my_cdevs[NUM_DEVICES];
static char *device_buffers[NUM_DEVICES];   // Separate memory for each device
static dev_t dev_number;

/* Open function */
static int my_open(struct inode *inode, struct file *file)
{
    int minor = iminor(inode);   // Get minor number
    printk(KERN_INFO "MyCharDevice%d: opened\n", minor);
    file->private_data = device_buffers[minor]; // Link buffer to file
    return 0;
}

/* Read function */
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char *device_buffer = file->private_data;
    int minor = iminor(file_inode(file));

    if (*ppos >= BUF_SIZE) return 0;   // EOF
    if (*ppos + count > BUF_SIZE) count = BUF_SIZE - *ppos;

    if (copy_to_user(buf, device_buffer + *ppos, count))
        return -EFAULT;

    *ppos += count;
    printk(KERN_INFO "MyCharDevice%d: read %zu bytes\n", minor, count);
    return count;
}

/* Write function */
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char *device_buffer = file->private_data;
    int minor = iminor(file_inode(file));

    if (*ppos >= BUF_SIZE) return -ENOSPC;
    if (*ppos + count > BUF_SIZE) count = BUF_SIZE - *ppos;

    if (copy_from_user(device_buffer + *ppos, buf, count))
        return -EFAULT;

    *ppos += count;
    printk(KERN_INFO "MyCharDevice%d: wrote %zu bytes\n", minor, count);
    return count;
}

/* File operations structure */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
};

/* Init function */
static int __init my_init(void)
{
    int i, ret;

    /* Allocate device numbers */
    ret = alloc_chrdev_region(&dev_number, 0, NUM_DEVICES, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to allocate major number\n");
        return ret;
    }

    major = MAJOR(dev_number);
    printk(KERN_INFO "Registered MyCharDevice with major %d\n", major);

    /* Initialize each device */
    for (i = 0; i < NUM_DEVICES; i++) {
        cdev_init(&my_cdevs[i], &fops);
        my_cdevs[i].owner = THIS_MODULE;

        ret = cdev_add(&my_cdevs[i], MKDEV(major, i), 1);
        if (ret < 0) {
            printk(KERN_ALERT "Failed to add cdev %d\n", i);
            return ret;
        }

        /* Allocate buffer */
        device_buffers[i] = kmalloc(BUF_SIZE, GFP_KERNEL);
        if (!device_buffers[i]) {
            printk(KERN_ALERT "Failed to allocate buffer for device %d\n", i);
            return -ENOMEM;
        }
    }

    return 0;
}

/* Exit function */
static void __exit my_exit(void)
{
    int i;
    for (i = 0; i < NUM_DEVICES; i++) {
        cdev_del(&my_cdevs[i]);
        kfree(device_buffers[i]);
    }
    unregister_chrdev_region(dev_number, NUM_DEVICES);
    printk(KERN_INFO "MyCharDevice unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Generic Multi-Device Char Driver");

