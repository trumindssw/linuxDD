// workqueue_sleep_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>   // for msleep

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Workqueue with Sleep Example");

static struct workqueue_struct *my_wq;
static struct work_struct my_work;

static void my_work_function(struct work_struct *work)
{
	printk(KERN_INFO "Work started...\n");
	msleep(2000);  // sleep for 2 seconds
	printk(KERN_INFO "Work finished after sleeping 2 seconds!\n");
}

static int __init my_module_init(void)
{
	printk(KERN_INFO "Loading Sleep Workqueue Module\n");

	my_wq = alloc_workqueue("sleep_wq", WQ_UNBOUND, 0);
	if (!my_wq)
		return -ENOMEM;

	INIT_WORK(&my_work, my_work_function);
	queue_work(my_wq, &my_work);

	return 0;
}

static void __exit my_module_exit(void)
{
	flush_workqueue(my_wq);
	destroy_workqueue(my_wq);
	printk(KERN_INFO "Unloading Sleep Workqueue Module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

