#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>   // copy_from_user
#include <linux/init.h>

#define PROC_NAME "tss"

static char my_string[] = "Hello";
static int  my_integer  = 42;

/* NEW: this holds whatever our app writes */
static unsigned long my_new_jiffies;

/* ---------- helpers ---------- */

static int count_processes(void)
{
    struct task_struct *task;
    int count = 0;

    for_each_process(task)
        count++;

    return count;
}

/* ---------- /proc/tss (read-only) ---------- */

static int tss_show(struct seq_file *m, void *v)
{
    /* jiffies is the live kernel tick counter */
    seq_printf(m, "my_jiffies: %lu\n", jiffies);
    seq_printf(m, "myProcesses: %d\n", count_processes());
    seq_printf(m, "myString: %s\n", my_string);
    seq_printf(m, "myInteger: %d\n", my_integer);

    /* show the value our app keeps updating */
    seq_printf(m, "my_new_jiffies: %lu\n", my_new_jiffies);
    return 0;
}

static int tss_open(struct inode *inode, struct file *file)
{
    return single_open(file, tss_show, NULL);
}

static const struct proc_ops tss_fops = {
    .proc_open    = tss_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* ---------- /proc/my_new_jiffies (read + write) ---------- */

static int my_new_jiffies_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%lu\n", my_new_jiffies);
    return 0;
}

static int my_new_jiffies_open(struct inode *inode, struct file *file)
{
    return single_open(file, my_new_jiffies_show, NULL);
}

/* accept a decimal integer and store it in my_new_jiffies */
static ssize_t my_new_jiffies_write(struct file *file,
                                    const char __user *ubuf,
                                    size_t len, loff_t *ppos)
{
    char buf[32];
    size_t n = (len < sizeof(buf) - 1) ? len : (sizeof(buf) - 1);
    unsigned long val;

    if (copy_from_user(buf, ubuf, n))
        return -EFAULT;
    buf[n] = '\0';

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    my_new_jiffies = val;
    return len;
}

static const struct proc_ops my_new_jiffies_fops = {
    .proc_open    = my_new_jiffies_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_write   = my_new_jiffies_write,   // <-- enables writes
};

/* ---------- module init/exit ---------- */

static int __init tss_init(void)
{
    struct proc_dir_entry *e1, *e2;

    /* /proc/tss: read-only (0444) */
    e1 = proc_create(PROC_NAME, 0444, NULL, &tss_fops);
    if (!e1) {
        pr_err("failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    /* /proc/my_new_jiffies: readable + writable (0666 for easy testing) */
    e2 = proc_create("my_new_jiffies", 0666, NULL, &my_new_jiffies_fops);
    if (!e2) {
        remove_proc_entry(PROC_NAME, NULL);
        pr_err("failed to create /proc/my_new_jiffies\n");
        return -ENOMEM;
    }

    my_new_jiffies = 0; /* initial value */
    pr_info("/proc/%s and /proc/my_new_jiffies created\n", PROC_NAME);
    return 0;
}

static void __exit tss_exit(void)
{
    remove_proc_entry("my_new_jiffies", NULL);
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("/proc/%s and /proc/my_new_jiffies removed\n", PROC_NAME);
}

module_init(tss_init);
module_exit(tss_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prasanthi");
MODULE_DESCRIPTION("Procfs: tss + my_new_jiffies (rw)");



