// workqueue_periodic_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Periodic Workqueue Example");

static struct workqueue_struct *my_wq;
static struct delayed_work my_periodic_work;

static void my_work_function(struct work_struct *work)
{
	printk(KERN_INFO "Periodic Workqueue: Executed at jiffies=%lu\n", jiffies);

	// Re-queue after 2 seconds
	queue_delayed_work(my_wq, &my_periodic_work, 2 * HZ);
}

static int __init my_module_init(void)
{
	printk(KERN_INFO "Loading Periodic Workqueue Module\n");

	my_wq = alloc_workqueue("my_periodic_wq", WQ_UNBOUND, 0);
	if (!my_wq)
		return -ENOMEM;

	INIT_DELAYED_WORK(&my_periodic_work, my_work_function);

	// Start the loop with initial delay of 1 second
	queue_delayed_work(my_wq, &my_periodic_work, 1 * HZ);

	return 0;
}

static void __exit my_module_exit(void)
{
	cancel_delayed_work_sync(&my_periodic_work);
	destroy_workqueue(my_wq);
	printk(KERN_INFO "Unloading Periodic Workqueue Module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

