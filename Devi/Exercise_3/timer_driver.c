#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/mutex.h>
#include <linux/ktime.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "timer_dev"
#define CLASS_NAME "timer_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Timer Driver with Workqueues and Spinlocks");

// IOCTL command definitions
#define MAGIC 'T'
#define SET_TIMER_VAL _IOW(MAGIC, 1, int)    // Set timer value in ms
#define ENABLE_TIMER _IO(MAGIC, 2)           // Enable timer functionality
#define STOP_TIMER _IO(MAGIC, 3)             // Stop timer functionality

// Global variables
static int major_number;                     // Major number for character device
static struct class *timer_class = NULL;     // Device class
static struct device *timer_device = NULL;   // Device instance
static struct cdev timer_cdev;               // Character device structure

// Driver state variables
static int timer_enabled = 0;                // Timer enabled flag
static int timer_val_ms = 1000;              // Timer wait value in milliseconds
static spinlock_t timer_spinlock;            // Spinlock for critical section
static struct mutex timer_mutex;             // Mutex for configuration
static struct hrtimer hr_timer;              // High-resolution timer
static struct workqueue_struct *timer_wq;    // Workqueue for deferred work
static struct work_struct timer_work;        // Work structure

// File operations prototypes
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static long device_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops = 
{
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
};

/**
 * HR Timer callback function
 */
static enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
	return HRTIMER_NORESTART;
}

/**
 * Workqueue function - Safe version without file operations
 */
static void timer_work_fn(struct work_struct *work)
{
	unsigned long flags;
	struct timespec64 now;

	printk(KERN_INFO "Workqueue started - acquiring spinlock\n");

	// Step i: Acquire spinlock with IRQ protection
	spin_lock_irqsave(&timer_spinlock, flags);

	// Step ii: Log current date time to kernel buffer (safe alternative to file write)
	ktime_get_real_ts64(&now);
	printk(KERN_INFO "TIMER_LOG: Kernel timestamp: %lld.%09ld\n", (long long)now.tv_sec, now.tv_nsec);

	printk(KERN_INFO "Holding spinlock, waiting for %d ms\n", timer_val_ms);

	// Step iii: Wait for SET_TIMER_VAL seconds using HR timer
	//wait_time = ktime_set(0, timer_val_ms * NSEC_PER_MSEC);
	//hrtimer_start(&hr_timer, wait_time, HRTIMER_MODE_REL);

	// Busy wait while holding spinlock (demonstrates spinlock protection)
	//while (hrtimer_active(&hr_timer)) {
	//cpu_relax();
	//}making kernel crash and stucked...

	// Step iv: Release spinlock
	spin_unlock_irqrestore(&timer_spinlock, flags);

	printk(KERN_INFO "Workqueue completed - spinlock released\n");

	// Step v: Demonstrate that userspace can't access during critical section
	// This is shown by the spinlock protection above
}

/**
 * Device open function
 */
static int device_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	printk(KERN_INFO "Device opened\n");
	return 0;
}

/**
 * Device release function
 */
static int device_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	printk(KERN_INFO "Device closed\n");
	return 0;
}

/**
 * IOCTL handling function
 */
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	int temp_val;

	//  mutex_lock(&timer_mutex);

	switch (cmd) 
	{
		case SET_TIMER_VAL:
			if (copy_from_user(&temp_val, (int *)arg, sizeof(int))) 
			{
				retval = -EFAULT;
			} 
			else if (temp_val < 100 || temp_val > 5000) 
			{
				retval = -EINVAL;
				printk(KERN_ERR "Invalid timer value: %d ms (100-5000)\n", temp_val);
			} 
			else 
			{
				timer_val_ms = temp_val;
				printk(KERN_INFO "Timer value set to: %d ms\n", timer_val_ms);
			}
			break;

		case ENABLE_TIMER:
			if (!timer_enabled) 
			{
				timer_enabled = 1;
				printk(KERN_INFO "Timer enabled, scheduling workqueue\n");
				queue_work(timer_wq, &timer_work);
			} 
			else 
			{
				printk(KERN_INFO "Timer already enabled - ignoring request\n");
			}
			break;

		case STOP_TIMER:
			timer_enabled = 0;
			printk(KERN_INFO "Timer stopped - future enables ignored\n");
			break;

		default:
			retval = -ENOTTY;
			printk(KERN_ERR "Unknown IOCTL command: %u\n", cmd);
			break;
	}

	//    mutex_unlock(&timer_mutex);
	return retval;
}

/**
 * Module initialization
 */
static int __init timer_driver_init(void)
{
	int retval;

	printk(KERN_INFO "=== Initializing Timer Driver ===\n");

	// Initialize synchronization primitives
	spin_lock_init(&timer_spinlock);
	mutex_init(&timer_mutex);

	// Initialize high-resolution timer
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &timer_callback;

	// Create single-threaded workqueue
	timer_wq = create_singlethread_workqueue("timer_workqueue");
	if (!timer_wq) 
	{
		printk(KERN_ERR "Failed to create workqueue\n");
		return -ENOMEM;
	}
	INIT_WORK(&timer_work, timer_work_fn);

	// Register character device
	retval = alloc_chrdev_region(&major_number, 0, 1, DEVICE_NAME);
	if (retval < 0) 
	{
		printk(KERN_ERR "Failed to allocate char device region\n");
		goto err_wq;
	}

	cdev_init(&timer_cdev, &fops);
	timer_cdev.owner = THIS_MODULE;

	retval = cdev_add(&timer_cdev, major_number, 1);
	if (retval < 0) 
	{
		printk(KERN_ERR "Failed to add cdev\n");
		goto err_region;
	}

	// Create device class
	timer_class = class_create(CLASS_NAME);
	if (IS_ERR(timer_class)) 
	{
		retval = PTR_ERR(timer_class);
		printk(KERN_ERR "Failed to create device class\n");
		goto err_cdev;
	}

	// Create device node
	timer_device = device_create(timer_class, NULL, major_number, NULL, DEVICE_NAME);
	if (IS_ERR(timer_device)) 
	{
		retval = PTR_ERR(timer_device);
		printk(KERN_ERR "Failed to create device\n");
		goto err_class;
	}

	printk(KERN_INFO "Timer driver loaded successfully with major number %d\n", major_number);
	printk(KERN_INFO "Device node: /dev/%s\n", DEVICE_NAME);
	return 0;

err_class:
	class_destroy(timer_class);
err_cdev:
	cdev_del(&timer_cdev);
err_region:
	unregister_chrdev_region(major_number, 1);
err_wq:
	destroy_workqueue(timer_wq);
	return retval;
}

/**
 * Module cleanup
 */
static void __exit timer_driver_exit(void)
{
	printk(KERN_INFO "=== Unloading Timer Driver ===\n");

	timer_enabled = 0;
	flush_workqueue(timer_wq);

	device_destroy(timer_class, major_number);
	class_destroy(timer_class);
	cdev_del(&timer_cdev);
	unregister_chrdev_region(major_number, 1);
	destroy_workqueue(timer_wq);
	hrtimer_cancel(&hr_timer);

	printk(KERN_INFO "Timer driver unloaded successfully\n");
}

module_init(timer_driver_init);
module_exit(timer_driver_exit);
