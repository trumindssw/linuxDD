// tasklet_irq_example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define MY_FAKE_IRQ 1   // Example IRQ number (for demo, not real)

static void my_tasklet_fn(struct tasklet_struct *t)
{
	printk(KERN_INFO "Tasklet executed after interrupt!\n");
}

DECLARE_TASKLET(my_tasklet, my_tasklet_fn);

static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
	printk(KERN_INFO "Interrupt occurred, scheduling tasklet...\n");
	tasklet_schedule(&my_tasklet);
	return IRQ_HANDLED;
}

static int __init my_init(void)
{
	int ret;

	ret = request_irq(MY_FAKE_IRQ, my_irq_handler, IRQF_SHARED, "my_irq_handler", &my_tasklet);
	if (ret) {
		pr_err("Failed to request IRQ %d, error %d\n", MY_FAKE_IRQ, ret);
		return ret;
	}

	pr_info("Module loaded, IRQ handler registered\n");
	return 0;
}

static void __exit my_exit(void)
{
	free_irq(MY_FAKE_IRQ, &my_tasklet);
	tasklet_kill(&my_tasklet);
	printk(KERN_INFO "Module exit, IRQ freed and tasklet killed.\n");
}

module_init(my_init);
module_exit(my_exit);


