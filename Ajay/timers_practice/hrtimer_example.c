// hrtimer_example.c
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

static struct hrtimer hrt;
static ktime_t interval;

enum hrtimer_restart hrt_fn(struct hrtimer *t)
{
    pr_info("hrtimer: callback at %lld ns\n", ktime_to_ns(ktime_get()));
    hrtimer_forward_now(t, interval);
    return HRTIMER_RESTART;
}

static int __init hr_init(void)
{
    interval = ms_to_ktime(500);
    hrtimer_init(&hrt, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    hrt.function = hrt_fn;
    hrtimer_start(&hrt, interval, HRTIMER_MODE_REL);
    pr_info("hrtimer: started\n");
    return 0;
}

static void __exit hr_exit(void)
{
    hrtimer_cancel(&hrt);
    pr_info("hrtimer: canceled\n");
}

module_init(hr_init);
module_exit(hr_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("High-resolution timer example");
