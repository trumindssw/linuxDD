#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");

static struct workqueue_struct *my_wq;
static struct work_struct my_work;

void my_work_function(struct work_struct *work)
{
    printk(KERN_INFO "Custom Workqueue: Work executed\n");
}

static int __init my_module_init(void)
{
    printk(KERN_INFO "Loading Custom Workqueue Example\n");

    // Create custom workqueue
    my_wq = alloc_workqueue("my_wq", WQ_UNBOUND | WQ_HIGHPRI, 0);
    if (!my_wq)
        return -ENOMEM;

    INIT_WORK(&my_work, my_work_function);

    // Queue work to custom wq
    queue_work(my_wq, &my_work);

    return 0;
}

static void __exit my_module_exit(void)
{
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
    printk(KERN_INFO "Unloading Custom Workqueue Example\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

