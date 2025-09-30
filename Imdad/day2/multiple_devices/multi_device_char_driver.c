#include <linux/module.h> // for kernel modules
#include <linux/kernel.h> 
#include <linux/fs.h> // file operations
#include <linux/cdev.h> // character device objects
#include <linux/device.h> // class create
#include <linux/slab.h> // memory allocations (zalloc, malloc)
#include <linux/uaccess.h> // copy_from and copy_to

#define DEVICE_NAME "MyCharDevice" // base name
#define DEVICE_COUNT 4       // Number of devices
#define BUFFER_SIZE 256      // Per-device buffer size

static dev_t dev_num; // struct for storing major and minor number
static struct cdev my_cdev[DEVICE_COUNT]; // one cdev structure per device
static struct class *my_class;  // class needed for auto node creation in /sys/class

struct device_data {
    char *buffer;
    size_t size;
};
// per device struct (one buffer for each minor number)

static struct device_data dev_data[DEVICE_COUNT];
// declaring an array named dev_data which holds device_count number of device_data structures.

static int my_open(struct inode *inode, struct file *file)
{
    int minor = MINOR(inode->i_rdev); // get the minor number of the device
    file->private_data = &dev_data[minor];  // attach device-specific data
    pr_info("%s: Device %d opened\n", DEVICE_NAME, minor);
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    int minor = MINOR(inode->i_rdev); 
    pr_info("%s: Device %d closed\n", DEVICE_NAME, minor);
    return 0;
}

static ssize_t my_write(struct file *file, const char __user *buf,
                        size_t len, loff_t *offset)
{
    struct device_data *data = file->private_data;
    // accessing the device_data of a particular device (basically the dev_data[minor] from the array)
    size_t to_copy;

    if (len > BUFFER_SIZE)
        to_copy = BUFFER_SIZE;
    else
        to_copy = len;

    if (copy_from_user(data->buffer, buf, to_copy))
        return -EFAULT;

    data->size = to_copy;
    pr_info("%s: Written %zu bytes\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static ssize_t my_read(struct file *file, char __user *buf,
                       size_t len, loff_t *offset)
{
    struct device_data *data = file->private_data;
    size_t to_copy;

    if (*offset >= data->size)
        return 0;

    to_copy = min(len, data->size - (size_t)*offset);

    if (copy_to_user(buf, data->buffer + *offset, to_copy))
        return -EFAULT;

    *offset += to_copy;
    pr_info("%s: Read %zu bytes\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = my_open,
    .release = my_release,
    .read    = my_read,
    .write   = my_write,
};

static int __init mychardev_init(void)
{
    int ret, i;

    // Allocate device numbers for DEVICE_COUNT devices
    ret = alloc_chrdev_region(&dev_num, 0, DEVICE_COUNT, DEVICE_NAME);
    if (ret < 0) {
        pr_err("Failed to allocate device numbers\n");
        return ret;
    }

    // Create device class
    my_class = class_create(DEVICE_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(dev_num, DEVICE_COUNT);
        return PTR_ERR(my_class);
    }

    // Initialize each device
    for (i = 0; i < DEVICE_COUNT; i++) {
        cdev_init(&my_cdev[i], &fops);
        ret = cdev_add(&my_cdev[i], dev_num + i, 1);
        if (ret < 0) {
            pr_err("Failed to add cdev %d\n", i);
            continue;
        }

        // Allocate buffer for each device
        dev_data[i].buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
        dev_data[i].size = 0;

        // Create device node: /dev/MyCharDevice<i>
        device_create(my_class, NULL, dev_num + i, NULL, DEVICE_NAME "%d", i);
    }

    pr_info("%s: Module loaded (Major=%d)\n", DEVICE_NAME, MAJOR(dev_num));
    return 0;
}

static void __exit mychardev_exit(void)
{
    int i;
    for (i = 0; i < DEVICE_COUNT; i++) {
        device_destroy(my_class, dev_num + i);
        cdev_del(&my_cdev[i]);
        kfree(dev_data[i].buffer);
    }
    class_destroy(my_class);
    unregister_chrdev_region(dev_num, DEVICE_COUNT);
    pr_info("%s: Module unloaded\n", DEVICE_NAME);
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION(" Multi-device char driver");
