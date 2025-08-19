// mixture_demo_fixed.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>   // for tasklets
#include <linux/workqueue.h>   // for workqueues
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anurag Example");
MODULE_DESCRIPTION("Mixture of Tasklets, Workqueues, Spinlocks, and Mutexes");

// ---------- Shared Data ----------
static int shared_counter = 0;
static spinlock_t my_spinlock;
static DEFINE_MUTEX(my_mutex);

// ---------- Tasklet ----------
static void my_tasklet_func(struct tasklet_struct *t);

DECLARE_TASKLET(my_tasklet, my_tasklet_func);

static void my_tasklet_func(struct tasklet_struct *t)
{
    unsigned long flags;

    pr_info("Tasklet: running (softirq context)\n");

    spin_lock_irqsave(&my_spinlock, flags);
    shared_counter++;
    pr_info("Tasklet: updated shared_counter = %d\n", shared_counter);
    spin_unlock_irqrestore(&my_spinlock, flags);
}

// ---------- Workqueue ----------
static struct work_struct my_work;

static void my_work_func(struct work_struct *work)
{
    pr_info("Workqueue: running (process context)\n");

    mutex_lock(&my_mutex);
    msleep(500); // simulate delay
    shared_counter += 10;
    pr_info("Workqueue: updated shared_counter = %d\n", shared_counter);
    mutex_unlock(&my_mutex);
}

// ---------- Module Init/Exit ----------
static int __init mixture_init(void)
{
    pr_info("Loading Mixture Demo Module...\n");

    spin_lock_init(&my_spinlock);

    INIT_WORK(&my_work, my_work_func);

    tasklet_schedule(&my_tasklet);
    schedule_work(&my_work);

    return 0;
}

static void __exit mixture_exit(void)
{
    pr_info("Unloading Mixture Demo Module...\n");

    tasklet_kill(&my_tasklet);
    flush_work(&my_work);  // ✅ flush only our work
}

module_init(mixture_init);
module_exit(mixture_exit);
