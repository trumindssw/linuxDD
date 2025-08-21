#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/timekeeping.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>       
#include <linux/delay.h>      
#include <linux/ktime.h>

#define DEVICE_NAME "my_drv_"
#define CLASS_NAME  "my_class1_"

#define MYDRV_ENABLE_TIMER  _IO('M', 1)
#define MYDRV_SET_TIMER_VAL _IOW('M', 2, unsigned int)
#define MYDRV_STOP_TIMER    _IO('M', 3)

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static struct workqueue_struct *my_wq;
static DECLARE_WORK(my_work, NULL);

static DEFINE_MUTEX(my_mutex);
static struct hrtimer hold_timer;


static bool timer_enabled = false;
static unsigned int hold_time_sec ;

struct ioctl_arg {
    char path[256];
    int tz_offset;
    int setval;
};
static struct ioctl_arg arg_data;

static enum hrtimer_restart hold_timer_callback(struct hrtimer *timer)
{
    mutex_unlock(&my_mutex);
    printk(KERN_INFO "Hold_timer expired , mutex released\n");
    return HRTIMER_NORESTART;
}

static void my_work_func(struct work_struct *work)
{
    char tsbuf[128];
    struct timespec64 ts;
    struct tm tm;
    struct file *filp;
    int n;
    ktime_t ktime_hold;
    loff_t pos = 0;

    if (!timer_enabled) {
        pr_info("Timer not enabled, returning\n");
        return;
    }

    pr_info("I am under work function , acquiring mutex");
    mutex_lock(&my_mutex);

    pr_info("Locked , writing current date/time to the file");

    ktime_get_real_ts64(&ts);
    // converting to local time
    ts.tv_sec += arg_data.tz_offset;
    time64_to_tm(ts.tv_sec, 0, &tm);

    n = scnprintf(tsbuf, sizeof(tsbuf),
                 "Date: %04ld-%02d-%02d Time: %02d:%02d:%02d\n",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec);

    filp = filp_open(arg_data.path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(filp)) {
        pr_err("Failed to open %s\n", arg_data.path);
        mutex_unlock(&my_mutex);
        return;
    }

    kernel_write(filp, tsbuf, n, &pos);
    filp_close(filp, NULL);

    hrtimer_cancel(&hold_timer);

    pr_info("Writing done , holding mutex for %u s by using HR timers", hold_time_sec);

    // using HR timer to wait
    ktime_hold = ktime_set(5, 0);
    hrtimer_start(&hold_timer, ktime_hold, HRTIMER_MODE_REL);

}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    // we have to set,enable or stop timer we using cmd argument for that
    // cmd ---> set , enable , stop

    // make 3 switch cases for set , enable , stop
    switch (cmd) {
    case MYDRV_ENABLE_TIMER:
        if(!timer_enabled){
            pr_crit("Timer is disabled");
            break;
        }
        else{
            pr_info("Timer is enabled , scheduling the work");
            queue_work(my_wq, &my_work);
        }
        break;

    case MYDRV_SET_TIMER_VAL:
        if (copy_from_user(&arg_data, (void __user *)arg, sizeof(arg_data)))
            return -EFAULT;
        hold_time_sec = (unsigned int)arg_data.setval;
        pr_crit("Setting SET_TIMER_VAL = %u sec\n", hold_time_sec);
        break;

    case MYDRV_STOP_TIMER:
        timer_enabled = false;
        pr_crit("Timer is disabled\n");
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

static int my_open(struct inode *inode, struct file *file) {
    printk(KERN_NOTICE "Device opened\n");
    return 0;
}
static int my_release(struct inode *inode, struct file *file) {
    pr_info("Device closed\n");
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
    int ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret) return ret;

    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev_num, 1);
    if (ret) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }

    my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }


    my_wq = create_singlethread_workqueue("my_wq");

    INIT_WORK(&my_work, my_work_func);
    timer_enabled = true;


    hrtimer_init(&hold_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hold_timer.function = hold_timer_callback;

    printk(KERN_NOTICE "Driver loaded successfully with Major no - %d , Minor no - %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

static void __exit mydrv_exit(void)
{
    if (my_wq) {
        flush_workqueue(my_wq);
        destroy_workqueue(my_wq);
    }

    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    

    pr_info("Driver unloaded successfully.\n");
}

module_init(mydrv_init);
module_exit(mydrv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Exercise 3 - Ioctl + workqueue + mutex + hrtimers");
