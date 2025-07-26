#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> // for creating proc file 
#include <linux/uaccess.h>  // for copy_to_user and copy_from_user

#define PROC_NAME "myinfo"

// This is the read function called on cat /proc/info
static ssize_t myinfo_read(struct file *file, char __user *buf,
                           size_t count, loff_t *ppos)
{
    char message[] = "This is internal debug info from kernel module.\n";
    int len = sizeof(message);

    // Only allow one read
    if (*ppos > 0)
        return 0;

    if (copy_to_user(buf, message, len))
        return -EFAULT;
        // returns an error on fault.

    *ppos = len;
    return len;
}

static const struct proc_ops proc_fops = {
    .proc_read = myinfo_read,
};

static int __init proc_debug_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &proc_fops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit proc_debug_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(proc_debug_init);
module_exit(proc_debug_exit);
MODULE_LICENSE("GPL");
