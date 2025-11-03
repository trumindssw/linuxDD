#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Simple procfs example with read/write support");

#define PROC_NAME "my_proc_entry"
#define BUFFER_SIZE 64

static struct proc_dir_entry *my_proc_entry;
static int my_value = 0;
static char proc_buffer[BUFFER_SIZE];

// --- procfs read ---
static ssize_t my_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char temp[BUFFER_SIZE];
	int len;
	if (*ppos > 0)
	{
		return 0;
	}

	len = snprintf(temp, BUFFER_SIZE, "%d\n", my_value);
	pr_info("procfs: my_value read = %d\n", my_value);

	if (copy_to_user(buf, temp, len))
	{
		return -EFAULT;
	}

	*ppos = len;
	return len;
}

// --- procfs write ---
static ssize_t my_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if (count >= BUFFER_SIZE)
	{
		return -EINVAL;
	}

	if (copy_from_user(proc_buffer, buf, count))
	{
		return -EFAULT;
	}

	proc_buffer[count] = '\0';

	if (kstrtoint(proc_buffer, 10, &my_value) == 0) 
	{
		pr_info("procfs: my_value written = %d\n", my_value);
	} 
	else 
	{
		pr_info("procfs: invalid write (not an integer): %s\n", proc_buffer);
	}

	return count;
}

static const struct proc_ops my_proc_ops = {
	.proc_read  = my_proc_read,
	.proc_write = my_proc_write,
};

// --- Module init ---
static int __init my_proc_init(void)
{
	my_proc_entry = proc_create(PROC_NAME, 0666, NULL, &my_proc_ops);
	if (!my_proc_entry)
	{
		return -ENOMEM;
	}

	pr_info("procfs: /proc/%s created\n", PROC_NAME);
	return 0;
}

// --- Module exit ---
static void __exit my_proc_exit(void)
{
	proc_remove(my_proc_entry);
	pr_info("procfs: /proc/%s removed\n", PROC_NAME);
}

module_init(my_proc_init);
module_exit(my_proc_exit);

