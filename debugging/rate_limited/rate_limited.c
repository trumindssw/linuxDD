#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init rate_limit_init(void)
{
    int i;

    for (i = 0; i < 100; i++) {
        printk_ratelimited(KERN_INFO "Rate-limited printk message: %d\n", i);
    }

    return 0;
}

static void __exit rate_limit_exit(void)
{
    printk(KERN_INFO "Exiting rate limit test\n");
}

module_init(rate_limit_init);
module_exit(rate_limit_exit);
MODULE_LICENSE("GPL");
