// writing.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/interrupt.h>

#define DEVICE_NAME "mydev"
#define CLASS_NAME "myclass"
#define IOCTL_WRITE_DATETIME _IO('a', 1)

static int major;
static struct class *my_class;
static struct cdev my_cdev;
static struct tasklet_struct my_tasklet;

// --- Tasklet function ---
static void my_tasklet_func(unsigned long data)
{
    struct file *filp;
    char buf[128];
    struct timespec64 ts;
    struct tm tm;
    int len;

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);

    len = snprintf(buf, sizeof(buf),
                   "%04ld-%02d-%02d %02d:%02d:%02d\n",
                   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                   tm.tm_hour, tm.tm_min, tm.tm_sec);

    filp = filp_open("datetime.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (IS_ERR(filp)) {
        pr_err("Failed to open file\n");
        return;
    }

    kernel_write(filp, buf, len, &filp->f_pos);

    filp_close(filp, NULL);
    pr_info("Tasklet wrote datetime: %s", buf);
}

// --- IOCTL handler ---
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case IOCTL_WRITE_DATETIME:
            tasklet_schedule(&my_tasklet);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_ioctl,
};

static int __init my_init(void)
{
    dev_t dev;
    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    my_class = class_create(CLASS_NAME);
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);

    tasklet_init(&my_tasklet, my_tasklet_func, 0);

    pr_info("Module loaded, major=%d\n", major);
    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(&my_tasklet);
    device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);

    pr_info("Module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
