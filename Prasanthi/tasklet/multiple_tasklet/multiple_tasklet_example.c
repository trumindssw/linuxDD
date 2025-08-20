#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prasanthi");
MODULE_DESCRIPTION("Multiple Tasklets Example");

// Normal tasklet handler
static void normal_tasklet_func(struct tasklet_struct *t)
{
    pr_info("Normal Tasklet executed\n");
}

// High-priority tasklet handler
static void hi_tasklet_func(struct tasklet_struct *t)
{
    pr_info("High-Priority Tasklet executed\n");
}

DECLARE_TASKLET(normal_tasklet, normal_tasklet_func);
DECLARE_TASKLET(hi_tasklet, hi_tasklet_func);

static int __init multi_tasklet_init(void)
{
    pr_info("Module loaded: scheduling both tasklets\n");
    tasklet_schedule(&normal_tasklet);
    tasklet_hi_schedule(&hi_tasklet);
    return 0;
}

static void __exit multi_tasklet_exit(void)
{
    tasklet_kill(&normal_tasklet);
    tasklet_kill(&hi_tasklet);
    pr_info("Module unloaded\n");
}

module_init(multi_tasklet_init);
module_exit(multi_tasklet_exit);

