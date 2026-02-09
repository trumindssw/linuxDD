// kernel_timer.c
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

static struct timer_list ktimer;

static void ktimer_fn(struct timer_list *t)
{
    pr_info("ktimer: timer expired at jiffies=%lu\n", jiffies);
    mod_timer(&ktimer, jiffies + msecs_to_jiffies(1000));
}

static int __init kt_init(void)
{
    timer_setup(&ktimer, ktimer_fn, 0);
    mod_timer(&ktimer, jiffies + msecs_to_jiffies(1000));
    pr_info("ktimer: started\n");
    return 0;
}

static void __exit kt_exit(void)
{
    del_timer_sync(&ktimer);
    pr_info("ktimer: canceled\n");
}

module_init(kt_init);
module_exit(kt_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel timer example");
