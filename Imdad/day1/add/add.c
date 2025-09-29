#include <linux/init.h>        // Needed for macros like __init and __exit
#include <linux/module.h>      // Needed by all kernel modules
#include <linux/kernel.h>      // Needed for pr_info(), printk(), etc.
#include <linux/moduleparam.h> // Needed for module_param() and parameter descriptions

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");                    
MODULE_DESCRIPTION("Add module with parameters");  
MODULE_VERSION("0.1");                  

static int a = 3;
static int b = 4;

// Define module parameters `a` and `b`
// 0444 = read-only from /sys/module/<modname>/parameters/
module_param(a, int, 0444);
MODULE_PARM_DESC(a, "First integer"); // Description for parameter `a`

module_param(b, int, 0444);
MODULE_PARM_DESC(b, "Second integer"); // Description for parameter `b`

int add_values(int x, int y)
{
    int s = x + y;
    pr_info("add_values(): %d + %d = %d\n", x, y, s);
    return s;
}

EXPORT_SYMBOL(add_values);

static int __init add_init(void)
{
    int s = add_values(a, b);

    pr_info("Add module loaded: parameters a=%d b=%d sum=%d\n", a, b, s);

    return 0; 
}

static void __exit add_exit(void)
{
    pr_info("Add module unloaded\n");
}

// Register the init and exit functions with the kernel
module_init(add_init);
module_exit(add_exit);
