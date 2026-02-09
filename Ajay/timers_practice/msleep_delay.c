// msleep_delay.c
#include <linux/module.h>
#include <linux/delay.h>

static int __init ms_init(void)
{
    pr_info("msleep: sleeping 2 seconds\n");
    msleep(2000);
    pr_info("msleep: woke up\n");
    return 0;
}

static void __exit ms_exit(void)
{
    pr_info("msleep: module exit\n");
}

module_init(ms_init);
module_exit(ms_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("msleep delay example");
