// spinlock_normal.c - Example of basic spin_lock and spin_unlock
#include <linux/module.h>   // For module init/exit macros
#include <linux/kernel.h>   // For printk and KERN_INFO
#include <linux/init.h>     // For __init and __exit macros
#include <linux/spinlock.h> // For spinlock_t and spinlock APIs
#include <linux/kthread.h>  // For creating kernel threads
#include <linux/delay.h>    // For msleep()

// Declare a spinlock
static spinlock_t my_lock;

// Shared resource (global variable) accessed by multiple threads
static int shared_counter;

// Two kernel thread pointers
static struct task_struct *thread1, *thread2;

// Thread function that increments shared_counter
static int thread_fn(void *data)
{
	int i;
	for (i = 0; i < 5; i++) {
		// Acquire spinlock (disables kernel preemption)
		spin_lock(&my_lock);

		// Critical section — only one thread at a time can enter here
		shared_counter++;
		pr_info("[NORMAL] %s: counter = %d\n", current->comm, shared_counter);

		// Release spinlock
		spin_unlock(&my_lock);

		// Sleep outside the spinlock (important: never sleep inside spinlock)
		msleep(300);
	}
	return 0; // Thread function ends
}

// Module initialization function
static int __init spinlock_normal_init(void)
{
	pr_info("=== Normal Spinlock Module Loaded ===\n");

	// Initialize spinlock
	spin_lock_init(&my_lock);

	// Reset shared_counter
	shared_counter = 0;

	// Create two kernel threads that will run concurrently
	thread1 = kthread_run(thread_fn, NULL, "thread1");
	thread2 = kthread_run(thread_fn, NULL, "thread2");

	return 0; // Success
}

// Module cleanup function
static void __exit spinlock_normal_exit(void)
{
	// Stop threads if they are running
	if (thread1) kthread_stop(thread1);
	if (thread2) kthread_stop(thread2);

	// Print final counter value
	pr_info("[NORMAL] Final counter value = %d\n", shared_counter);
	pr_info("=== Normal Spinlock Module Unloaded ===\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Basic Spinlock Example");
module_init(spinlock_normal_init);
module_exit(spinlock_normal_exit);

