#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anurag");
MODULE_DESCRIPTION("Spinlock examples in kernel");

static struct task_struct *thread1;
static struct task_struct *thread2;

static DEFINE_SPINLOCK(my_spinlock); 

static int shared_counter = 0;

static int thread_fn(void *data)
{
    int i;
    unsigned long flags;

    for (i = 0; i < 5; i++) {
        // Protect critical section with spinlock + interrupt save
        spin_lock_irqsave(&my_spinlock, flags);

        shared_counter++;
        printk(KERN_INFO "Thread %s incremented counter to %d\n",
               current->comm, shared_counter);

        spin_unlock_irqrestore(&my_spinlock, flags);

    }

    return 0;
}

static int __init spinlock_demo_init(void)
{
    printk(KERN_INFO "Spinlock example module loaded\n");

    thread1 = kthread_run(thread_fn, NULL, "spin_thread1");
    thread2 = kthread_run(thread_fn, NULL, "spin_thread2");

    return 0;
}

static void __exit spinlock_demo_exit(void)
{
    if (thread1)
        kthread_stop(thread1);
    if (thread2)
        kthread_stop(thread2);

    printk(KERN_INFO "Spinlock example module unloaded. Final counter = %d\n", shared_counter);
}

module_init(spinlock_demo_init);
module_exit(spinlock_demo_exit);
