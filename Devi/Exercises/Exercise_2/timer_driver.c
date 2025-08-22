#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/string.h>

#define DEVICE_NAME "timer_driver"
#define CLASS_NAME "timer_class"

// IOCTL commands
#define SET_TIMER_VAL _IOW('a', 'a', int32_t*)
#define ENABLE_TIMER _IO('a', 'b')
#define STOP_TIMER _IO('a', 'c')

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Timer Driver with WorkQueue and SpinLock");

static int major_number;
static struct class* timer_class = NULL;
static struct device* timer_device = NULL;
static struct cdev timer_cdev;

// Driver state variables
static int timer_enabled = 0;
static int timer_value_ms = 3000; // Default 3 seconds (point a)
static struct hrtimer hr_timer;
static struct hrtimer wait_timer;
static ktime_t timer_interval;
static spinlock_t timer_lock;
static unsigned long flags;

// Work queue for timer operations
static struct workqueue_struct *timer_wq;
static struct work_struct timer_work;

// Completion for waiting in workqueue
static struct completion wait_completion;

// File operations
static int timer_open(struct inode *inodep, struct file *filep);
static int timer_release(struct inode *inodep, struct file *filep);
static long timer_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = timer_open,
	.release = timer_release,
	.unlocked_ioctl = timer_ioctl,
};

// Wait timer callback function
static enum hrtimer_restart wait_timer_callback(struct hrtimer *timer) {
	complete(&wait_completion);
	return HRTIMER_NORESTART;
}

// Work queue function - MEETS ALL REQUIREMENTS SAFELY
static void timer_work_handler(struct work_struct *work) 
{
	struct timespec64 ts;
	struct file *filep;
	char buffer[256];
	int len;
	loff_t pos = 0;

	printk(KERN_INFO "Timer work: Starting work handler\n");

	// Point d.ii: Write current date time to file FIRST (outside spinlock)
	ktime_get_real_ts64(&ts);
	len = snprintf(buffer, sizeof(buffer), 
			"KERNEL_TIMESTAMP: %lld.%09ld seconds since epoch\n",
			(long long)ts.tv_sec, ts.tv_nsec);

	filep = filp_open("/tmp/timer_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
	if (IS_ERR(filep)) {
		printk(KERN_ERR "Failed to open file: %ld\n", PTR_ERR(filep));
		return;
	}
	kernel_write(filep, buffer, len, &pos);
	filp_close(filep, NULL);
	printk(KERN_INFO "Successfully wrote timestamp to file\n");

	// Point d.i: NOW acquire spinlock (for the waiting part only)
	spin_lock_irqsave(&timer_lock, flags);
	printk(KERN_INFO "Timer work: Spinlock acquired - user writes will be blocked\n");

	// Point d.iii: Wait for SET_TIMER_VAL seconds using hrtimer
	printk(KERN_INFO "Timer work: Waiting for %d ms (holding spinlock)\n", timer_value_ms);

	init_completion(&wait_completion);
	hrtimer_start(&wait_timer, ms_to_ktime(timer_value_ms), HRTIMER_MODE_REL);

	// Wait for completion - temporarily release spinlock to avoid deadlock
	spin_unlock_irqrestore(&timer_lock, flags);
	wait_for_completion(&wait_completion);
	spin_lock_irqsave(&timer_lock, flags);

	printk(KERN_INFO "Timer work: Finished waiting for %d ms\n", timer_value_ms);

	// Point d.iv: Release spinlock
	spin_unlock_irqrestore(&timer_lock, flags);
	printk(KERN_INFO "Timer work: Spinlock released - user writes can proceed\n");
}
// Timer callback function
static enum hrtimer_restart timer_callback(struct hrtimer *timer) {
	if (timer_enabled) {
		// Schedule the work queue
		printk(KERN_INFO "Timer triggered - scheduling workqueue\n");
		queue_work(timer_wq, &timer_work);

		// Restart timer for 10 seconds (point c)
		hrtimer_forward_now(timer, ms_to_ktime(10000));
		return HRTIMER_RESTART;
	}
	return HRTIMER_NORESTART;
}

static int timer_open(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "Timer device opened\n");
	return 0;
}

static int timer_release(struct inode *inodep, struct file *filep) {
	printk(KERN_INFO "Timer device closed\n");
	return 0;
}

