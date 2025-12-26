
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/sched/signal.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>

#define PROC_NAME "tss"

static struct timer_list my_timer;
static unsigned long my_jiffies;
static int myProcesses;

static char *myString = "Welcome to Anurag Proc Entry File";
static int myInt = 0777;


static void my_timer_callback(struct timer_list *t)
{
    struct task_struct *task;
    int count = 0;

    my_jiffies = jiffies;

    // for counting total no of processes
    for_each_process(task) {
        count++;
    }

    myProcesses = count;

    // Restarting timer after 1 second (periodic)
    mod_timer(&my_timer, jiffies + HZ);
}


static int tss_show(struct seq_file *m, void *v)
{
    seq_printf(m, "my_jiffies: %lu\n", my_jiffies);
    seq_printf(m, "myProcesses: %d\n", myProcesses);
    seq_printf(m, "myString: %s\n", myString);
    seq_printf(m, "myInt: %d\n", myInt);
    return 0;
}


static int tss_open(struct inode *inode, struct file *file)
{
    pr_info("i am under tss_open function");
    return single_open(file, tss_show, NULL);
}

static const struct proc_ops tss_fops = {
    .proc_open = tss_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init tss_init(void)
{
    // creating proc entry
    proc_create(PROC_NAME, 0, NULL, &tss_fops);

    // Setup timer
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + HZ);

    pr_info("/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit tss_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    del_timer(&my_timer);
    pr_info("/proc/%s removed\n", PROC_NAME);
}

MODULE_LICENSE("GPL");

module_init(tss_init);
module_exit(tss_exit);
