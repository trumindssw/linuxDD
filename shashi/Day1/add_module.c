#include <linux/module.h>   // Needed for all modules
#include <linux/kernel.h>   // Needed for KERN_INFO
#include <linux/init.h>     // Needed for macros
#include <linux/moduleparam.h> // Needed for module_param()

// Declare parameters with default values
static int a = 0;
static int b = 0;

// Define module parameters (int, permissions)
module_param(a, int, 0644);
MODULE_PARM_DESC(a, "First integer value");

module_param(b, int, 0644);
MODULE_PARM_DESC(b, "Second integer value");

// Init function
static int __init add_init(void)
{
    int sum = a + b;
    printk(KERN_INFO "Add Module Loaded...\n");
    printk(KERN_INFO "Parameters: a=%d, b=%d\n", a, b);
    printk(KERN_INFO "Sum = %d\n", sum);
    return 0;
}

// Exit function
static void __exit add_exit(void)
{
    printk(KERN_INFO "Add Module Unloaded...\n");
}

module_init(add_init);
module_exit(add_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Add Module with Parameters");

