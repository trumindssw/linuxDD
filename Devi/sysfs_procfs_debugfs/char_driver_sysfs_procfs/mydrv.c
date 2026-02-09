#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Sysfs and Procfs Character Driver Module");
MODULE_VERSION("1.0");

static struct kobject *mymod_kobj;
static int my_value = 0;

#define PROC_NAME "mymod_proc"

/* ==================== Sysfs Section ==================== */
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", my_value);
}

static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int temp;
    if (kstrtoint(buf, 10, &temp) == 0) 
    {
        my_value = temp;
        pr_info("mymod: sysfs value updated to %d\n", my_value);
        return count;
    }
    return -EINVAL;
}

static struct kobj_attribute mymod_attr = __ATTR(value, 0664, value_show, value_store);

/* ==================== Procfs Section ==================== */
static ssize_t proc_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
    char buf[64];
    int len = scnprintf(buf, sizeof(buf), "%d\n", my_value);
    return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static ssize_t proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    char kbuf[64];
    int temp;

    if (count >= sizeof(kbuf))
    {
        return -EINVAL;
    }
    if (copy_from_user(kbuf, ubuf, count))
    {
        return -EFAULT;
    }

    kbuf[count] = '\0';

    if (kstrtoint(kbuf, 10, &temp) == 0) 
    {
        my_value = temp;
        pr_info("mymod: proc value updated to %d\n", my_value);
        return count;
    }

    return -EINVAL;
}

static const struct proc_ops mymod_proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

/* ==================== Module Init/Exit ==================== */
static int __init mymod_init(void)
{
    int ret;

    pr_info("mymod: loading module safely\n");

    /* Create /sys/kernel/mymod */
    mymod_kobj = kobject_create_and_add("mymod", kernel_kobj);
    if (!mymod_kobj) 
    {
        pr_err("mymod: failed to create kobject\n");
        return -ENOMEM;
    }

    /* Create sysfs file /sys/kernel/mymod/value */
    ret = sysfs_create_file(mymod_kobj, &mymod_attr.attr);
    if (ret) 
    {
        pr_err("mymod: failed to create sysfs file\n");
        kobject_put(mymod_kobj);
        return ret;
    }

    /* Create /proc/mymod_proc */
    if (!proc_create(PROC_NAME, 0666, NULL, &mymod_proc_ops)) 
    {
        pr_err("mymod: failed to create proc entry\n");
        sysfs_remove_file(mymod_kobj, &mymod_attr.attr);
        kobject_put(mymod_kobj);
        return -ENOMEM;
    }

    pr_info("mymod: loaded successfully (sysfs:/sys/kernel/mymod/value, proc:/proc/mymod_proc)\n");
    return 0;
}

static void __exit mymod_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    sysfs_remove_file(mymod_kobj, &mymod_attr.attr);
    kobject_put(mymod_kobj);
    pr_info("mymod: unloaded safely\n");
}

module_init(mymod_init);
module_exit(mymod_exit);

