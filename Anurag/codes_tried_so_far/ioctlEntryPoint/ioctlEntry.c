
// just learning about ioctl
// ioctl + timers

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include "mydrv_ioctl.h"

#define DEVICE_NAME "my_drv"
#define CLASS_NAME  "my_class"

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static struct timer_list my_timer;
static int timer_val = 5;
static int timer_enabled = 0;

static void my_timer_callback(struct timer_list *t)
{
    pr_info("mydrv: Timer expired! (val=%d)\n", timer_val);

    if (timer_enabled) {
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_val * 1000));
    }
}

static int my_open(struct inode *inode, struct file *file)
{
    pr_info("mydrv: Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    pr_info("mydrv: Device released\n");
    return 0;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case MYDRV_ENABLE_TIMER:
        pr_info("mydrv: IOCTL ENABLE_TIMER\n");
        timer_enabled = 1;
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_val * 1000));
        break;

    case MYDRV_SET_TIMER_VAL:
        if (copy_from_user(&timer_val, (int __user *)arg, sizeof(timer_val)))
            return -EFAULT;
        pr_info("mydrv: IOCTL SET_TIMER_VAL = %d\n", timer_val);
        break;

    case MYDRV_STOP_TIMER:
        pr_info("mydrv: IOCTL STOP_TIMER\n");
        timer_enabled = 0;
        del_timer(&my_timer);
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = my_open,
    .release        = my_release,
    .read           = my_read,
    .write          = my_write,
    .unlocked_ioctl = my_ioctl,
};

static int __init mydrv_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        pr_err("mydrv: Failed to allocate device number\n");
        return -1;
    }

    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) {
        pr_err("mydrv: Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        pr_err("mydrv: Failed to create class\n");
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }

    my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        pr_err("mydrv: Failed to create device\n");
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }

    timer_setup(&my_timer, my_timer_callback, 0);

    pr_info("mydrv: Module loaded, device /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit mydrv_exit(void)
{
    del_timer_sync(&my_timer);
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("mydrv: Module unloaded\n");
}

module_init(mydrv_init);
module_exit(mydrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Char driver with IOCTL and Timer");
