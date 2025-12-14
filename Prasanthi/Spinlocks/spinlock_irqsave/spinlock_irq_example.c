#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prasanthi");
MODULE_DESCRIPTION("Spinlock with irqsave Example");

static spinlock_t my_lock;        // spinlock
static int counter = 0;           // shared variable
static struct task_struct *thread;
static unsigned long irq_flags;   // to save interrupt state

// Dummy IRQ number (for testing only, not a real HW IRQ)
#define MY_FAKE_IRQ 1

// Interrupt handler
static irqreturn_t my_irq_handler(int irq, void *dev_id)
{
    spin_lock_irqsave(&my_lock, irq_flags);
    counter++;
    pr_info("Interrupt: counter = %d\n", counter);
    spin_unlock_irqrestore(&my_lock, irq_flags);
    return IRQ_HANDLED;
}

// Kernel thread function
static int my_thread_fn(void *data)
{
    while (!kthread_should_stop()) {
        spin_lock_irqsave(&my_lock, irq_flags);
        counter++;
        pr_info("Thread: counter = %d\n", counter);
        spin_unlock_irqrestore(&my_lock, irq_flags);

        msleep(1000); // sleep for 1 second
    }
    return 0;
}

// Module init
static int __init my_module_init(void)
{
    int ret;

    pr_info("Spinlock irqsave module loaded\n");
    spin_lock_init(&my_lock);

    // Request fake IRQ (won’t actually trigger unless you simulate)
    ret = request_irq(MY_FAKE_IRQ, my_irq_handler, IRQF_SHARED, "my_irq_handler", &my_lock);
    if (ret) {
        pr_err("Failed to request IRQ %d\n", MY_FAKE_IRQ);
        return ret;
    }

    // Start kernel thread
    thread = kthread_run(my_thread_fn, NULL, "my_thread");
    if (IS_ERR(thread)) {
        free_irq(MY_FAKE_IRQ, &my_lock);
        return PTR_ERR(thread);
    }

    return 0;
}

// Module exit
static void __exit my_module_exit(void)
{
    if (thread)
        kthread_stop(thread);

    free_irq(MY_FAKE_IRQ, &my_lock);
    pr_info("Spinlock irqsave module unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

