#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Keerthana");
MODULE_DESCRIPTION("Day-1 assigment on basic char driver");

//declare parameters
int a=0;
int b=0;

//module parameters
module_param(a,int,0);
MODULE_PARM_DESC(a, "first integer");
module_param(b,int,0);
MODULE_PARM_DESC(b, "second integer");

static int __init fun_init(void)
{
	int result = a+b;
	printk(KERN_INFO "%s: I am in init module...\n", __func__);
	printk(KERN_INFO "%s: parameters: a=%d, b=%d\n", __func__, a,b);
	printk(KERN_INFO "%s: result = %d\n", __func__,result);
	return 0;
}

static void __exit fun_exit(void)
{
	printk(KERN_INFO "%s: I am in cleanup module...\n", __func__);
}

module_init(fun_init);
module_exit(fun_exit);
