#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include <linux/delay.h>

#define DEVICE_NAME "mydrv"

// IOCTL commands
#define MYDRV_SET_TIMER_VAL  _IOW('a', 1, int)
#define MYDRV_ENABLE_TIMER  _IO('a', 2)
#define MYDRV_STOP_TIMER    _IO('a', 3)

// device data 

static dev_t dev;
static struct cdev my_cdev;
static struct class *dev_class;

// timer & work 

static struct hrtimer my_timer;
static struct work_struct my_work;

// synchronization 

static spinlock_t state_lock;     /* protects flags */
static struct mutex work_mutex;   /* sleepable section */
static struct semaphore sem;      /* demo blocking */
static atomic_t stop_requested;


static int timer_val_sec = 1;
static int timer_enabled;

 //     workqueue function 

static void my_work_fn(struct work_struct *work)
{
    if (atomic_read(&stop_requested))
        return;

    mutex_lock(&work_mutex);
    down(&sem);

    printk(KERN_INFO "mydrv: workqueue started\n");

    msleep(timer_val_sec * 1000);

    if (!atomic_read(&stop_requested))
        printk(KERN_INFO "mydrv: workqueue finished\n");

    up(&sem);
    mutex_unlock(&work_mutex);
}

//  hrtimer callback

static enum hrtimer_restart my_timer_fn(struct hrtimer *t)
{
    if (atomic_read(&stop_requested))
        return HRTIMER_NORESTART;

    schedule_work(&my_work);
    return HRTIMER_NORESTART;
}

//  file operations 

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

static long my_ioctl(struct file *file,
                     unsigned int cmd,
                     unsigned long arg)
{
    int val;

    switch (cmd) {

    case MYDRV_SET_TIMER_VAL:
        if (copy_from_user(&val, (int __user *)arg, sizeof(int)))
            return -EFAULT;

        spin_lock(&state_lock);
        timer_val_sec = val;
        spin_unlock(&state_lock);

        printk(KERN_INFO "mydrv: SET_TIMER_VAL = %d sec\n", val);
        break;

    case MYDRV_ENABLE_TIMER:
        spin_lock(&state_lock);
        timer_enabled = 1;
        atomic_set(&stop_requested, 0);
        spin_unlock(&state_lock);

        hrtimer_start(&my_timer,
                      ktime_set(timer_val_sec, 0),
                      HRTIMER_MODE_REL);

        printk(KERN_INFO "mydrv: ENABLE_TIMER\n");
        break;

    case MYDRV_STOP_TIMER:
        spin_lock(&state_lock);
        timer_enabled = 0;
        atomic_set(&stop_requested, 1);
        spin_unlock(&state_lock);

        hrtimer_cancel(&my_timer);
        cancel_work_sync(&my_work);

        printk(KERN_INFO "mydrv: STOP_TIMER\n");
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

// fops 

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = my_open,
    .release        = my_release,
    .unlocked_ioctl = my_ioctl,
};

//  init 

static int __init my_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    cdev_init(&my_cdev, &fops);
    ret = cdev_add(&my_cdev, dev, 1);
    if (ret)
        goto err_cdev;

    dev_class = class_create(DEVICE_NAME);
    if (IS_ERR(dev_class)) {
        ret = PTR_ERR(dev_class);
        goto err_class;
    }

    device_create(dev_class, NULL, dev, NULL, DEVICE_NAME);

    spin_lock_init(&state_lock);
    mutex_init(&work_mutex);
    sema_init(&sem, 1);
    atomic_set(&stop_requested, 0);

    INIT_WORK(&my_work, my_work_fn);

    hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_timer.function = my_timer_fn;

    printk(KERN_INFO "mydrv: driver loaded\n");
    return 0;

err_class:
    cdev_del(&my_cdev);
err_cdev:
    unregister_chrdev_region(dev, 1);
    return ret;
}

//  exit 

static void __exit my_exit(void)
{
    atomic_set(&stop_requested, 1);
    hrtimer_cancel(&my_timer);
    cancel_work_sync(&my_work);

    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "mydrv: driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rana Yuvraj Singh");
MODULE_DESCRIPTION("Advanced char driver with ioctl, hrtimer, workqueue & sync");
