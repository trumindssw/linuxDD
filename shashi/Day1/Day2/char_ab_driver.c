// char_ab_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>        // for register_chrdev
#include <linux/uaccess.h>   // for copy_to_user, copy_from_user

#define DEVICE_NAME "char_ab"
#define BUFFER_SIZE 10

static int major;   
static char kernel_buffer[BUFFER_SIZE];
static int data_size = 0;

// Open
static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "CharAB: Device opened\n");
    return 0;
}

// Release
static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "CharAB: Device closed\n");
    return 0;
}

// Write
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    char ch;

    if (count < 1) return -EINVAL; // need at least 1 char

    if (copy_from_user(&ch, buf, 1))
        return -EFAULT;

    printk(KERN_INFO "CharAB: User wrote '%c'\n", ch);

    if (ch == 'a')
        kernel_buffer[0] = 'b';
    else if (ch == 'A')
        kernel_buffer[0] = 'B';
    else
        kernel_buffer[0] = '?';  // unknown input

    data_size = 1;
    return 1; // wrote 1 byte
}

// Read
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    if (*ppos > 0 || data_size == 0)
        return 0;  // nothing left to read

    if (copy_to_user(buf, kernel_buffer, data_size))
        return -EFAULT;

    printk(KERN_INFO "CharAB: Sent '%c' to user\n", kernel_buffer[0]);
    *ppos += data_size;
    return data_size;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

// Init
static int __init ab_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops); // dynamic major
    if (major < 0) {
        printk(KERN_ALERT "CharAB: Failed to register\n");
        return major;
    }
    printk(KERN_INFO "CharAB: Loaded with major=%d\n", major);
    return 0;
}

// Exit
static void __exit ab_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "CharAB: Unloaded\n");
}

module_init(ab_init);
module_exit(ab_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Char Driver: a/A -> b/B response");

