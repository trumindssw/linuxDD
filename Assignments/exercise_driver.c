// exercise_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/file.h>
#include <linux/semaphore.h>
#include <linux/completion.h>

#define DRV_NAME            "exercise_driver"
#define DEV_NAME            "mydrv"

#define MYDRV_MAGIC         'T'
#define MYDRV_ENABLE_TIMER  _IO(MYDRV_MAGIC, 1)
#define MYDRV_STOP_TIMER    _IO(MYDRV_MAGIC, 2)
#define MYDRV_SET_TIMER_VAL _IOW(MYDRV_MAGIC, 3, int)

struct mydrv_device {
    /* Work/timer */
    struct workqueue_struct *wq;
    struct work_struct work;
    struct hrtimer hr_timer;
    struct completion timer_done;

    /* Sync */
    spinlock_t state_lock;     /* protects flags and timer_val */
    struct semaphore gate;     /* held while work runs, shared with .write() */

    /* Config/state */
    int timer_enabled;         /* 1 = enabled, 0 = stopped */
    int timer_val_sec;         /* 1..2 seconds as per assignment */

    /* Logging file */
    struct file *fp;
};

static struct mydrv_device *gdev;

/* ---------- Small helper: write current date/time to file ---------- */
static void mydrv_write_datetime_locked(struct file *fp)
{
    struct timespec64 ts;
    struct tm tm;
    char buf[128];
    int len;

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);

    len = scnprintf(buf, sizeof(buf),
                    "Time: %04ld-%02d-%02d %02d:%02d:%02d\n",
                    (long)(tm.tm_year + 1900), tm.tm_mon + 1, tm.tm_mday,
                    tm.tm_hour, tm.tm_min, tm.tm_sec);

    /* kernel_write updates f_pos internally when we pass &fp->f_pos */
    kernel_write(fp, buf, len, &fp->f_pos);
}

/* ---------- hrtimer callback: completes a completion ---------- */
static enum hrtimer_restart mydrv_timer_cb(struct hrtimer *t)
{
    struct mydrv_device *dev = gdev;
    if (dev)
        complete(&dev->timer_done);
    return HRTIMER_NORESTART;
}

/* ---------- Workqueue function ---------- */
static void mydrv_workfn(struct work_struct *work)
{
    struct mydrv_device *dev = gdev;
    int wait_sec;

    if (!dev)
        return;

    /*
     * Take the semaphore FIRST: this is the "Normal semaphore" demo.
     * While we hold this, user writes (.write) will block.
     */
    if (down_interruptible(&dev->gate)) {
        pr_info(DRV_NAME ": workfn: interrupted while waiting for semaphore\n");
        return;
    }

    /* Grab the spinlock only for quick, non-sleeping state access */
    spin_lock(&dev->state_lock);
    if (!dev->timer_enabled) {
        spin_unlock(&dev->state_lock);
        up(&dev->gate);
        pr_info(DRV_NAME ": workfn: timer disabled; skipping\n");
        return;
    }
    wait_sec = dev->timer_val_sec;
    spin_unlock(&dev->state_lock);

    /* Write current date/time to file (no sleeping locks held here) */
    if (dev->fp)
        mydrv_write_datetime_locked(dev->fp);

    /* Set up high-resolution wait using hrtimer + completion */
    reinit_completion(&dev->timer_done);
    hrtimer_start(&dev->hr_timer, ktime_set(wait_sec, 0), HRTIMER_MODE_REL);

    /* Sleep here (allowed), still holding the semaphore so .write() blocks */
    wait_for_completion(&dev->timer_done);

    /* Release semaphore so user-space echo/write can proceed */
    up(&dev->gate);

    pr_info(DRV_NAME ": workfn: finished (waited %d sec)\n", wait_sec);
}

/* ---------- IOCTL ---------- */
static long mydrv_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    struct mydrv_device *dev = gdev;
    int val;

    if (!dev)
        return -ENODEV;

    switch (cmd) {
    case MYDRV_ENABLE_TIMER:
        spin_lock(&dev->state_lock);
        if (dev->timer_enabled) {
            spin_unlock(&dev->state_lock);
            pr_info(DRV_NAME ": ENABLE_TIMER: queueing work\n");
            queue_work(dev->wq, &dev->work);
        } else {
            spin_unlock(&dev->state_lock);
            pr_info(DRV_NAME ": ENABLE_TIMER ignored (timer disabled)\n");
        }
        break;

    case MYDRV_STOP_TIMER:
        spin_lock(&dev->state_lock);
        dev->timer_enabled = 0;
        spin_unlock(&dev->state_lock);

        hrtimer_cancel(&dev->hr_timer);
        /* Let any current work finish; future ENABLEs will be ignored */
        pr_info(DRV_NAME ": STOP_TIMER: timer disabled\n");
        break;

    case MYDRV_SET_TIMER_VAL:
        if (copy_from_user(&val, (int __user *)arg, sizeof(val)))
            return -EFAULT;

        if (val < 1) val = 1;
        if (val > 2) val = 2; /* as per assignment */

        spin_lock(&dev->state_lock);
        dev->timer_val_sec = val;
        dev->timer_enabled = 1; /* enable on first set, per assignment */
        spin_unlock(&dev->state_lock);

        pr_info(DRV_NAME ": SET_TIMER_VAL=%d (enabled=1)\n", val);
        break;

    default:
        return -ENOTTY;
    }
    return 0;
}

