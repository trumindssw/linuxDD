// add_module.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// Function to add two numbers
int add_integers(int a, int b)
{
    return a + b;
}
EXPORT_SYMBOL(add_integers);   // Export symbol so other modules can use it

static int __init add_init(void)
{
    printk(KERN_INFO "Add Module Loaded...\n");
    return 0;
}

static void __exit add_exit(void)
{
    printk(KERN_INFO "Add Module Unloaded...\n");
}

module_init(add_init);
module_exit(add_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Addition Module exporting add_integers()");

