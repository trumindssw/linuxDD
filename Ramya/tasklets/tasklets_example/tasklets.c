// tasklet_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>   // for tasklets
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ramya");
MODULE_DESCRIPTION("Simple Tasklet Example (Kernel 6.x compatible)");

// We want to pass some data (e.g., 1234) to the tasklet
static unsigned long my_data = 1234;

// Tasklet function (new signature in 6.x kernels)
static void my_tasklet_function(struct tasklet_struct *t)
{
	// Use container_of to recover our data
	unsigned long *data = &my_data;

	printk(KERN_INFO "Tasklet executed with data: %lu\n", *data);
}

// Declare the tasklet (no data argument in 6.x)
DECLARE_TASKLET(my_tasklet, my_tasklet_function);

static int __init my_init(void)
{
	printk(KERN_INFO "Module loaded, scheduling tasklet...\n");

	// Schedule the tasklet
	tasklet_schedule(&my_tasklet);

	return 0;
}

static void __exit my_exit(void)
{
	// Kill the tasklet if pending
	tasklet_kill(&my_tasklet);
	printk(KERN_INFO "Module unloaded, tasklet killed.\n");
}

module_init(my_init);
module_exit(my_exit);