/* ---------- .write so user can: echo "..." > /dev/mydrv ---------- */
/* This will block while the workfn holds dev->gate (semaphore) */
static ssize_t mydrv_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *ppos)
{
    struct mydrv_device *dev = gdev;
    char *kbuf;
    ssize_t ret = 0;

    if (!dev)
        return -ENODEV;

    /* Block here if workfn is holding the semaphore */
    if (down_interruptible(&dev->gate))
        return -ERESTARTSYS;

    /* We will also append user data to the same log file for visibility */
    kbuf = kmalloc(count + 2, GFP_KERNEL);
    if (!kbuf) {
        up(&dev->gate);
        return -ENOMEM;
    }

    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        up(&dev->gate);
        return -EFAULT;
    }
    kbuf[count] = '\n'; /* ensure newline */
    kbuf[count + 1] = '\0';

    if (dev->fp) {
        kernel_write(dev->fp, kbuf, count + 1, &dev->fp->f_pos);
        ret = count;
    } else {
        ret = -ENXIO;
    }

    kfree(kbuf);
    up(&dev->gate);
    return ret;
}

static const struct file_operations mydrv_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = mydrv_ioctl,
    .write          = mydrv_write,
};

static struct miscdevice mydrv_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEV_NAME,
    .fops  = &mydrv_fops,
};

/* ---------- Module init/exit ---------- */
static int __init mydrv_init(void)
{
    int ret;

    pr_info(DRV_NAME ": init\n");

    gdev = kzalloc(sizeof(*gdev), GFP_KERNEL);
    if (!gdev)
        return -ENOMEM;

    spin_lock_init(&gdev->state_lock);
    sema_init(&gdev->gate, 1); /* unlocked initially */

    INIT_WORK(&gdev->work, mydrv_workfn);
    init_completion(&gdev->timer_done);

    gdev->wq = alloc_workqueue("mydrv_wq", WQ_UNBOUND, 0);
    if (!gdev->wq) {
        pr_err(DRV_NAME ": alloc_workqueue failed\n");
        ret = -ENOMEM;
        goto err_free;
    }

    hrtimer_init(&gdev->hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    gdev->hr_timer.function = mydrv_timer_cb;

    /* open log file */
    gdev->fp = filp_open("/tmp/mydrv_time.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(gdev->fp)) {
        ret = PTR_ERR(gdev->fp);
        gdev->fp = NULL;
        pr_err(DRV_NAME ": filp_open failed: %d\n", ret);
        goto err_wq;
    }

    ret = misc_register(&mydrv_misc);
    if (ret) {
        pr_err(DRV_NAME ": misc_register failed: %d\n", ret);
        goto err_file;
    }

    /* default state */
    gdev->timer_enabled = 0;
    gdev->timer_val_sec = 1;

    pr_info(DRV_NAME ": loaded, device /dev/%s ready\n", DEV_NAME);
    return 0;

err_file:
    if (gdev->fp)
        filp_close(gdev->fp, NULL);
err_wq:
    destroy_workqueue(gdev->wq);
err_free:
    kfree(gdev);
    gdev = NULL;
    return ret;
}

static void __exit mydrv_exit(void)
{
    pr_info(DRV_NAME ": exit\n");

    if (!gdev)
        return;

    misc_deregister(&mydrv_misc);

    hrtimer_cancel(&gdev->hr_timer);
    flush_work(&gdev->work);
    if (gdev->wq)
        destroy_workqueue(gdev->wq);

    if (gdev->fp)
        filp_close(gdev->fp, NULL);

    kfree(gdev);
    gdev = NULL;

    pr_info(DRV_NAME ": unloaded\n");
}

module_init(mydrv_init);
module_exit(mydrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ramya");
MODULE_DESCRIPTION("Exercise driver using workqueue + hrtimer + spinlock + semaphore");

