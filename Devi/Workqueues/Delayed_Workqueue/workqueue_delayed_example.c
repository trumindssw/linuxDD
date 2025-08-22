// workqueue_delayed_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Delayed Workqueue Example");

static struct workqueue_struct *my_wq;
static struct delayed_work my_delayed_work;

static void my_work_function(struct work_struct *work)
{
	printk(KERN_INFO "Delayed Workqueue: Executed at jiffies=%lu\n", jiffies);
}

static int __init my_module_init(void)
{
	printk(KERN_INFO "Loading Delayed Workqueue Module\n");

	my_wq = alloc_workqueue("my_delayed_wq", WQ_UNBOUND, 0);
	if (!my_wq)
		return -ENOMEM;

	INIT_DELAYED_WORK(&my_delayed_work, my_work_function);

	// Queue the work after 5 seconds (5 * HZ ticks)
	queue_delayed_work(my_wq, &my_delayed_work, 5 * HZ);

	return 0;
}

static void __exit my_module_exit(void)
{
	cancel_delayed_work_sync(&my_delayed_work);
	destroy_workqueue(my_wq);
	printk(KERN_INFO "Unloading Delayed Workqueue Module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

