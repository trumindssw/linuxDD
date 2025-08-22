#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");

static struct workqueue_struct *st_wq;
static struct work_struct my_work;

void my_work_function(struct work_struct *work)
{
    printk(KERN_INFO "Singlethreaded Workqueue: Work executed sequentially\n");
}

static int __init my_module_init(void)
{
    printk(KERN_INFO "Loading Singlethreaded Workqueue Example\n");

    // Create single-threaded workqueue
    st_wq = alloc_workqueue("st_wq", WQ_UNBOUND, 1);
    if (!st_wq)
        return -ENOMEM;

    INIT_WORK(&my_work, my_work_function);

    queue_work(st_wq, &my_work);

    return 0;
}

static void __exit my_module_exit(void)
{
    flush_workqueue(st_wq);
    destroy_workqueue(st_wq);
    printk(KERN_INFO "Unloading Singlethreaded Workqueue Example\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

