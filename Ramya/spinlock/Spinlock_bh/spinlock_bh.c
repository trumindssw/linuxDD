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
	for (i = 0; i < 5; i++) {
		// Acquire spinlock and disable bottom halves (softirqs/tasklets)
		spin_lock_bh(&my_lock);

		shared_counter++;
		pr_info("[BH] %s: counter = %d (BHs OFF)\n", current->comm, shared_counter);

		// Release spinlock and re-enable bottom halves
		spin_unlock_bh(&my_lock);

		msleep(300);
	}
	return 0;
}

static int __init spinlock_bh_init(void)
{
	pr_info("=== Bottom-Half-Safe Spinlock Module Loaded ===\n");
	spin_lock_init(&my_lock);
	shared_counter = 0;
	thread1 = kthread_run(thread_fn, NULL, "thread1");
	thread2 = kthread_run(thread_fn, NULL, "thread2");
	return 0;
}

static void __exit spinlock_bh_exit(void)
{
	if (thread1) kthread_stop(thread1);
	if (thread2) kthread_stop(thread2);

	pr_info("[BH] Final counter value = %d\n", shared_counter);
	pr_info("=== Bottom-Half-Safe Spinlock Module Unloaded ===\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Spinlock Example for Bottom Halves");
module_init(spinlock_bh_init);
module_exit(spinlock_bh_exit);


