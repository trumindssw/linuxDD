#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/file.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/timekeeping.h>
#include <linux/kfifo.h>

#define DEVICE_NAME "mydrv"
#define CLASS_NAME  "mydrv_class"

#define MYDRV_MAGIC 'a'
#define MYDRV_ENABLE_TIMER   _IO(MYDRV_MAGIC, 1)
#define MYDRV_SET_TIMER_VAL  _IOW(MYDRV_MAGIC, 2, int)
#define MYDRV_STOP_TIMER     _IO(MYDRV_MAGIC, 3)

static dev_t dev_number;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static struct tasklet_struct my_tasklet;
static struct hrtimer hold_hrtimer;
static spinlock_t my_lock;

static unsigned int tasklet_delay_sec = 2;
static unsigned long tasklet_tick_count;

#define FIFO_BYTES (16 * 1024)
static DECLARE_KFIFO(ts_fifo, char, FIFO_BYTES);

static const char *log_path = "/tmp/mydrv_log.txt";


static void fmt_now(char *buf, size_t sz, const char *tag)
{
    struct timespec64 ts;
    struct tm tm;
    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec - (sys_tz.tz_minuteswest * 60),0, &tm);

    /* Example line: [BEGIN] 2025-08-13 12:34:56 +0000 (tick=7) */
    scnprintf(buf, sz,
              "[%s] %04ld-%02d-%02d %02d:%02d:%02d (tick=%lu)\n",
              tag,
              tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
              tm.tm_hour, tm.tm_min, tm.tm_sec,
              tasklet_tick_count);
}

static ssize_t kernel_file_append(const char *path, const char *buf, size_t len)
{
    struct file *filp;
    loff_t pos = 0;
    ssize_t ret;

    filp = filp_open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (IS_ERR(filp))
        return PTR_ERR(filp);

    ret = kernel_write(filp, buf, len, &pos);
    filp_close(filp, NULL);
    return ret;
}


static void my_tasklet_fn(unsigned long data)
{
    unsigned long flags;
    char line[128];
    pr_info("acquiring lock\n");
    spin_lock_irqsave(&my_lock, flags);
    pr_info("lock acquired\n");
    tasklet_tick_count++;
    fmt_now(line, sizeof(line), "BEGIN");

    if (kfifo_avail(&ts_fifo) >= strlen(line))
        kfifo_in(&ts_fifo, line, strlen(line));
    else
        pr_warn("mydrv: FIFO full, dropping BEGIN line\n");
    pr_info("writing done\n");
    hrtimer_start(&hold_hrtimer, ktime_set(tasklet_delay_sec, 0), HRTIMER_MODE_REL);
    spin_unlock_irqrestore(&my_lock, flags);
    pr_info("lock released\n");
    pr_info("mydrv: tasklet BEGIN (delay=%us)\n", tasklet_delay_sec);
}

static enum hrtimer_restart hold_hrtimer_cb(struct hrtimer *t)
{
    unsigned long flags;
    char line[128];

    spin_lock_irqsave(&my_lock, flags);
    fmt_now(line, sizeof(line), "AFTER");

    if (kfifo_avail(&ts_fifo) >= strlen(line))
        kfifo_in(&ts_fifo, line, strlen(line));
    else
        pr_warn("mydrv: FIFO full, dropping AFTER line\n");

    spin_unlock_irqrestore(&my_lock, flags);

    pr_info("mydrv: hrtimer AFTER\n");
    return HRTIMER_NORESTART;
}


