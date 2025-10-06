#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>

#define DEVICE_NAME "syncdev"
#define BUF_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION("Simple char device using semaphores and mutex");

static dev_t dev_number;
static struct cdev sync_cdev;
static struct class *sync_class;

// single-slot message buffer 
static char* msg_buf;
static size_t msg_len;

// synchronization primitives 
static struct semaphore sem_empty; // counts empty slots (0 or 1) 
static struct semaphore sem_full;  // counts full slots (0 or 1) 
static struct mutex buf_mutex;     // protects msg_buf and msg_len 

// open/release - trivial 
static int sync_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    // Incrments usage count, basically useful if any other device is using the driver, prevent rmmod during use.
    printk("Open function");
    return 0;
}

static int sync_release(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    // Decrements usage count, allows unloading when no longer in use.
    printk("Close function");
    return 0;
}


// write() - Producer: places message into the single-slot buffer.
// Behavior:
// - Blocks if buffer is full (sem_empty == 0)
// - On success, returns number of bytes written


// write in file operation called when a user-space program does something like write(fd, *msg, length)

static ssize_t sync_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
// ubuf - pointer to the user space buffer which contains the data to write
// count - size of the user data 
{
    ssize_t ret;
    ret = 0;

    if (count == 0)
        return 0;

    if (count > BUF_SIZE)
        return -EINVAL;

    // Wait for empty slot

    if (down_interruptible(&sem_empty))
        return -EINTR; 
    
    // interrupted by signal, process wakes up before slot available.

    // Got empty slot: lock buffer and copy 
    mutex_lock(&buf_mutex);
    if (copy_from_user(msg_buf, ubuf, count)) {
        ret = -EFAULT;
        msg_len = 0;
    } else {
        msg_len = count;
        ret = count;
    }
    mutex_unlock(&buf_mutex);
 
    up(&sem_full);

    return ret;
}

// read() - Consumer: takes message from the single-slot buffer.
// Behavior:
// - Blocks if buffer is empty (sem_full == 0)
// - On success, returns number of bytes read


static ssize_t sync_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    ssize_t ret;
    size_t copy_count;
    ret = 0;

    // Wait for data 
    if (down_interruptible(&sem_full))
        return -EINTR;

    // Lock and copy data out 
    mutex_lock(&buf_mutex);
    copy_count = min(count, msg_len);
    if (copy_to_user(ubuf, msg_buf, copy_count)) {
        ret = -EFAULT;
    } else {
        ret = copy_count;
        // clear slot 
        msg_len = 0;
    }
    mutex_unlock(&buf_mutex);

    // Mark buffer empty 
    up(&sem_empty);

    return ret;
}
// workflow
// Writer writes:
// down_interruptible(&sem_empty)	-> Wait for space	-> decrements to 0	-> sem_empty = 0 and sem_full = 0.
// then acquiring the lock for only one thread to access.
// up(&sem_full)    -> Signal data ready	-> increments full	-> sem_empty = 0 and sem_full = 1.
// Reader reads:
// down_interruptible(&sem_full)	-> Wait for data	-> decrements to 0	-> sem_empty = 0 and sem_full = 0.
// up(&sem_full)    -> Signal data ready	-> increments full	-> sem_empty = 0 and sem_full = 1.   

static const struct file_operations sync_fops = {
    .owner = THIS_MODULE,
    .open = sync_open,
    .release = sync_release,
    .read = sync_read,
    .write = sync_write,
};

static int __init sync_init(void)
{
    int ret;

    // allocate device number 
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("syncdev: can't alloc char dev region\n");
        return ret;
    }

    // init cdev 
    cdev_init(&sync_cdev, &sync_fops);
    sync_cdev.owner = THIS_MODULE;
    ret = cdev_add(&sync_cdev, dev_number, 1);
    if (ret) {
        pr_err("syncdev: cdev_add failed\n");
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    // create class and device node (so udev can create /dev/syncdev) 
    sync_class = class_create("syncdev_class");
    if (IS_ERR(sync_class)) {
        pr_err("syncdev: class_create failed\n");
        cdev_del(&sync_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(sync_class);
    }

    if (IS_ERR(device_create(sync_class, NULL, dev_number, NULL, DEVICE_NAME))) {
        pr_err("syncdev: device_create failed\n");
        class_destroy(sync_class);
        cdev_del(&sync_cdev);
        unregister_chrdev_region(dev_number, 1);
        return -ENOMEM;
    }

    // allocate buffer 
    msg_buf = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!msg_buf) {
        pr_err("syncdev: kmalloc failed\n");
        device_destroy(sync_class, dev_number);
        class_destroy(sync_class);
        cdev_del(&sync_cdev);
        unregister_chrdev_region(dev_number, 1);
        return -ENOMEM;
    }
    msg_len = 0;

    // init synchronization primitives 
    sema_init(&sem_empty, 1); // one empty slot initially 
    // we can change the number here according to the no.of devices can have access to the buffer, but we need to be caareful and use multiple buffers to prevent overwrites.
    sema_init(&sem_full, 0);  // no message initially 
    mutex_init(&buf_mutex);

    pr_info("syncdev: loaded, major=%d minor=%d\n", MAJOR(dev_number), MINOR(dev_number));
    return 0;
}

static void __exit sync_exit(void)
{
    kfree(msg_buf);
    device_destroy(sync_class, dev_number);
    class_destroy(sync_class);
    cdev_del(&sync_cdev);
    unregister_chrdev_region(dev_number, 1);
    pr_info("syncdev: unloaded\n");
}

module_init(sync_init);
module_exit(sync_exit);
