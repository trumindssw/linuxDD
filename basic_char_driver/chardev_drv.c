#include <linux/module.h>       
#include <linux/init.h>         
#include <linux/fs.h>        
   
#include <linux/cdev.h>         
// Character device structures

#include <linux/device.h>       
// For device creation

#include <linux/slab.h>         
// For kmalloc and kfree

#include <linux/uaccess.h>      
// user level to kernel level access


#define DEVICE_NAME "mychardev"
#define CLASS_NAME "myclass"
#define BUFFER_SIZE 1024


static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static char *kernel_buffer; 
static size_t current_len;
static size_t len;



// Open handler for device
static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev: device opened\n");
    return 0;
}

// Release or close handler
static int dev_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "mychardev: device closed\n");
    return 0;
}

// Read handler
static ssize_t dev_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) {
    // Get current data length
    len = strlen(kernel_buffer);

    // If offset is beyond data, return 0 (EOF)
    if (*ppos >= len)
        return 0;

    // Copy data from kernel to user space
    if (copy_to_user(user_buf, kernel_buffer, len)) {
        printk(KERN_ERR "mychardev: copy_to_user failed\n");
        return -EFAULT;
        // return error in case of failure
    }

    *ppos += len; // Moving the offset
    printk(KERN_INFO "mychardev: sent %zu bytes to user space\n", len);
    return len;
}

// Write handler
static ssize_t dev_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos) {
    
    current_len = strlen(kernel_buffer);

    // Ensure buffer has space to append
    if (current_len + count >= BUFFER_SIZE) {
        printk(KERN_ERR "mychardev: String is too long\n");
        return -EINVAL;
    }
    // Copy data from user to kernel space
    if (copy_from_user(kernel_buffer + current_len, user_buf, count)) {
        printk(KERN_ERR "mychardev: copy_from_user failed\n");
        return -EFAULT;
    }

    kernel_buffer[current_len + count] = '\0'; // Null terminate string
    printk(KERN_INFO "mychardev: appended: %s\n", kernel_buffer);
    return count;
}

// File operations table, this is defined in fs.h
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

// Initialization function
static int __init mychardev_init(void) {
    int ret;

    // Dynamically allocate major and minor numbers
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "Failed to allocate dev_num\n");
        return ret;
    }

    // Allocating kernel memory
    kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }

    // Initialize and register cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, dev_num, 1);

    if (ret < 0) {
        kfree(kernel_buffer);
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // Create device class under /sys/class
    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        kfree(kernel_buffer);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }

    // Create the device file under /dev
    device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "mychardev: loaded with major %d\n", MAJOR(dev_num));
    return 0;
}

// Module cleanup function
static void __exit mychardev_exit(void) {
    device_destroy(my_class, dev_num);              
    // Removing device from /dev

    class_destroy(my_class);                     
    // Removing class from /sys/class

    cdev_del(&my_cdev);                          
    // Unregistering character device

    kfree(kernel_buffer);                        
    // Freeing the allocated memory in kernel

    unregister_chrdev_region(dev_num, 1);        

    printk(KERN_INFO "mychardev: unloaded\n");
}

module_init(mychardev_init);
module_exit(mychardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION("Char device driver with automatic /dev entry");
