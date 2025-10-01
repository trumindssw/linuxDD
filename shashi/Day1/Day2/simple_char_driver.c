// simple_char_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>        // file_operations structure
#include <linux/uaccess.h>   // copy_to_user, copy_from_user

#define DEVICE_NAME "simple_char"
#define BUFFER_SIZE 1024
#define MY_MAJOR 255   // Static major number (choose between 240–255 for experiments)

static char device_buffer[BUFFER_SIZE];
static int open_count = 0;

// Open function
static int my_open(struct inode *inode, struct file *file)
{
    open_count++;
    printk(KERN_INFO "SimpleChar: Device opened %d times\n", open_count);
    return 0;
}

// Release function
static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "SimpleChar: Device closed\n");
    return 0;
}

// Read function
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int bytes_to_read;

    // Limit read size
    bytes_to_read = min((int)count, BUFFER_SIZE - (int)(*ppos));

    if (bytes_to_read <= 0)
        return 0;

    if (copy_to_user(buf, device_buffer + *ppos, bytes_to_read))
        return -EFAULT;

    *ppos += bytes_to_read;
    printk(KERN_INFO "SimpleChar: Read %d bytes\n", bytes_to_read);
    return bytes_to_read;
}

// Write function
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int bytes_to_write;

    // Limit write size
    bytes_to_write = min((int)count, BUFFER_SIZE - (int)(*ppos));

    if (bytes_to_write <= 0)
        return -ENOMEM;

    if (copy_from_user(device_buffer + *ppos, buf, bytes_to_write))
        return -EFAULT;

    *ppos += bytes_to_write;
    printk(KERN_INFO "SimpleChar: Wrote %d bytes\n", bytes_to_write);
    return bytes_to_write;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

// Init function
static int __init my_init(void)
{
    int ret;

    ret = register_chrdev(MY_MAJOR, DEVICE_NAME, &fops);
    if (ret < 0) {
        printk(KERN_ALERT "SimpleChar: Failed to register device\n");
        return ret;
    }

    printk(KERN_INFO "SimpleChar: Module loaded with major=%d\n", MY_MAJOR);
    return 0;
}

// Exit function
static void __exit my_exit(void)
{
    unregister_chrdev(MY_MAJOR, DEVICE_NAME);
    printk(KERN_INFO "SimpleChar: Module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Character Driver with static major number");

