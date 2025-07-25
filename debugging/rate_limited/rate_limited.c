#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/printk.h>

static int __init rate_limit_init(void)
{
    int i;

    printk(KERN_INFO "Loading rate limit test module...\n");

    for (i = 0; i < 100; i++) {
        if (printk_ratelimit()) {
            printk(KERN_INFO "Rate-limited printk message: %d\n", i);
        }
    }

    return 0;
}

static void __exit rate_limit_exit(void)
{
    printk(KERN_INFO "Exiting rate limit test module\n");
}

module_init(rate_limit_init);
module_exit(rate_limit_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION("Kernel module to test printk_ratelimit behavior");
