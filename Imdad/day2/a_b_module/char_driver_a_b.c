#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
 
#define DEVICE_NAME "atob"
#define BUFFER_SIZE 1024
 
static dev_t dev_num;
static struct cdev my_cdev;
static char *device_buffer;
 
static int my_open(struct inode *inode, struct file *file)
{
    pr_info("mychardev: Device opened\n");
    return 0;
}
 

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("mychardev: Device closed\n");
    return 0;
}
 
static ssize_t my_write(struct file *file, const char __user *buf,
                        size_t len, loff_t *offset)
{
    char input;
 
    if (len < 1)
        return -EINVAL;
 
    if (copy_from_user(&input, buf, 1)) {
        return -EFAULT;
    }
 
    pr_info("mychardev: Received char = %c\n", input);
 
    // Convert 'a' → 'b', 'A' → 'B'
    if (input == 'a')
        device_buffer[0] = 'b';
    else if (input == 'A')
        device_buffer[0] = 'B';
    else
        device_buffer[0] = '?';   // invalid input
 
    return len;
}
 
static ssize_t my_read(struct file *file, char __user *buf,
                       size_t len, loff_t *offset)
{
    char input[2];

    if (*offset > 0)   // allow reading only once
        return 0;

    // Copy stored char from buffer
    input[0] = device_buffer[0];
    input[1] = '\n';  // add newline

    if (copy_to_user(buf, input, 2)) {
        return -EFAULT;
    }

    *offset += 2;  // mark as read completely
    pr_info("mychardev: Sent string \"%c\\n\"\n", device_buffer[0]);

    return 2;  // we sent 2 bytes
}

 
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
    .write   = my_write,
};
 
static int __init mychardev_init(void)
{
    int ret;
 
    // Allocate device numbers
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device number\n");
        return ret;
    }
 
    // Init cdev
    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        pr_err("Failed to add cdev\n");
        return ret;
    }
 
    // Allocate buffer
    device_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }
 
    pr_info("mychardev: Module loaded, Major=%d Minor=%d\n",
            MAJOR(dev_num), MINOR(dev_num));
    return 0;
}
 
static void __exit mychardev_exit(void)
{
    kfree(device_buffer);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("mychardev: Module unloaded\n");
}
 
module_init(mychardev_init);
module_exit(mychardev_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION("Simple char driver: a/A -> b/B");