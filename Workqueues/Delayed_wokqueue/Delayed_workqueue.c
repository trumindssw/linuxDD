#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ramya");
MODULE_DESCRIPTION("Delayed Workqueue Example");

static struct workqueue_struct *my_wq;
static struct delayed_work my_dwork;

static void my_delayed_fn(struct work_struct *work)
{
    printk(KERN_INFO "Delayed work executed after delay\n");
}

static int __init my_module_init(void)
{
    my_wq = create_singlethread_workqueue("my_wq");
    if (!my_wq)
        return -ENOMEM;

    INIT_DELAYED_WORK(&my_dwork, my_delayed_fn);

    printk(KERN_INFO "Queuing delayed work (5 seconds)\n");
    queue_delayed_work(my_wq, &my_dwork, 5 * HZ);

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

