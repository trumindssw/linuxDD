#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Simple debugfs example with read/write support");

#define DEBUG_DIR  "my_debug_dir"
#define DEBUG_FILE "my_debug_entry"
#define BUFFER_SIZE 64

static struct dentry *debug_dir, *debug_file;
static int my_value = 0;
static char debug_buffer[BUFFER_SIZE];

// --- debugfs read ---
static ssize_t my_debug_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char temp[BUFFER_SIZE];
    int len;
    if (*ppos > 0)
    {
        return 0;
    }
    len = snprintf(temp, BUFFER_SIZE, "%d\n", my_value);
    pr_info("debugfs: my_value read = %d\n", my_value);

    if (copy_to_user(buf, temp, len))
    {
        return -EFAULT;
    }

    *ppos = len;
    return len;
}

// --- debugfs write ---
static ssize_t my_debug_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (count >= BUFFER_SIZE)
    {
        return -EINVAL;
    }

    if (copy_from_user(debug_buffer, buf, count))
    {
        return -EFAULT;
    }

    debug_buffer[count] = '\0';

    if (kstrtoint(debug_buffer, 10, &my_value) == 0) 
    {
        pr_info("debugfs: my_value written = %d\n", my_value);
    } 
    else 
    {
        pr_info("debugfs: invalid write (not an integer): %s\n", debug_buffer);
    }

    return count;
}

static const struct file_operations my_debug_fops = {
    .read  = my_debug_read,
    .write = my_debug_write,
};

// --- Module init ---
static int __init my_debug_init(void)
{
    debug_dir = debugfs_create_dir(DEBUG_DIR, NULL);
    if (!debug_dir) 
    {
        pr_err("debugfs: failed to create /sys/kernel/debug/%s\n", DEBUG_DIR);
        return -ENOMEM;
    }

    debug_file = debugfs_create_file(DEBUG_FILE, 0666, debug_dir, NULL, &my_debug_fops);
    if (!debug_file) 
    {
        debugfs_remove_recursive(debug_dir);
        pr_err("debugfs: failed to create %s/%s\n", DEBUG_DIR, DEBUG_FILE);
        return -ENOMEM;
    }

    pr_info("debugfs: /sys/kernel/debug/%s/%s created\n", DEBUG_DIR, DEBUG_FILE);
    return 0;
}

// --- Module exit ---
static void __exit my_debug_exit(void)
{
    debugfs_remove_recursive(debug_dir);
    pr_info("debugfs: /sys/kernel/debug/%s removed\n", DEBUG_DIR);
}

module_init(my_debug_init);
module_exit(my_debug_exit);

