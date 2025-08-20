#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timekeeping.h>
#include <linux/slab.h>

#define DEVICE_NAME "datetime_drv"
#define CLASS_NAME  "datetime_class"

#define IOCTL_WRITE_DATETIME _IOW('d', 1, struct ioctl_arg *)

struct ioctl_arg {
    char path[256];   // file path
    int tz_offset;    // offset in seconds from UTC
};

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *cls;

static long datetime_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct timespec64 ts;
    struct tm tm;
    struct file *filp;
    char buf[128];
    struct ioctl_arg arg_data;
    int len;
    loff_t pos = 0;

    switch (cmd) {
        case IOCTL_WRITE_DATETIME:
            if (copy_from_user(&arg_data, (void __user *)arg, sizeof(arg_data)))
                return -EFAULT;

            // get UTC time
            ktime_get_real_ts64(&ts);

            // adjust to local time
            ts.tv_sec += arg_data.tz_offset;

            time64_to_tm(ts.tv_sec, 0, &tm);

            len = snprintf(buf, sizeof(buf),
                           "Date: %04ld-%02d-%02d Time: %02d:%02d:%02d\n",
                           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                           tm.tm_hour, tm.tm_min, tm.tm_sec);

            // open or create file
            pr_info("%s",arg_data.path);
            filp = filp_open(arg_data.path, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (IS_ERR(filp)) {
                pr_err("datetime_drv: failed to open file %s\n", arg_data.path);
                return PTR_ERR(filp);
            }

            kernel_write(filp, buf, len, &pos);
            filp_close(filp, NULL);

            pr_info("datetime_drv: wrote datetime to %s\n", arg_data.path);
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static int datetime_open(struct inode *inode, struct file *file) {
    pr_info("datetime_drv: device opened\n");
    return 0;
}

static int datetime_release(struct inode *inode, struct file *file) {
    pr_info("datetime_drv: device closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = datetime_ioctl,
    .open = datetime_open,
    .release = datetime_release,
};

static int __init datetime_init(void) {
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_num, 1);
    cls = class_create(CLASS_NAME);
    device_create(cls, NULL, dev_num, NULL, DEVICE_NAME);
    pr_info("datetime_drv: loaded\n");
    return 0;
}

static void __exit datetime_exit(void) {
    device_destroy(cls, dev_num);
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("datetime_drv: unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("IOCTL datetime writer driver with local timezone support");

module_init(datetime_init);
module_exit(datetime_exit);
