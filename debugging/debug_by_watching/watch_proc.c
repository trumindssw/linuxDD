#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>

#define DEVICE_NAME "watch_drv"
#define CLASS_NAME "watch_class"
#define PROC_NAME "watch_info"

static int value;
static dev_t dev;
static struct cdev my_cdev;
static struct class *my_class;


// copy_to_user() and copy_from_user() work on memory (buffers), not raw variables: These functions require pointers to memory blocks, not direct integers.

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "Dev value: %d\n", value);

    // snprintf() formats a string (like printf), but writes it into a character array (buffer) instead of printing to console.

    if (*ppos > 0) return 0;
    if (copy_to_user(buf, buffer, len)) return -EFAULT;

    // buf: a user-space pointer (e.g., from cat /dev/watch_drv)
    // buffer: a kernel-space array (e.g., char buffer[32])

    *ppos = len;
    return len;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];

    if (count > sizeof(buffer) - 1) return -EINVAL;

    if (copy_from_user(buffer, buf, count)) {
    return -EFAULT;
    }
    printk(KERN_INFO "Written Successfully\n");

    buffer[count] = '\0';
    sscanf(buffer, "%d", &value);

    // sscanf() reads formatted input from a string (buffer) and stores it into a variable.

    return count;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
};


static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "Proc value: %d\n", value);

    if (*ppos > 0) return 0;
    if (copy_to_user(buf, buffer, len)) return -EFAULT;

    *ppos = len;
    return len;
}

static ssize_t proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char buffer[32];

    if (count > sizeof(buffer) - 1) return -EINVAL;

    if (copy_from_user(buffer, buf, count)) {
    return -EFAULT;
    }
    printk(KERN_INFO "Written Successfully\n");
    buffer[count] = '\0';
    sscanf(buffer, "%d", &value);

    return count;
}

static struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init watch_init(void) {

    value = 100;
    // &dev Pointer to a `dev_t` variable that will store the assigned device number.
    // 0: Starting minor number.
    // 1:  Number of device numbers to allocate (only 1 device here).
    // DEVICE_NAME The name shown in `/proc/devices`.

    alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev, 1);

    // my_class: Class under which to create the device.
    // NULL: No parent device.
    // dev: Device number (major/minor).
    // NULL: No special data for the device.
    // DEVICE_NAME: Name of the device file.

    my_class = class_create(CLASS_NAME);
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);


    // PROC_NAME: Name of the proc file.
    // 0666: Read/write permission for all users.
    // NULL: Default parent directory in /proc/.
    // &proc_fops: File operations for the proc entry.

    proc_create(PROC_NAME, 0666, NULL, &proc_fops);

    printk(KERN_INFO "Watch driver loaded: /dev/%s, /proc/%s\n", DEVICE_NAME, PROC_NAME);
    return 0;
}

static void __exit watch_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    device_destroy(my_class, dev);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Watch driver unloaded\n");
}

module_init(watch_init);
module_exit(watch_exit);
MODULE_LICENSE("GPL");
