// avg_module.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

// Declare the function from add_module (definition provided there)
extern int add_integers(int a, int b);

static int __init avg_init(void)
{
    int a = 10, b = 20;
    int sum = add_integers(a, b);   // Call function from Add Module
    int avg = sum / 2;

    printk(KERN_INFO "Avg Module Loaded...\n");
    printk(KERN_INFO "Numbers: %d, %d\n", a, b);
    printk(KERN_INFO "Sum = %d, Avg = %d\n", sum, avg);
    return 0;
}

static void __exit avg_exit(void)
{
    printk(KERN_INFO "Avg Module Unloaded...\n");
}

module_init(avg_init);
module_exit(avg_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Average Module dependent on Add Module");

