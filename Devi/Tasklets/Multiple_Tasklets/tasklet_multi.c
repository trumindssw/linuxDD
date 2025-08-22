// tasklet_multi.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static void tasklet_fn1(struct tasklet_struct *t)
{
	printk(KERN_INFO "Tasklet 1 executed!\n");
}

static void tasklet_fn2(struct tasklet_struct *t)
{
	printk(KERN_INFO "Tasklet 2 executed!\n");
}

DECLARE_TASKLET(tasklet1, tasklet_fn1);
DECLARE_TASKLET(tasklet2, tasklet_fn2);

static int __init my_init(void)
{
	printk(KERN_INFO "Scheduling tasklet1 and tasklet2...\n");
	tasklet_schedule(&tasklet1);
	tasklet_schedule(&tasklet2);
	return 0;
}

static void __exit my_exit(void)
{
	tasklet_kill(&tasklet1);
	tasklet_kill(&tasklet2);
	printk(KERN_INFO "Tasklets killed, module exit.\n");
}

module_init(my_init);
module_exit(my_exit);

