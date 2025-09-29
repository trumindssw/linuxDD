#include<linux/kernel.h>
#include<linux/module.h>

MODULE_LICENSE("GPL");

int add(int a, int b)
{
	return a+b;
}

EXPORT_SYMBOL(add);

static int __init add_init(void)
{
	printk(KERN_INFO "%s: add module loaded", __func__);
	return 0;
}

static void __exit add_exit(void)
{
	printk(KERN_INFO "%s: add module unloaded", __func__);
}

module_init(add_init);
module_exit(add_exit);
