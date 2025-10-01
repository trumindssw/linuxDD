#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Radeen");
MODULE_DESCRIPTION("Simple Add Module using module_param()");
MODULE_VERSION("1.0");


static int a = 0;
static int b = 0;

// Register parameters with permission 0444 (read-only in sysfs)
module_param(a, int, 0444);
MODULE_PARM_DESC(a, "First integer value");

module_param(b, int, 0444);
MODULE_PARM_DESC(b, "Second integer value");

// Module init
static int __init add_module_init(void)
{
    int sum = a + b;
    pr_info("Add Module Loaded\n");
    pr_info("a = %d, b = %d, sum = %d\n", a, b, sum);
    return 0;
}

// Module exit
static void __exit add_module_exit(void)
{
    pr_info("Add Module Unloaded\n");
}

module_init(add_module_init);
module_exit(add_module_exit);
