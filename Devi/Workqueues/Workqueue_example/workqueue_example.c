#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Workqueue Example with Private Queue");

// Define a pointer to our private workqueue
static struct workqueue_struct *my_wq;

// Define the work_struct
static struct work_struct my_work;

// Work function
static void my_work_function(struct work_struct *work)
{
	printk(KERN_INFO "Workqueue: Running deferred work at jiffies=%lu\n", jiffies);
	// Here we are in process context, so sleeping is allowed if needed
}

// Module init
static int __init my_module_init(void)
{
	printk(KERN_INFO "Loading Workqueue Module\n");

	// 1. Create a private workqueue named "my_queue"
	//    WQ_UNBOUND -> not tied to a single CPU, 0 -> default max active works
	my_wq = alloc_workqueue("my_queue", WQ_UNBOUND, 0);
	if (!my_wq) {
		printk(KERN_ERR "Failed to create workqueue\n");
		return -ENOMEM;
	}

	// 2. Initialize the work with our function
	INIT_WORK(&my_work, my_work_function);

	// 3. Queue the work into our private workqueue
	queue_work(my_wq, &my_work);

	return 0;
}

// Module exit
static void __exit my_module_exit(void)
{
	// 1. Ensure any pending work in our queue is finished
	flush_workqueue(my_wq);

	// 2. Destroy the private workqueue (free resources)
	destroy_workqueue(my_wq);

	printk(KERN_INFO "Unloading Workqueue Module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

