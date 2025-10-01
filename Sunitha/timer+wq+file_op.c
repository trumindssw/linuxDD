#include <linux/module.h>       // MODULE_LICENSE, module_init, module_exit
#include <linux/kernel.h>       // pr_info, printk
#include <linux/init.h>         // __init, __exit
#include <linux/fs.h>           // register_chrdev_region, alloc_chrdev_region, file_operations
#include <linux/cdev.h>         // struct cdev, cdev_init, cdev_add, cdev_del
#include <linux/device.h>       // class_create, device_create, device_destroy, class_destroy
#include <linux/uaccess.h>      // copy_to_user, copy_from_user
#include <linux/ioctl.h>        // _IO, _IOW macros
#include <linux/timer.h>        // struct timer_list, timer_setup, mod_timer, del_timer_sync
#include <linux/workqueue.h>    // struct work_struct, INIT_WORK, schedule_work, cancel_work_sync
#include <linux/timekeeping.h>  // ktime_get_real_ts64, struct timespec64
#include <linux/time.h>         // struct tm, time64_to_tm
#include <linux/slab.h>         // kmalloc, kfree (if needed)
#include <linux/types.h>        // dev_t
#include <linux/delay.h>


#define MY_MAGIC 'T'
#define IOCTL_TIMER_SET   _IOW(MY_MAGIC, 1, int)
#define IOCTL_TIMER_START _IO(MY_MAGIC, 2)
#define IOCTL_TIMER_STOP  _IO(MY_MAGIC, 3)


static struct class *my_class;
static dev_t dev_num;
static struct cdev my_cdev;

static struct timer_list my_timer;
static int timer_interval = 0;   // ms
static int timer_enabled = 0;

// Workqueue
static struct work_struct my_work;

// Timer callback → schedule work
static void my_timer_callback(struct timer_list *t)
{
    pr_info("Timer expired, scheduling workqueue\n");
    msleep(100);
    schedule_work(&my_work);

    /*if (timer_enabled)
        mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_interval));*/
}

// Workqueue function → write current date/time into a file
static void my_work_func(struct work_struct *work)
{
	/*Use this when you need high-precision, 64-bit time in kernel space.
          Old version: struct timespec used 32-bit tv_sec on 32-bit systems → overflow in 2038.
        */
    struct timespec64 ts;
    struct tm tm;
    struct file *filp;
    char buf[128];
    int len;
    loff_t pos = 0;

    ktime_get_real_ts64(&ts);  // <-- initializes ts.tv_sec

    ts.tv_sec = ts.tv_sec + 5*3600 + 30*60;  //convert UTC time to IST(add 5 hr 30 min)

    //time64_to_tm(time64_t time, int offset(days), struct tm *tm)
    time64_to_tm(ts.tv_sec, 0, &tm);  // convert epoch time to human readable time


    //format the time and date string into the buffer
    len = snprintf(buf, sizeof(buf),
        "Date: %04ld-%02d-%02d Time: %02d:%02d:%02d\n",
        (tm.tm_year + 1900), tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Open file for appending
    filp = filp_open("/tmp/timer_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(filp)) {
        pr_err("Failed to open log file\n");
        return;
    }

    // Write data at current file position
    kernel_write(filp, buf, len, &pos);

    filp_close(filp, NULL);
}


// IOCTL handler
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd) {
        case IOCTL_TIMER_SET:
            if (copy_from_user(&timer_interval, (int __user *)arg, sizeof(int)))
                return -EFAULT;
            pr_info("Timer interval set to %d ms\n", timer_interval);
            break;

        case IOCTL_TIMER_START:
            timer_enabled = 1;
	    //jiffies → current kernel tick count .msecs_to_jiffies(10) → converts 10 milliseconds to jiffies
            mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_interval));
            break;

        case IOCTL_TIMER_STOP:
            timer_enabled = 0;
            del_timer(&my_timer);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

// File operations
static struct file_operations fops = {
    .unlocked_ioctl = my_ioctl,
};

// Init
static int __init my_init(void)
{
    int ret;

    INIT_WORK(&my_work, my_work_func);
    timer_setup(&my_timer, my_timer_callback, 0);  //3rd argument , the value that sends to callback function.


    // Allocate major/minor dynamically
    ret = alloc_chrdev_region(&dev_num, 0, 1, "mytimer");
    if (ret < 0) return ret;

    // Register cdev
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_num, 1);

    // Create class & device → auto /dev/mytimer
    my_class = class_create("my_class");
    device_create(my_class, NULL, dev_num, NULL, "mytimer");

    pr_info("Timer driver loaded: major=%d minor=%d\n",
            MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

// Exit
static void __exit my_exit(void)
{
    del_timer_sync(&my_timer);
    cancel_work_sync(&my_work);

    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("Timer driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");

