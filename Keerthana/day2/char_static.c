#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cdd01"
#define MAJOR_NUM 117        // Static major number
#define MEM_SIZE 1024        // Memory size for device

MODULE_LICENSE("GPL");

// Device memory
static char device_buffer[MEM_SIZE];
static int device_open_count = 0;

// Open function
static int my_open(struct inode *inode, struct file *file)
{
    device_open_count++;
    printk(KERN_INFO "Device opened %d times\n", device_open_count);
    return 0;
}

// Release function
static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// Read function
static ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    if (*offset >= MEM_SIZE) return 0;  // EOF
    if (len > MEM_SIZE - *offset) len = MEM_SIZE - *offset;

    if (copy_to_user(buf, device_buffer + *offset, len))
        return -EFAULT;

    *offset += len;
    return len;
}

// Write function
static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    if (*offset >= MEM_SIZE) return -ENOSPC;
    if (len > MEM_SIZE - *offset) len = MEM_SIZE - *offset;

    if (copy_from_user(device_buffer + *offset, buf, len))
        return -EFAULT;

    *offset += len;
    return len;
}

// File operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

// Init function
static int __init mychardev_init(void)
{
    int ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return ret;
    }
    printk(KERN_INFO "Registered device %s with major %d\n", DEVICE_NAME, MAJOR_NUM);
    return 0;
}

// Exit function
static void __exit mychardev_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "Device unregistered\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);
