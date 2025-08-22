#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Demo");
MODULE_DESCRIPTION("Workqueue with Shared Data Example");

static struct workqueue_struct *my_wq;
static struct work_struct work1, work2;
static int counter = 0;
static spinlock_t counter_lock;

static void my_work_fn(struct work_struct *work)
{
    spin_lock(&counter_lock);
    counter++;
    printk(KERN_INFO "Workqueue: counter = %d\n", counter);
    spin_unlock(&counter_lock);
}

static int __init my_module_init(void)
{
    my_wq = alloc_workqueue("my_wq", WQ_UNBOUND, 0);
    spin_lock_init(&counter_lock);

    INIT_WORK(&work1, my_work_fn);
    INIT_WORK(&work2, my_work_fn);

    queue_work(my_wq, &work1);
    queue_work(my_wq, &work2);

    return 0;
}

static void __exit my_module_exit(void)
{
    flush_workqueue(my_wq);
    destroy_workqueue(my_wq);
    printk(KERN_INFO "Module unloaded, final counter=%d\n", counter);
}

module_init(my_module_init);
module_exit(my_module_exit);


