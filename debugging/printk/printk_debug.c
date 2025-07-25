#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Imdad");
MODULE_DESCRIPTION("Example module using printk");

static int __init hello_init(void)
{
    printk(KERN_EMERG "LOGLEVEL 0: EMERGENCY\n");
    printk(KERN_ALERT "LOGLEVEL 1: ALERT\n");
    printk(KERN_CRIT "LOGLEVEL 2: CRITICAL\n");
    printk(KERN_ERR "LOGLEVEL 3: ERROR\n");
    printk(KERN_WARNING "LOGLEVEL 4: WARNING\n");
    printk(KERN_NOTICE "LOGLEVEL 5: NOTICE\n");
    printk(KERN_INFO "LOGLEVEL 6: INFO\n");
    printk(KERN_DEBUG "LOGLEVEL 7: DEBUG\n");

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Exiting printk test module\n");
}

module_init(hello_init);
module_exit(hello_exit);
