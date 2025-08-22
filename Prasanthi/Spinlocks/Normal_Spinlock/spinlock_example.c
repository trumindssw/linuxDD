#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

static spinlock_t my_lock;        // our spinlock
static int counter = 0;           // shared variable
static struct task_struct *t1, *t2;

static int my_thread(void *data)
{
    int i;
    for (i = 0; i < 10; i++) {
        spin_lock(&my_lock);   //  lock before using counter
        counter++;
        spin_unlock(&my_lock); //  unlock after using counter
    }
    return 0;
}

static int __init my_init(void)
{
    spin_lock_init(&my_lock);     // initialize lock
    counter = 0;

    // create 2 kernel threads
    t1 = kthread_run(my_thread, NULL, "thread1");
    t2 = kthread_run(my_thread, NULL, "thread2");
    return 0;
}

static void __exit my_exit(void)
{
    pr_info("Final counter = %d\n", counter);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
