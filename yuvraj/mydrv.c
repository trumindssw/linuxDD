
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydrv"
#define BUF_SIZE 128

static int major;
static char kbuf[BUF_SIZE];
static int data_len; 

/*  file operations */

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mydrv: device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "mydrv: device closed\n");
    return 0;
}

static ssize_t my_write(struct file *file,
                        const char __user *ubuf,
                        size_t len,
                        loff_t *off)
{
    if (len > BUF_SIZE)
        len = BUF_SIZE;

    if (copy_from_user(kbuf, ubuf, len))
        return -EFAULT;

    data_len = len;
    printk(KERN_INFO "mydrv: received %zu bytes\n", len);
    return len;
}

static ssize_t my_read(struct file *file,
                       char __user *ubuf,
                       size_t len,
                       loff_t *off)
{
    if (*off > 0)
        return 0;

    if (copy_to_user(ubuf, kbuf, data_len))
        return -EFAULT;

    *off = data_len;
    printk(KERN_INFO "mydrv: sent %d bytes\n", data_len);
    return data_len;
}

/* fops table  */

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
    .write   = my_write,
};

/*  module init / exit  */

static int __init mydrv_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0)
        return major;

    printk(KERN_INFO "mydrv: registered with major %d\n", major);
    return 0;
}

static void __exit mydrv_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "mydrv: unregistered\n");
}

module_init(mydrv_init);
module_exit(mydrv_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuvraj");
MODULE_DESCRIPTION("Basic Char Driver");