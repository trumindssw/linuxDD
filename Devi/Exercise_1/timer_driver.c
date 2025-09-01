#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define DEVICE_NAME "timer_device"
#define CLASS_NAME "timer_class"

// IOCTL commands
#define SET_TIMER_VAL _IOW('T', 1, int)
#define ENABLE_TIMER _IO('T', 2)
#define STOP_TIMER _IO('T', 3)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Timer Driver with WorkQueue and SpinLock");

// Device structure to hold all driver data
struct timer_dev 
{
	struct cdev cdev;
	dev_t devno;
	struct class *class;
	struct device *device;

	// Timer and workqueue components
	struct timer_list kernel_timer;
	struct work_struct timer_work;
	spinlock_t lock;

	// Configuration
	int timer_interval;    // Sleep duration (1 or 2 seconds)
	bool timer_enabled;    // Timer state
	bool timer_active;     // Whether timer is currently running

	// File for logging
	struct file *log_file;
};

static struct timer_dev timer_device;

// Workqueue handler function
static void timer_work_handler(struct work_struct *work)
{
	struct timespec64 current_time;
	char time_str[128];
	loff_t pos = 0;

	printk(KERN_INFO "Timer workqueue started\n");

	// i. Acquire spinlock
	spin_lock(&timer_device.lock);
	printk(KERN_INFO "Spinlock acquired\n");

	// ii. Write current date time to file
	ktime_get_real_ts64(&current_time);
	snprintf(time_str, sizeof(time_str), "[%lld.%09ld] Kernel: Timer triggered - entering %d second sleep\n", (long long)current_time.tv_sec, current_time.tv_nsec, timer_device.timer_interval);

	// Write to kernel log
	printk(KERN_INFO "%s", time_str);

	// Write to file (if log file is available)
	if (timer_device.log_file && !IS_ERR(timer_device.log_file)) 
	{
		kernel_write(timer_device.log_file, time_str, strlen(time_str), &pos);
	}

	// iii. Wait for SET_TIMER_VAL seconds using high-resolution timer
	// This demonstrates the blocking behavior while holding the spinlock
	printk(KERN_INFO "Starting %d second sleep while holding spinlock...\n", timer_device.timer_interval);

	// Use msleep for sleeping
	msleep(timer_device.timer_interval * 1000);

	// Get time after sleep
	ktime_get_real_ts64(&current_time);
	snprintf(time_str, sizeof(time_str), "[%lld.%09ld] Kernel: Sleep completed - releasing spinlock\n", (long long)current_time.tv_sec, current_time.tv_nsec);

	printk(KERN_INFO "%s", time_str);

	// Write completion message to file
	if (timer_device.log_file && !IS_ERR(timer_device.log_file)) 
	{
		kernel_write(timer_device.log_file, time_str, strlen(time_str), &pos);
	}

	// iv. Release spinlock
	spin_unlock(&timer_device.lock);
	printk(KERN_INFO "Spinlock released\n");
}

// Timer callback function (runs in interrupt context)
static void timer_callback(struct timer_list *t)
{
	printk(KERN_INFO "Timer callback triggered\n");

	if (timer_device.timer_enabled && timer_device.timer_active) 
	{
		// Schedule the workqueue (runs in process context)
		schedule_work(&timer_device.timer_work);

		// Restart timer for 10 seconds
		mod_timer(&timer_device.kernel_timer, jiffies + msecs_to_jiffies(10000));
		printk(KERN_INFO "Timer rescheduled for 10 seconds from now\n");
	} 
	else 
	{
		printk(KERN_INFO "Timer not enabled or active, not scheduling work\n");
	}
}

// File operations
static int device_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device opened\n");
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device closed\n");
	return 0;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) 
	{
		case SET_TIMER_VAL:
			// Set the sleep interval (1 or 2 seconds)
			if (arg == 1 || arg == 2) 
			{
				timer_device.timer_interval = (int)arg;
				printk(KERN_INFO "Timer sleep interval set to %d seconds\n", (int)arg);
			} 
			else 
			{
				printk(KERN_WARNING "Invalid timer value: %lu (must be 1 or 2)\n", arg);
				ret = -EINVAL;
			}
			break;

		case ENABLE_TIMER:
			// Enable the timer if not already enabled
			if (!timer_device.timer_enabled) 
			{
				timer_device.timer_enabled = true;
				timer_device.timer_active = true;

				// Initialize and start the timer (10 second interval)
				timer_setup(&timer_device.kernel_timer, timer_callback, 0);
				mod_timer(&timer_device.kernel_timer, jiffies + msecs_to_jiffies(10000));

				printk(KERN_INFO "Timer enabled - will trigger every 10 seconds\n");
			} 
			else 
			{
				printk(KERN_INFO "Timer already enabled\n");
			}
			break;

		case STOP_TIMER:
			// Stop the timer
			timer_device.timer_enabled = false;
			timer_device.timer_active = false;
			del_timer_sync(&timer_device.kernel_timer);
			printk(KERN_INFO "Timer stopped - no further triggers will occur\n");
			break;

		default:
			printk(KERN_WARNING "Unknown IOCTL command: %u\n", cmd);
			return -ENOTTY;
	}

	return ret;
}

