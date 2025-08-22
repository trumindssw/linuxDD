// workqueue_multiple_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Multiple Work Items Example");

static struct workqueue_struct *my_wq;
static struct work_struct work1, work2;

static void work_fn1(struct work_struct *work)
{
	printk(KERN_INFO "Work1 executed at jiffies=%lu\n", jiffies);
}

static void work_fn2(struct work_struct *work)
{
	printk(KERN_INFO "Work2 executed at jiffies=%lu\n", jiffies);
}

static int __init my_module_init(void)
{
	printk(KERN_INFO "Loading Multiple Workqueue Module\n");

	my_wq = alloc_workqueue("multi_wq", WQ_UNBOUND, 0);
	if (!my_wq)
		return -ENOMEM;

	INIT_WORK(&work1, work_fn1);
	INIT_WORK(&work2, work_fn2);

	// Queue both works
	queue_work(my_wq, &work1);
	queue_work(my_wq, &work2);

	return 0;
}

static void __exit my_module_exit(void)
{
	flush_workqueue(my_wq);
	destroy_workqueue(my_wq);
	printk(KERN_INFO "Unloading Multiple Workqueue Module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

