#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "seqdemo"

static int my_array[] = {10, 20, 30, 40, 50};  // Data to show

// START: called first, like an iterator
static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= ARRAY_SIZE(my_array))
        return NULL;  // End of file
    return &my_array[*pos];
}

// NEXT: called after show() for next element
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    return my_seq_start(s, pos);
}

// STOP: called after all iterations (optional cleanup)
static void my_seq_stop(struct seq_file *s, void *v) { 

    
}

// SHOW: called for each valid element
static int my_seq_show(struct seq_file *s, void *v)
{
    int *val = (int *)v;
    seq_printf(s, "Value: %d\n", *val);
    return 0;
}


static const struct seq_operations my_seq_ops = {
    .start = my_seq_start,
    .next  = my_seq_next,
    .stop  = my_seq_stop,
    .show  = my_seq_show,
};

// Open handler
static int my_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &my_seq_ops);
}

// File operations using seq_file
static const struct proc_ops my_proc_ops = {
    .proc_open    = my_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = seq_release,
};

// Init and cleanup
static int __init seqdemo_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &my_proc_ops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit seqdemo_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(seqdemo_init);
module_exit(seqdemo_exit);
MODULE_LICENSE("GPL");
