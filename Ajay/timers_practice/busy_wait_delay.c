// busy_wait_delay.c
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

static int __init bw_init(void)
{
    unsigned long start = jiffies;
    unsigned long delay = msecs_to_jiffies(2000);  // 2 seconds

    pr_info("busy_wait: start jiffies=%lu\n", start);
    while (time_before(jiffies, start + delay))
        cpu_relax();
    pr_info("busy_wait: end jiffies=%lu\n", jiffies);
    return 0;
}

static void __exit bw_exit(void)
{
    pr_info("busy_wait: module exit\n");
}

module_init(bw_init);
module_exit(bw_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Busy-wait delay example");
