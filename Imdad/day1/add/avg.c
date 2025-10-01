#include <linux/init.h>    
#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/moduleparam.h> 

MODULE_LICENSE("GPL");   
MODULE_AUTHOR("You");    
MODULE_DESCRIPTION("Avg module which uses add_values exported by add module");
MODULE_VERSION("0.1");   

// Declare module parameters (default values: 12 and 8)
static int x = 12;
static int y = 8;

module_param(x, int, 0444);   // permissions: read-only from sysfs
MODULE_PARM_DESC(x, "First integer for avg module");
module_param(y, int, 0444);
MODULE_PARM_DESC(y, "Second integer for avg module");

extern int add_values(int a, int b);

static int __init avg_init(void)
{
    int sum = add_values(x, y);

    int avg = sum / 2;

    pr_info("avg module: x=%d y=%d sum=%d avg=%d\n", x, y, sum, avg);

    return 0;
}

static void __exit avg_exit(void)
{
    pr_info("avg module unloaded\n");
}

module_init(avg_init);
module_exit(avg_exit);
