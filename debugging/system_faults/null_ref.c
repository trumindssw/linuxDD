#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int *ptr = NULL;

static int __init fault_init(void)
{
    printk(KERN_INFO "Triggering null pointer dereference...\n");

    *ptr = 42; // Null dereferencing 
    // we will see oops message

    return 0;
}

static void __exit fault_exit(void)
{
    printk(KERN_INFO "Fault module exiting\n");
}

module_init(fault_init);
module_exit(fault_exit);
MODULE_LICENSE("GPL");