static ssize_t mydrv_write(struct file *filep, const char __user *ubuf, size_t count, loff_t *ppos)
{
    char *kbuf = NULL;
    char drain[1024];
    int drained;
    ssize_t ret = 0;

    while ((drained = kfifo_out(&ts_fifo, drain, sizeof(drain))) > 0) {
        ssize_t w = kernel_file_append(log_path, drain, drained);
        if (w < 0) {
            pr_err("mydrv: failed append (drain): %zd\n", w);
            return w;
        }
        pr_info("mydrv: drained %d bytes to %s\n", drained, log_path);
    }

    if (count) {
        size_t n = min(count, (size_t)4096);
        kbuf = kmalloc(n, GFP_KERNEL);
        if (!kbuf) return -ENOMEM;
        if (copy_from_user(kbuf, ubuf, n)) { kfree(kbuf); return -EFAULT; }

        ret = kernel_file_append(log_path, kbuf, n);
        if (ret < 0) {
            pr_err("mydrv: failed append (user): %zd\n", ret);
            kfree(kbuf);
            return ret;
        }
        pr_info("mydrv: wrote %zu bytes to %s (user)\n", n, log_path);
        kfree(kbuf);
        return (ssize_t)n;
    }

    return 0;
}

static ssize_t mydrv_read(struct file *filep, char __user *ubuf, size_t count, loff_t *ppos)
{
    char msg[160];
    int n;
    size_t len;

    if (*ppos > 0) return 0;

    n = scnprintf(msg, sizeof(msg),
                  "mydrv: tick=%lu delay=%us fifo_len=%u\n",
                  tasklet_tick_count, tasklet_delay_sec, kfifo_len(&ts_fifo));
    len = (size_t)n;
    if (count > len) count = len;
    if (copy_to_user(ubuf, msg, count)) return -EFAULT;
    *ppos += count;
    return count;
}

static long mydrv_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int tmp;

    switch (cmd) {
    case MYDRV_ENABLE_TIMER:
        pr_crit("mydrv: IOCTL ENABLE_TIMER\n");
        tasklet_schedule(&my_tasklet);
        return 0;

    case MYDRV_SET_TIMER_VAL:
        pr_crit("mydrv: IOCTL SET_TIMER_VAL\n");
        if (copy_from_user(&tmp, (int __user *)arg, sizeof(tmp)))
            return -EFAULT;
        if (tmp < 0) return -EINVAL;
        tasklet_delay_sec = (unsigned int)tmp;
        pr_info("mydrv: tasklet_delay_sec=%u\n", tasklet_delay_sec);
        return 0;

    case MYDRV_STOP_TIMER:
        pr_crit("mydrv: IOCTL STOP_TIMER\n");
        hrtimer_cancel(&hold_hrtimer);
        tasklet_kill(&my_tasklet);
        return 0;
    }
    return -ENOTTY;
}

static int mydrv_open(struct inode *inodep, struct file *filep)
{
    pr_info("mydrv: opened\n");
    return 0;
}
static int mydrv_release(struct inode *inodep, struct file *filep)
{
    pr_info("mydrv: closed\n");
    return 0;
}

static const struct file_operations mydrv_fops = {
    .owner          = THIS_MODULE,
    .read           = mydrv_read,
    .write          = mydrv_write,
    .open           = mydrv_open,
    .release        = mydrv_release,
    .unlocked_ioctl = mydrv_ioctl,
};


static int __init mydrv_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret) return ret;

    cdev_init(&my_cdev, &mydrv_fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, dev_number, 1);
    if (ret) { unregister_chrdev_region(dev_number, 1); return ret; }

    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(my_class);
    }

    my_device = device_create(my_class, NULL, dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(my_device);
    }

    spin_lock_init(&my_lock);
    INIT_KFIFO(ts_fifo);

    tasklet_init(&my_tasklet, my_tasklet_fn, 0);

    hrtimer_init(&hold_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hold_hrtimer.function = hold_hrtimer_cb;

    tasklet_tick_count = 0;

    pr_info("mydrv: loaded, dev=%d:%d\n", MAJOR(dev_number), MINOR(dev_number));
    return 0;
}

static void __exit mydrv_exit(void)
{
    hrtimer_cancel(&hold_hrtimer);
    tasklet_kill(&my_tasklet);

    device_destroy(my_class, dev_number);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_number, 1);

    pr_info("mydrv: unloaded\n");
}

module_init(mydrv_init);
module_exit(mydrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajay");
MODULE_DESCRIPTION("Exercise 3: tasklet+spinlock with hrtimer and date+time write in log file");
