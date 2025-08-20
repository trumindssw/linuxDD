#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>   // for tasklet API

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prasanthi");
MODULE_DESCRIPTION("Normal Tasklet Example");

// Tasklet handler function
static void my_tasklet_func(struct tasklet_struct *t)
{
    pr_info("Normal Tasklet executed!\n");
}

// Declare a tasklet (normal type)
DECLARE_TASKLET(my_tasklet, my_tasklet_func);

static int __init tasklet_example_init(void)
{
    pr_info("Module loaded: scheduling normal tasklet\n");
    tasklet_schedule(&my_tasklet);   // Schedule tasklet
    return 0;
}

static void __exit tasklet_example_exit(void)
{
    tasklet_kill(&my_tasklet);  // Ensure cleanup
    pr_info("Module unloaded\n");
}

module_init(tasklet_example_init);
module_exit(tasklet_example_exit);

