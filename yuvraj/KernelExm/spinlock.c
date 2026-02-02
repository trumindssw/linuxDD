#include <linux/module.h>
#include <linux/spinlock.h>

static spinlock_t my_lock;
static int shared;

static int __init my_init(void)
{
    spin_lock_init(&my_lock);

    spin_lock(&my_lock);
    shared++;
    printk(KERN_INFO "spinlock value=%d\n", shared);
    spin_unlock(&my_lock);

    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "spinlock module exit\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
