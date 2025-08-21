// tasklet_example.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

// Correct callback prototype
static void tl_fn(struct tasklet_struct *t)
{
    pr_info("tasklet: running\n");
}

// Declare tasklet with new-style function
DECLARE_TASKLET(my_tasklet, tl_fn);

static int __init tl_init(void)
{
    pr_info("tasklet: scheduling\n");
    tasklet_schedule(&my_tasklet);
    return 0;
}

static void __exit tl_exit(void)
{
    tasklet_kill(&my_tasklet);
    pr_info("tasklet: killed\n");
}

module_init(tl_init);
module_exit(tl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajaypal");
MODULE_DESCRIPTION("Modern tasklet example for Linux kernel 6.14+");
