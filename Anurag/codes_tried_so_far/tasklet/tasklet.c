
// basic tasklet scheduling 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anurag Example");
MODULE_DESCRIPTION("Dynamic Tasklet Scheduling Example");

struct tasklet_struct my_tasklet;

static void my_tasklet_function(unsigned long data)
{
    pr_info("Tasklet executed dynamically. Data = %lu\n", data);
}

static int __init tasklet_init_example(void)
{
    pr_info("Loading Dynamic Tasklet Module...\n");

    // init + scheduling
    tasklet_init(&my_tasklet, my_tasklet_function, 5678);

    tasklet_schedule(&my_tasklet);

    return 0;
}

static void __exit tasklet_exit_example(void)
{
    pr_info("Unloading Dynamic Tasklet Module...\n");

    tasklet_kill(&my_tasklet);
}

module_init(tasklet_init_example);
module_exit(tasklet_exit_example);
