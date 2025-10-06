/* semmutex_char.c
 * Character driver demonstrating semaphore and mutex usage (no goto style).
 *
 * Author: Keerthana
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Keerthana");
MODULE_DESCRIPTION("Character driver demonstrating semaphore and mutex (without goto)");

#define DEVICE_NAME "semmutex"
#define BUFFER_SIZE 1024

/* module parameter: how many concurrent openers allowed (semaphore initial count) */
static int max_users = 2;
module_param(max_users, int, 0444);
MODULE_PARM_DESC(max_users, "Maximum concurrent openers (semaphore initial count)");

/* device infrastructure */
static dev_t devt;
static struct class *drv_class;
static struct cdev drv_cdev;

/* device storage & tracking */
static char *device_buffer;
static size_t data_size; /* valid bytes in buffer */

/* synchronization primitives */
static struct semaphore sem;    /* counting semaphore */
static struct mutex buf_lock;   /* mutex for protecting buffer/data_size */

/* --- file operations --- */

static int semmutex_open(struct inode *inode, struct file *file)
{
    if (down_interruptible(&sem))
        return -ERESTARTSYS;

    pr_info("%s: open succeeded (semaphore taken)\n", DEVICE_NAME);
    return 0;
}

static int semmutex_release(struct inode *inode, struct file *file)
{
    up(&sem);
    pr_info("%s: release called (semaphore released)\n", DEVICE_NAME);
    return 0;
}

static ssize_t semmutex_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    ssize_t avail, to_copy;

    if (mutex_lock_interruptible(&buf_lock))
        return -ERESTARTSYS;

    if (*offset >= data_size) {
        mutex_unlock(&buf_lock);
        return -EINVAL;  /* nothing left to read */
    }

    avail = (ssize_t)(data_size - *offset);
    to_copy = min((ssize_t)len, avail);

    if (copy_to_user(buf, device_buffer + *offset, to_copy)) {
        mutex_unlock(&buf_lock);
        return -EFAULT;
    }

    *offset += to_copy;
    mutex_unlock(&buf_lock);

    pr_debug("%s: read returning %zu\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static ssize_t semmutex_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    ssize_t avail, to_copy;

    if (mutex_lock_interruptible(&buf_lock))
        return -ERESTARTSYS;

    if (*offset >= BUFFER_SIZE) {
        mutex_unlock(&buf_lock);
        return -ENOSPC;
    }

    avail = (ssize_t)(BUFFER_SIZE - *offset);
    to_copy = min((ssize_t)len, avail);

    if (to_copy <= 0) {
        mutex_unlock(&buf_lock);
        return -ENOSPC;
    }

    if (copy_from_user(device_buffer + *offset, buf, to_copy)) {
        mutex_unlock(&buf_lock);
        return -EFAULT;
    }

    *offset += to_copy;
    if (*offset > data_size)
        data_size = *offset;

    mutex_unlock(&buf_lock);

    pr_debug("%s: write returning %zu\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static const struct file_operations semmutex_fops = {
    .owner   = THIS_MODULE,
    .open    = semmutex_open,
    .release = semmutex_release,
    .read    = semmutex_read,
    .write   = semmutex_write,
};

/* --- module init / exit --- */

static int __init semmutex_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    drv_class = class_create(DEVICE_NAME);
    if (IS_ERR(drv_class)) {
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(drv_class);
    }

    cdev_init(&drv_cdev, &semmutex_fops);
    drv_cdev.owner = THIS_MODULE;

    ret = cdev_add(&drv_cdev, devt, 1);
    if (ret) {
        class_destroy(drv_class);
        unregister_chrdev_region(devt, 1);
        return ret;
    }

    device_create(drv_class, NULL, devt, NULL, DEVICE_NAME "0");

    device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer) {
        device_destroy(drv_class, devt);
        cdev_del(&drv_cdev);
        class_destroy(drv_class);
        unregister_chrdev_region(devt, 1);
        return -ENOMEM;
    }
    data_size = 0;

    sema_init(&sem, max_users);
    mutex_init(&buf_lock);

    pr_info("%s: loaded (major=%d minor=%d, max_users=%d)\n",
            DEVICE_NAME, MAJOR(devt), MINOR(devt), max_users);
    return 0;
}

static void __exit semmutex_exit(void)
{
    kfree(device_buffer);
    device_destroy(drv_class, devt);
    cdev_del(&drv_cdev);
    class_destroy(drv_class);
    unregister_chrdev_region(devt, 1);
    pr_info("%s: unloaded\n", DEVICE_NAME);
}

module_init(semmutex_init);
module_exit(semmutex_exit);

