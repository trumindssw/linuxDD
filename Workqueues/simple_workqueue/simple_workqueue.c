#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Demo");
MODULE_DESCRIPTION("Simple Workqueue Example");

static struct workqueue_struct *my_wq;
static struct work_struct my_work;

/* The work function */
static void my_work_fn(struct work_struct *work)
{
    printk(KERN_INFO "Workqueue: Function started\n");

    // Unlike tasklets, we can sleep here
    msleep(2000);

    printk(KERN_INFO "Workqueue: Function finished after sleep\n");
}

static int __init my_module_init(void)
{
    printk(KERN_INFO "Module loaded, creating workqueue\n");

    my_wq = create_singlethread_workqueue("my_wq");
    if (!my_wq)
        return -ENOMEM;

    INIT_WORK(&my_work, my_work_fn);

    // Queue work
    queue_work(my_wq, &my_work);

    return 0;
}

static void __exit my_module_exit(void)
{
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
    printk(KERN_INFO "Module unloaded, workqueue destroyed\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

