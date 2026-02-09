#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

static struct hrtimer my_timer;

static enum hrtimer_restart timer_fn(struct hrtimer *t)
{
    printk(KERN_INFO "hrtimer fired\n");
    return HRTIMER_NORESTART;
}

static int __init my_init(void)
{
    hrtimer_init(&my_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    my_timer.function = timer_fn;
    hrtimer_start(&my_timer, ktime_set(2, 0), HRTIMER_MODE_REL);
    printk(KERN_INFO "hrtimer module loaded\n");
    return 0;
}

static void __exit my_exit(void)
{
    hrtimer_cancel(&my_timer);
    printk(KERN_INFO "hrtimer module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
