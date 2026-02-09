#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/sched/signal.h>  // for_each_process
#include <linux/uaccess.h>

#define PROC_ENTRY_FILENAME "tss"

static unsigned long my_new_jiffies = 0;
static char *myString = "Welcome to Anurag Proc Entry File";
static int myInt = 0777;

static int tss_proc_show(struct seq_file *m, void *v) {
    struct task_struct *task;
    int process_count = 0;

    // Count all processes
    for_each_process(task)
        process_count++;

    seq_printf(m, "my_jiffies: %lu\n", jiffies);
    seq_printf(m, "myProcesses: %d\n", process_count);
    seq_printf(m, "my_new_jiffies: %lu\n", my_new_jiffies);
    seq_printf(m, "myString: %s\n", myString);
    seq_printf(m, "myInt: %d\n", myInt);

    return 0;
}

static int tss_proc_open(struct inode *inode, struct file *file) {
    // pr_info("i am under tss function");
    return single_open(file, tss_proc_show, NULL);
}

// writing function to allow writing new jiffies
static ssize_t tss_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos) {
    char buf[64];
    unsigned long val;

    if (count > sizeof(buf) - 1)
        return -EINVAL;

    if (copy_from_user(buf, buffer, count))
        return -EFAULT;

    buf[count] = '\0';

    if (kstrtoul(buf, 10, &val) == 0)
        my_new_jiffies = val;

    return count;
}

static const struct proc_ops proc_fops = {
    .proc_open = tss_proc_open,
    .proc_read = seq_read,
    .proc_write = tss_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};


static int __init tss_init(void) {

    /* First have to create file name "tss"
    Parameters ->
    1 -> file name you want to create
    2 -> permissions
    3 -> parent name if you are creating under any dir , null specifies proc 
    4 -> file operations (what you want to done on file) */
    proc_create(PROC_ENTRY_FILENAME, 0666, NULL, &proc_fops);

    pr_info("/proc/%s created\n", PROC_ENTRY_FILENAME);
    
    return 0;

}

static void __exit tss_exit(void) {
    remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
    pr_info("/proc/%s removed\n", PROC_ENTRY_FILENAME);
}

MODULE_LICENSE("GPL");

module_init(tss_init);
module_exit(tss_exit);