// File operations structure
static const struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
};

// Module initialization
static int __init timer_module_init(void)
{
	int ret = 0;

	printk(KERN_INFO "Timer driver module initializing\n");

	// Initialize spinlock
	spin_lock_init(&timer_device.lock);

	// Initialize workqueue
	INIT_WORK(&timer_device.timer_work, timer_work_handler);

	// Initialize default values
	timer_device.timer_interval = 1;  // default 1 second
	timer_device.timer_enabled = false;
	timer_device.timer_active = false;
	timer_device.log_file = NULL;
	timer_device.class = NULL;
	timer_device.device = NULL;

	// Try to open log file (optional)
	timer_device.log_file = filp_open("/tmp/timer_driver.log", O_WRONLY|O_CREAT|O_APPEND, 0644);
	if (IS_ERR(timer_device.log_file)) 
	{
		printk(KERN_WARNING "Failed to open log file, using kernel log only\n");
		timer_device.log_file = NULL;
	} 
	else 
	{
		printk(KERN_INFO "Log file opened successfully\n");
	}

	// Allocate device number
	ret = alloc_chrdev_region(&timer_device.devno, 0, 1, DEVICE_NAME);
	if (ret < 0) 
	{
		printk(KERN_ERR "Failed to allocate device number\n");
		goto cleanup;
	}

	// Create device class - use the new API for newer kernels
	timer_device.class = class_create(CLASS_NAME);
	if (IS_ERR(timer_device.class)) 
	{
		printk(KERN_ERR "Failed to create device class\n");
		ret = PTR_ERR(timer_device.class);
		goto cleanup;
	}

	// Initialize cdev structure
	cdev_init(&timer_device.cdev, &fops);
	timer_device.cdev.owner = THIS_MODULE;

	// Add character device
	ret = cdev_add(&timer_device.cdev, timer_device.devno, 1);
	if (ret < 0) 
	{
		printk(KERN_ERR "Failed to add character device\n");
		goto cleanup;
	}

	// Create device node
	timer_device.device = device_create(timer_device.class, NULL, timer_device.devno, NULL, DEVICE_NAME);
	if (IS_ERR(timer_device.device)) 
	{
		printk(KERN_ERR "Failed to create device node\n");
		ret = PTR_ERR(timer_device.device);
		goto cleanup;
	}

	printk(KERN_INFO "Timer driver module loaded successfully\n");
	printk(KERN_INFO "Device created: /dev/%s\n", DEVICE_NAME);
	printk(KERN_INFO "Use IOCTL commands: SET_TIMER_VAL(1/2), ENABLE_TIMER, STOP_TIMER\n");

	return 0;

cleanup:
	// Cleanup resources in reverse order
	if (timer_device.device && !IS_ERR(timer_device.device)) 
	{
		device_destroy(timer_device.class, timer_device.devno);
	}
	if (timer_device.cdev.ops) 
	{
		cdev_del(&timer_device.cdev);
	}
	if (timer_device.class && !IS_ERR(timer_device.class)) 
	{
		class_destroy(timer_device.class);
	}
	if (timer_device.devno) 
	{
		unregister_chrdev_region(timer_device.devno, 1);
	}
	if (timer_device.log_file && !IS_ERR(timer_device.log_file)) 
	{
		filp_close(timer_device.log_file, NULL);
	}

	return ret;
}

// Module cleanup
static void __exit timer_module_exit(void)
{
	// Stop timer if running
	if (timer_device.timer_active) 
	{
		del_timer_sync(&timer_device.kernel_timer);
	}

	// Cancel any pending work
	cancel_work_sync(&timer_device.timer_work);

	// Cleanup device
	if (timer_device.device && !IS_ERR(timer_device.device)) 
	{
		device_destroy(timer_device.class, timer_device.devno);
	}
	if (timer_device.cdev.ops) 
	{
		cdev_del(&timer_device.cdev);
	}
	if (timer_device.class && !IS_ERR(timer_device.class)) 
	{
		class_destroy(timer_device.class);
	}
	if (timer_device.devno) 
	{
		unregister_chrdev_region(timer_device.devno, 1);
	}

	// Close log file if open
	if (timer_device.log_file && !IS_ERR(timer_device.log_file)) 
	{
		filp_close(timer_device.log_file, NULL);
	}

	printk(KERN_INFO "Timer driver module unloaded\n");
}

module_init(timer_module_init);
module_exit(timer_module_exit);
