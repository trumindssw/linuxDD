#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
    pr_info("I am in init module...\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("I am in cleanup module...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Radeen");
MODULE_DESCRIPTION("Simple Hello Module");