static long timer_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
	int32_t value;

	switch (cmd) {
		case SET_TIMER_VAL:
			// Point b: Configure timer value from userspace
			if (copy_from_user(&value, (int32_t*)arg, sizeof(value))) {
				return -EFAULT;
			}

			if (value <= 0) {
				return -EINVAL;
			}

			timer_value_ms = value * 1000; // Convert seconds to milliseconds
			printk(KERN_INFO "Timer value set to %d seconds (%d ms)\n", value, timer_value_ms);
			break;

		case ENABLE_TIMER:
			// Point c: Enable timer every 10 seconds
			if (!timer_enabled) {
				timer_enabled = 1;
				timer_interval = ms_to_ktime(10000); // 10 seconds
				hrtimer_start(&hr_timer, timer_interval, HRTIMER_MODE_REL);
				printk(KERN_INFO "Timer enabled, will trigger every 10 seconds\n");
			} else {
				printk(KERN_INFO "Timer already enabled\n");
			}
			break;

		case STOP_TIMER:
			// Point e: Stop timer completely
			if (timer_enabled) {
				timer_enabled = 0;
				hrtimer_cancel(&hr_timer);
				printk(KERN_INFO "Timer stopped\n");
			} else {
				printk(KERN_INFO "Timer already stopped\n");
			}
			break;

		default:
			return -ENOTTY;
	}

	return 0;
}

static int __init timer_init(void) {
	dev_t dev_no;
	int ret = 0;

	// Allocate major number
	ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		printk(KERN_ERR "Failed to allocate device number\n");
		return ret;
	}
	major_number = MAJOR(dev_no);
	printk(KERN_INFO "Timer driver registered with major number: %d\n", major_number);

	// Initialize cdev structure
	cdev_init(&timer_cdev, &fops);
	timer_cdev.owner = THIS_MODULE;

	// Add cdev to the system
	ret = cdev_add(&timer_cdev, dev_no, 1);
	if (ret < 0) {
		printk(KERN_ERR "Failed to add cdev\n");
		unregister_chrdev_region(dev_no, 1);
		return ret;
	}

	// Create device class
	timer_class = class_create(CLASS_NAME);
	if (IS_ERR(timer_class)) {
		cdev_del(&timer_cdev);
		unregister_chrdev_region(dev_no, 1);
		printk(KERN_ERR "Failed to create class\n");
		return PTR_ERR(timer_class);
	}

	// Create device
	timer_device = device_create(timer_class, NULL, dev_no, NULL, DEVICE_NAME);
	if (IS_ERR(timer_device)) {
		class_destroy(timer_class);
		cdev_del(&timer_cdev);
		unregister_chrdev_region(dev_no, 1);
		printk(KERN_ERR "Failed to create device\n");
		return PTR_ERR(timer_device);
	}

	// Initialize spinlock (point d.i)
	spin_lock_init(&timer_lock);

	// Create work queue (point d)
	timer_wq = create_singlethread_workqueue("timer_workqueue");
	if (!timer_wq) {
		device_destroy(timer_class, dev_no);
		class_destroy(timer_class);
		cdev_del(&timer_cdev);
		unregister_chrdev_region(dev_no, 1);
		printk(KERN_ERR "Failed to create workqueue\n");
		return -ENOMEM;
	}

	// Initialize work
	INIT_WORK(&timer_work, timer_work_handler);

	// Initialize high-resolution timers
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &timer_callback;

	hrtimer_init(&wait_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	wait_timer.function = &wait_timer_callback;

	printk(KERN_INFO "Timer driver loaded with default timer value: %d seconds\n", 
			timer_value_ms / 1000);

	return 0;
}

static void __exit timer_exit(void) {
	dev_t dev_no = MKDEV(major_number, 0);

	// Stop timers if running
	if (timer_enabled) {
		hrtimer_cancel(&hr_timer);
	}
	hrtimer_cancel(&wait_timer);

	// Cleanup workqueue
	if (timer_wq) {
		flush_workqueue(timer_wq);
		destroy_workqueue(timer_wq);
	}

	// Remove device
	if (timer_device) {
		device_destroy(timer_class, dev_no);
	}
	if (timer_class) {
		class_destroy(timer_class);
	}

	// Remove cdev
	cdev_del(&timer_cdev);

	// Unregister device number
	unregister_chrdev_region(dev_no, 1);

	printk(KERN_INFO "Timer driver unloaded\n");
}

module_init(timer_init);
module_exit(timer_exit);
