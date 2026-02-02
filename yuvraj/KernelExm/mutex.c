#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/delay.h>

static struct mutex my_mutex;

static int __init my_init(void)
{
    mutex_init(&my_mutex);

    mutex_lock(&my_mutex);
    printk(KERN_INFO "mutex locked\n");
    msleep(2000);
    printk(KERN_INFO "mutex unlocked\n");
    mutex_unlock(&my_mutex);

    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "mutex module exit\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
