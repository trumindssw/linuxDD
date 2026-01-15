// workqueue_example.c
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static struct workqueue_struct *wq;
static DECLARE_DELAYED_WORK(dw, NULL);

static void wq_fn(struct work_struct *work)
{
    pr_info("workqueue: work running\n");
}

static int __init wq_init(void)
{
    INIT_DELAYED_WORK(&dw, wq_fn);
    wq = create_singlethread_workqueue("demo_wq");
    pr_info("workqueue: scheduling after 1s\n");
    queue_delayed_work(wq, &dw, msecs_to_jiffies(1000));
    return 0;
}

static void __exit wq_exit(void)
{
    cancel_delayed_work_sync(&dw);
    destroy_workqueue(wq);
    pr_info("workqueue: destroyed\n");
}

module_init(wq_init);
module_exit(wq_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Delayed workqueue example");
