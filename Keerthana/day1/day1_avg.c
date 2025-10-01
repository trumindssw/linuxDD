#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");

//declare function from day1_add.c
extern int add(int a, int b);

static int __init avg_init(void)
{
    int sum = add(10, 20);
    int avg = sum / 2;
    printk(KERN_INFO "%s: Avg Module Loaded\n", __func__);
    printk(KERN_INFO "%s: Sum = %d, Average = %d\n", __func__, sum, avg);
    return 0;
}

static void __exit avg_exit(void)
{
    printk(KERN_INFO "Avg Module Unloaded\n");
}

module_init(avg_init);
module_exit(avg_exit);
