#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/hrtimer.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/workqueue.h>   // <-- added for workqueues

#define DEVICE_NAME "my_drv_"
#define CLASS_NAME  "my_class1_"

#define MYDRV_ENABLE_TIMER  _IO('M', 1)
#define MYDRV_SET_TIMER_VAL _IOW('M', 2, unsigned int)
#define MYDRV_STOP_TIMER    _IO('M', 3)

#define BUF_SIZE 4096
static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static struct hrtimer hold_timer;
static spinlock_t my_lock;

/* Workqueue related */
static struct workqueue_struct *my_wq;
static DECLARE_WORK(my_work, NULL);

static bool timer_enabled = false;
static unsigned int hold_time_sec;

struct ioctl_arg {
    char path[256];
    int tz_offset; 
    int setval;
};
static struct ioctl_arg arg_data;

/* hrtimer callback */
static enum hrtimer_restart hold_timer_callback(struct hrtimer *timer)
{
    spin_unlock(&my_lock);
    printk(KERN_INFO "Hold_timer expired — spinlock released\n");
    return HRTIMER_NORESTART;
}

/* Workqueue function (replaces tasklet function) */
static void my_work_func(struct work_struct *work)
{
    char tsbuf[128];
    struct timespec64 ts;
    struct tm tm;
    struct file *filp;
    ktime_t ktime_hold;
    int n;
    loff_t pos = 0;

    pr_info("i am under workqueue function , acquiring spin lock now");
    spin_lock(&my_lock);

    pr_info("spinlock acquired , writing into the file");

    /* writing current date/time to file */
    ktime_get_real_ts64(&ts);

    /* adjust timezone */
    ts.tv_sec += arg_data.tz_offset;
    time64_to_tm(ts.tv_sec, 0, &tm);

    n = snprintf(tsbuf, sizeof(tsbuf),
                 "Date: %04ld-%02d-%02d Time: %02d:%02d:%02d\n",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);

    pr_info("%s", arg_data.path);
    filp = filp_open(arg_data.path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(filp)) {
        pr_err("datetime_drv: failed to open file %s\n", arg_data.path);
        spin_unlock(&my_lock);
        return;
    }

    kernel_write(filp, tsbuf, n, &pos);
    filp_close(filp, NULL);

    printk(KERN_INFO "Writing to file is completed, starting hr timer for 5s");

    /* using HR timer to wait */
    ktime_hold = ktime_set(5, 0);
    hrtimer_start(&hold_timer, ktime_hold, HRTIMER_MODE_REL);
}

/* ioctl entry point */
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case MYDRV_ENABLE_TIMER:
        pr_crit("Enabling the timer");
        timer_enabled = true;
        pr_info("timer is enabled, queuing work");
        queue_work(my_wq, &my_work);
        break;

    case MYDRV_SET_TIMER_VAL:
        if (copy_from_user(&arg_data, (unsigned int __user *)arg, sizeof(arg_data)))
            return -EFAULT;

        hold_time_sec = (unsigned int)arg_data.setval;
        printk(KERN_CRIT "mydrv: IOCTL SET_TIMER_VAL = %u sec\n", hold_time_sec);
        break;

    case MYDRV_STOP_TIMER:
        timer_enabled = false;
        printk(KERN_CRIT "mydrv : timer is disabled\n");
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

static int my_open(struct inode *inode, struct file *file) {
    pr_info("datetime_drv: device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file) {
    pr_info("datetime_drv: device closed\n");
    return 0;
}

static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = my_ioctl,
    .open           = my_open,
    .release        = my_release,
};

static int __init mydrv_init(void)
{
    int ret;

    /* allocate char dev region */
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("mydrv: alloc_chrdev_region failed\n");
        return ret;
    }

    /* add device */
    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret) {
        pr_err("mydrv: cdev_add failed\n");
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    /* create class */
    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }

    /* create device */
    my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }

    /* Init spinlock */
    spin_lock_init(&my_lock);

    /* Init HR timer */
    hrtimer_init(&hold_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hold_timer.function = hold_timer_callback;

    /* Init workqueue */
    my_wq = create_singlethread_workqueue("my_wq");
    INIT_WORK(&my_work, my_work_func);

    printk(KERN_INFO "mydrv: loaded, dev %d:%d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

static void __exit mydrv_exit(void)
{
    hrtimer_cancel(&hold_timer);

    /* cleanup workqueue */
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);

    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "mydrv: unloaded\n");
}

module_init(mydrv_init);
module_exit(mydrv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Exercise 3 : Ioctl + workqueue + hrtimer + spinlock");
