#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

static spinlock_t my_lock;
static int shared_counter = 0;
static struct task_struct *my_thread;
static struct tasklet_struct my_tasklet;

static void my_tasklet_fn(unsigned long data)
{
    unsigned long flags;

    spin_lock_irqsave(&my_lock, flags);
    shared_counter++;
    pr_info("[Tasklet] Counter = %d\n", shared_counter);
    spin_unlock_irqrestore(&my_lock, flags);
}

// Kernel thread
static int my_thread_fn(void *data)
{
    unsigned long flags;

    while (!kthread_should_stop()) {
        // Take lock in process context
        spin_lock_irqsave(&my_lock, flags);
        shared_counter++;
        pr_info("[Thread] Counter = %d\n", shared_counter);
        spin_unlock_irqrestore(&my_lock, flags);

        // Schedule tasklet simulates IRQ firing
        tasklet_schedule(&my_tasklet);

        msleep(500);
    }
    return 0;
}

static int __init spinlock_irq_demo_init(void)
{
    spin_lock_init(&my_lock);
    tasklet_init(&my_tasklet, my_tasklet_fn, 0);

    pr_info("Spinlock IRQ demo loaded\n");

    my_thread = kthread_run(my_thread_fn, NULL, "my_lock_thread");
    if (IS_ERR(my_thread)) {
        pr_err("Failed to create thread\n");
        return PTR_ERR(my_thread);
    }

    return 0;
}

static void __exit spinlock_irq_demo_exit(void)
{
    if (my_thread)
        kthread_stop(my_thread);

    tasklet_kill(&my_tasklet);
    pr_info("Spinlock IRQ demo unloaded\n");
}

module_init(spinlock_irq_demo_init);
module_exit(spinlock_irq_demo_exit);

MODULE_LICENSE("GPL");
