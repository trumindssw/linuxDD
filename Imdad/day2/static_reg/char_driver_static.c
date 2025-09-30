#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // copy_to_user, copy_from_user

#define DEVICE_NAME "simplechardev"
#define BUF_SIZE 100
#define MAJOR 280
// hardcoded the major number to 280
static char device_buffer[BUF_SIZE] = {0};
static size_t data_size = 0;

// File operations
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "simplechardev: Device opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "simplechardev: Device closed\n");
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    ssize_t available, to_copy;

    // nothing left to read
    if (*offset >= data_size)
        return 0; // EOF

    // how many bytes are left
    available = data_size - *offset;

    // copy only what user requested, or what's available
    to_copy = (len < available) ? len : available;

    if (copy_to_user(buf, device_buffer + *offset, to_copy) != 0)
        return -EFAULT;

    *offset += to_copy;

    printk(KERN_INFO "simplechardev: Read %zu bytes, new offset=%lld\n", to_copy, *offset);
    return to_copy;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    ssize_t space, to_copy;

    // how much space left
    space = BUF_SIZE - *offset;
    if (space <= 0)
        return -ENOSPC;

    // copy only what fits
    to_copy = (len < space) ? len : space;

    if (copy_from_user(device_buffer + *offset, buf, to_copy) != 0)
        return -EFAULT;

    *offset += to_copy;
    if (*offset > data_size)
        data_size = *offset;

    printk(KERN_INFO "simplechardev: Written %zu bytes, new offset=%lld, data_size=%zu\n",
           to_copy, *offset, data_size);
    return to_copy;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

// Module init
static int __init simple_char_init(void)
{
    int result;

    result = register_chrdev(MAJOR, DEVICE_NAME, &fops); // static major
    if (result < 0) {
        printk(KERN_ALERT "simplechardev: Failed to register device with major\n");
        return result;
    }

    printk(KERN_INFO "simplechardev: Registered with static major\n");
    return 0;
}

// Module exit
static void __exit simple_char_exit(void)
{
    unregister_chrdev(MAJOR, DEVICE_NAME);
    printk(KERN_INFO "simplechardev: Unregistered device\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION("Static registration");
MODULE_VERSION("1.0");

module_init(simple_char_init);
module_exit(simple_char_exit);
