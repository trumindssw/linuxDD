#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajaypal");
MODULE_DESCRIPTION("Single /proc/tss file with read+write");
MODULE_VERSION("1.0");

#define PROC_FILE_NAME "tss"

static unsigned long my_jiffies;
static unsigned int myProcesses;
static char my_str[64] = "hello from proc/tss";
static int my_int = 42;
static unsigned long my_new_jiffies;

static unsigned int count_processes(void)
{
    unsigned int n = 0;
    struct task_struct *p;
    for_each_process(p) {
        n++;
    pr_info("%s\n",p->comm);
    }
    return n;
}

static int tss_show(struct seq_file *m, void *v)
{
    my_jiffies = jiffies;
    count_processes();
    seq_printf(m, "my_jiffies: %lu\n", my_jiffies);
    seq_printf(m, "myProcesses: %u\n", myProcesses);
    seq_printf(m, "my_string: %s\n", my_str);
    seq_printf(m, "my_int: %d\n", my_int);
    seq_printf(m, "my_new_jiffies: %lu\n", my_new_jiffies);
    return 0;
}

static int tss_open(struct inode *inode, struct file *file)
{
    return single_open(file, tss_show, NULL);
}

static ssize_t tss_write(struct file *file, const char __user *ubuf,
                         size_t len, loff_t *ppos)
{
    char kbuf[64];
    unsigned long val;
    size_t to_copy = min(len, sizeof(kbuf) - 1);

    if (copy_from_user(kbuf, ubuf, to_copy))
        return -EFAULT;
    kbuf[to_copy] = '\0';

    if (kstrtoul(kbuf, 0, &val) == 0) {
        my_new_jiffies = val;
    } else {
        return -EINVAL;
    }

    *ppos = 0;
    return len;
}

static const struct proc_ops tss_ops = {
    .proc_open    = tss_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = tss_write,
};

static int __init tss_init(void)
{
    if (!proc_create(PROC_FILE_NAME, 0666, NULL, &tss_ops)) {
        pr_err("tss: failed to create /proc/%s\n", PROC_FILE_NAME);
        return -ENOMEM;
    }

    pr_info("tss: /proc/%s created\n", PROC_FILE_NAME);
    return 0;
}

static void __exit tss_exit(void)
{
    remove_proc_entry(PROC_FILE_NAME, NULL);
    pr_info("tss: unloaded\n");
}

module_init(tss_init);
module_exit(tss_exit);
