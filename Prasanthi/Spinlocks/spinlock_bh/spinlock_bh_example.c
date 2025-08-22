

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>  /* tasklet API */
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/spinlock.h>

static spinlock_t bh_lock;           /* shared lock */
static int shared_counter = 0;       /* shared data */
static struct task_struct *thr;      /* kernel thread */
static struct tasklet_struct my_tasklet;

/* -------------------------
 * Tasklet handler (softirq)
 * -------------------------
 * Runs in softirq context — cannot sleep.
 * Uses plain spin_lock() because softirq is already a "bottom half".
 */
static void my_tasklet_fn(unsigned long data)
{
    /* Softirq context: do minimal work only */
    spin_lock(&bh_lock);
    shared_counter++;
    pr_info("tasklet: counter = %d\n", shared_counter);
    spin_unlock(&bh_lock);
}

/* -------------------------
 * Kernel thread (process context)
 * -------------------------
 * Uses spin_lock_bh() to disable bottom halves on local CPU
 * while modifying the shared data, so the tasklet cannot run
 * on the same CPU and re-enter the lock.
 */
static int thr_fn(void *unused)
{
    while (!kthread_should_stop()) {
        /* protect critical section and prevent local bottom-halves */
        spin_lock_bh(&bh_lock);
        shared_counter++;
        pr_info("thread: counter = %d\n", shared_counter);
        spin_unlock_bh(&bh_lock);

        /* schedule tasklet to run soon (softirq context) */
        tasklet_schedule(&my_tasklet);

        /* sleep so output is readable */
        msleep(500);
    }
    return 0;
}

static int __init spinlock_bh_init(void)
{
    /* init lock & tasklet */
    spin_lock_init(&bh_lock);
    shared_counter = 0;

    tasklet_init(&my_tasklet, my_tasklet_fn, 0);

    /* start kernel thread */
    thr = kthread_run(thr_fn, NULL, "spin_bh_thread");
    if (IS_ERR(thr)) {
        pr_err("Failed to create thread\n");
        tasklet_kill(&my_tasklet);
        return PTR_ERR(thr);
    }

    pr_info("spinlock_bh_example loaded\n");
    return 0;
}

static void __exit spinlock_bh_exit(void)
{
    if (thr)
        kthread_stop(thr);

    /* ensure the tasklet is finished before unloading */
    tasklet_kill(&my_tasklet);

    pr_info("spinlock_bh_example unloaded, final counter = %d\n", shared_counter);
}

module_init(spinlock_bh_init);
module_exit(spinlock_bh_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Example: spin_lock_bh() with tasklet");

