#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Radeen");
MODULE_DESCRIPTION("Add Module exporting add_numbers function");
MODULE_VERSION("1.0");

// Function
int add_numbers(int x, int y)
{
    return x + y;
}
EXPORT_SYMBOL(add_numbers);   // Make function available to other modules

static int __init add_module_init(void)
{
    pr_info("Add Module Loaded\n");
    return 0;
}

static void __exit add_module_exit(void)
{
    pr_info("Add Module Unloaded\n");
}

module_init(add_module_init);
module_exit(add_module_exit);
