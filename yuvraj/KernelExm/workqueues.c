#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static struct work_struct my_work;

static void work_fn(struct work_struct *work)
{
    printk(KERN_INFO "workqueue start\n");
    msleep(3000);
    printk(KERN_INFO "workqueue end\n");
}

static int __init my_init(void)
{
    INIT_WORK(&my_work, work_fn);
    schedule_work(&my_work);
    printk(KERN_INFO "workqueue module loaded\n");
    return 0;
}

static void __exit my_exit(void)
{
    cancel_work_sync(&my_work);
    printk(KERN_INFO "workqueue module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
