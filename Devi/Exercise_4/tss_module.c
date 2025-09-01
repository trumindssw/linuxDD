// tss_module.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/slab.h> // For kmalloc/kfree

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("TSS Proc Entry Module");

#define PROC_NAME "tss"
#define MAX_BUF_SIZE 256

static struct timer_list my_timer;
static unsigned long my_jiffies;
static int myProcesses;
static unsigned long my_new_jiffies;
static char my_string[256] = "Initial TSS String";

// Function to count running processes
static int count_processes(void)
{
	struct task_struct *task;
	int count = 0;

	rcu_read_lock();
	for_each_process(task) 
	{
		count++;
	}
	rcu_read_unlock();

	return count;
}

// Timer callback function
static void timer_callback(struct timer_list *t)
{
	// Update variables every second
	my_jiffies = jiffies;
	myProcesses = count_processes();

	// Restart timer
	mod_timer(&my_timer, jiffies + HZ);
}

// Proc read function
static int tss_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "my_jiffies: %lu\n", my_jiffies);
	seq_printf(m, "myProcesses: %d\n", myProcesses);
	seq_printf(m, "my_new_jiffies: %lu\n", my_new_jiffies);
	seq_printf(m, "my_string: %s\n", my_string);
	return 0;
}

// Proc write function
static ssize_t tss_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	char *buf = NULL;
	unsigned long temp_jiffies;
	ssize_t retval = count;

	// Allocate buffer dynamically to avoid large stack usage
	buf = kmalloc(MAX_BUF_SIZE, GFP_KERNEL);
	if (!buf) 
	{
		return -ENOMEM;
	}

	if (count > MAX_BUF_SIZE - 1)
	{
		count = MAX_BUF_SIZE - 1;
	}

	if (copy_from_user(buf, buffer, count)) 
	{
		retval = -EFAULT;
		goto cleanup;
	}

	buf[count] = '\0';

	// Check if writing to my_new_jiffies
	if (strncmp(buf, "my_new_jiffies=", 15) == 0) 
	{
		if (kstrtoul(buf + 15, 10, &temp_jiffies) == 0) 
		{
			my_new_jiffies = temp_jiffies;
		}
	}
	// Check if writing to my_string
	else if (strncmp(buf, "my_string=", 10) == 0) 
	{
		strncpy(my_string, buf + 10, sizeof(my_string) - 1);
		my_string[sizeof(my_string) - 1] = '\0';
	}

cleanup:
	kfree(buf);
	return retval;
}

// Proc open function
static int tss_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, tss_proc_show, NULL);
}

// Modern proc_ops structure
static const struct proc_ops tss_proc_ops = 
{
	.proc_open = tss_proc_open,
	.proc_read = seq_read,
	.proc_write = tss_proc_write,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

// Module initialization
static int __init tss_module_init(void)
{
	struct proc_dir_entry *entry;

	printk(KERN_INFO "TSS Module: Initializing\n");

	// Create proc entry
	entry = proc_create(PROC_NAME, 0666, NULL, &tss_proc_ops);
	if (!entry) 
	{
		printk(KERN_ERR "TSS Module: Failed to create /proc/%s\n", PROC_NAME);
		return -ENOMEM;
	}

	// Initialize variables
	my_jiffies = jiffies;
	myProcesses = count_processes();
	my_new_jiffies = 0;

	// Setup timer
	timer_setup(&my_timer, timer_callback, 0);
	mod_timer(&my_timer, jiffies + HZ);

	printk(KERN_INFO "TSS Module: /proc/%s created successfully\n", PROC_NAME);
	return 0;
}

// Module cleanup
static void __exit tss_module_exit(void)
{
	// Remove timer
	del_timer_sync(&my_timer);

	// Remove proc entry
	remove_proc_entry(PROC_NAME, NULL);

	printk(KERN_INFO "TSS Module: Removed /proc/%s\n", PROC_NAME);
}

module_init(tss_module_init);
module_exit(tss_module_exit);
