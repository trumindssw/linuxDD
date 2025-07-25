#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "hello_dev"
#define MAJOR_NUM 117

MODULE_LICENSE("GPL");

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
};

static int __init hello_init(void) {
    register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    printk(KERN_INFO "Driver loaded\n");
    return 0;
}

static void __exit hello_exit(void) {
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "Driver unloaded\n");
}

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset) {
    //char msg[] = "Hello from kernel\n";
    //copy_to_user(buffer, msg, sizeof(msg));
    printk(KERN_INFO "Hello from kernel\n");
    return 0;
}


module_init(hello_init);
module_exit(hello_exit);

