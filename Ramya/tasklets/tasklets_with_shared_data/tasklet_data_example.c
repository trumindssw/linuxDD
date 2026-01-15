// tasklet_data_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Tasklet with data example");

static int shared_counter = 0;

static void my_tasklet_function(struct tasklet_struct *t)
{
    shared_counter++;
    printk(KERN_INFO "Tasklet executed, counter = %d\n", shared_counter);
}

// Declare tasklet
DECLARE_TASKLET(my_tasklet, my_tasklet_function);

static int __init my_init(void)
{
    printk(KERN_INFO "Module loaded, scheduling tasklet multiple times...\n");

    tasklet_schedule(&my_tasklet);
    tasklet_schedule(&my_tasklet);
    tasklet_schedule(&my_tasklet);

    return 0;
}

static void __exit my_exit(void)
{
    tasklet_kill(&my_tasklet);
    printk(KERN_INFO "Module unloaded, final counter = %d\n", shared_counter);
}

module_init(my_init);
module_exit(my_exit);

