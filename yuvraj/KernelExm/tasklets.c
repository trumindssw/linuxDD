#include <linux/module.h>
#include <linux/interrupt.h>

static void tasklet_fn(struct tasklet_struct *t);

DECLARE_TASKLET(my_tasklet, tasklet_fn);

static void tasklet_fn(struct tasklet_struct *t)
{
    printk(KERN_INFO "tasklet executed\n");
}

static int __init my_init(void)
{
    tasklet_schedule(&my_tasklet);
    printk(KERN_INFO "tasklet scheduled\n");
    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(&my_tasklet);
    printk(KERN_INFO "tasklet module exit\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
