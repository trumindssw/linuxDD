// basic workqueue example

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anurag Bansal");
MODULE_DESCRIPTION("Simple Workqueue Example with Renamed Init");

static struct work_struct my_work;

static void work_handler(struct work_struct *work)
{
    pr_info("Workqueue handler: Running in process context\n");
    msleep(10000);  // Sleep for 1 second 
    pr_info("Workqueue handler: Finished work\n");
}

static int __init my_workqueue_module_init(void)
{
    pr_info("Workqueue Module: Initializing...\n");

    // Initialize work struct + handler function
    INIT_WORK(&my_work, work_handler);

    schedule_work(&my_work);

    pr_info("after schedule work");

    return 0;
}

static void __exit my_workqueue_module_exit(void)
{
    pr_info("Workqueue Module: Exiting...\n");

    flush_scheduled_work();
}

module_init(my_workqueue_module_init);
module_exit(my_workqueue_module_exit);
