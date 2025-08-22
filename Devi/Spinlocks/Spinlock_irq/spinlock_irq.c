// spinlock_irq.c - Example of spin_lock_irqsave and spin_unlock_irqrestore
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static spinlock_t my_lock;
static int shared_counter;
static struct task_struct *thread1, *thread2;

static int thread_fn(void *data)
{
	int i;
	unsigned long flags; // Used to save CPU's interrupt state

	for (i = 0; i < 5; i++) {
		// Acquire spinlock and disable local CPU interrupts
		// Saves previous interrupt state in 'flags'
		spin_lock_irqsave(&my_lock, flags);

		shared_counter++;
		pr_info("[IRQ] %s: counter = %d (IRQs OFF)\n", current->comm, shared_counter);

		// Release spinlock and restore saved interrupt state
		spin_unlock_irqrestore(&my_lock, flags);

		msleep(300);
	}
	return 0;
}

static int __init spinlock_irq_init(void)
{
	pr_info("=== IRQ-Safe Spinlock Module Loaded ===\n");
	spin_lock_init(&my_lock);
	shared_counter = 0;
	thread1 = kthread_run(thread_fn, NULL, "thread1");
	thread2 = kthread_run(thread_fn, NULL, "thread2");
	return 0;
}

static void __exit spinlock_irq_exit(void)
{
	if (thread1) kthread_stop(thread1);
	if (thread2) kthread_stop(thread2);

	pr_info("[IRQ] Final counter value = %d\n", shared_counter);
	pr_info("=== IRQ-Safe Spinlock Module Unloaded ===\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Spinlock Example with IRQ Disable");
module_init(spinlock_irq_init);
module_exit(spinlock_irq_exit);

