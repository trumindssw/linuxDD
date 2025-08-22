#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prasanthi");
MODULE_DESCRIPTION("Workqueue using DECLARE_WORK example");
MODULE_VERSION("1.0");

// Work function definition
void my_work_function(struct work_struct *work)
{
    printk(KERN_INFO "Workqueue executed from my_work_function\n");
}

// Declare work statically
DECLARE_WORK(my_work, my_work_function);

// Simulate a handler (e.g., from interrupt or any event)
static void some_handler(void)
{
    printk(KERN_INFO "some_handler called, scheduling work\n");
    schedule_work(&my_work);  // Queue the work for execution
}

static int __init my_module_init(void)
{
    printk(KERN_INFO "Workqueue module loaded\n");

    // Simulate an event triggering the handler
    some_handler();

    return 0;
}
static void __exit my_module_exit(void)
{
    // Ensure work is completed before exit
    flush_scheduled_work();
    printk(KERN_INFO "Workqueue module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

