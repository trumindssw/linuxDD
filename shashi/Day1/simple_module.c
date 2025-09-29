#include <linux/module.h>   // Needed for all kernel modules
#include <linux/kernel.h>   // Needed for KERN_INFO
#include <linux/init.h>     // Needed for macros like module_init & module_exit

//function executed when module is loaded

static int __init my_init(void) {
	printk(KERN_INFO "shashi is loading the module...\n");
	return 0;
}

//function executed when module is unloaded
static void __exit my_cleanup(void) {
	printk(KERN_INFO "shashi is unloading the module....\n");
}

module_init(my_init); //register init function
module_exit(my_cleanup); // registe cleanup function
		
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shashi");
MODULE_DESCRIPTION("sample init module loaded and unloaded code");

