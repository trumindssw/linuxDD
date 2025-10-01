#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Radeen");
MODULE_DESCRIPTION("Avg Module depending on Add Module");
MODULE_VERSION("1.0");

//To tell the compiler that this function is coming from another module
extern int add_numbers(int x, int y);

static int __init avg_module_init(void)
{
    int a = 10, b = 20;
    int sum = add_numbers(a, b); // Use function from add_module
    int avg = sum / 2;

    pr_info("Avg Module Loaded\n");
    pr_info("a=%d, b=%d, sum=%d, avg=%d\n", a, b, sum, avg);

    return 0;
}

static void __exit avg_module_exit(void)
{
    pr_info("Avg Module Unloaded\n");
}

module_init(avg_module_init);
module_exit(avg_module_exit);
