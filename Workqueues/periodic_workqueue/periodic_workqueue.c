#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ramya");
MODULE_DESCRIPTION("Periodic Workqueue Example");

static struct workqueue_struct *my_wq;
static struct delayed_work my_dwork;

static void my_periodic_fn(struct work_struct *work)
{
    printk(KERN_INFO "Periodic work executed at jiffies=%lu\n", jiffies);

    /* Requeue itself after 5 seconds */
    queue_delayed_work(my_wq, &my_dwork, 5*HZ);
}

static int __init my_module_init(void)
{
    my_wq = create_singlethread_workqueue("my_wq");
    INIT_DELAYED_WORK(&my_dwork, my_periodic_fn);

    printk(KERN_INFO "Starting periodic work every 5 sec\n");
    queue_delayed_work(my_wq, &my_dwork, 5*HZ);

    return 0;
}

static void __exit my_module_exit(void)
{
    cancel_delayed_work(&my_dwork);
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
    printk(KERN_INFO "Module exit, cleaned up\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

