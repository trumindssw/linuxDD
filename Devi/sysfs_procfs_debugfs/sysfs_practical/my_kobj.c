#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Valli");
MODULE_DESCRIPTION("Simple sysfs example with read/write support");

static struct kobject *my_kobj;
static int my_value = 0;

// --- sysfs show (read) ---
static ssize_t my_value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	pr_info("sysfs: my_value read = %d\n", my_value);
	return sprintf(buf, "%d\n", my_value);
}

// --- sysfs store (write) ---
static ssize_t my_value_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	ret = kstrtoint(buf, 10, &my_value);
	if (ret < 0)
	{
		return ret;
	}

	pr_info("sysfs: my_value written = %d\n", my_value);
	return count;
}

// Create sysfs attribute file: /sys/kernel/my_kobj/my_value
static struct kobj_attribute my_value_attr = __ATTR(my_value, 0664, my_value_show, my_value_store);

// Attribute list
static struct attribute *attrs[] = {
	&my_value_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

// --- Module init ---
static int __init my_kobj_init(void)
{
	int retval;

	// Create /sys/kernel/my_kobj
	my_kobj = kobject_create_and_add("my_kobj", kernel_kobj);
	if (!my_kobj)
	{
		return -ENOMEM;
	}

	// Create attribute files
	retval = sysfs_create_group(my_kobj, &attr_group);
	if (retval)
	{
		kobject_put(my_kobj);
	}

	pr_info("sysfs: /sys/kernel/my_kobj/my_value created\n");
	return retval;
}

// --- Module exit ---
static void __exit my_kobj_exit(void)
{
	kobject_put(my_kobj);
	pr_info("sysfs: /sys/kernel/my_kobj removed\n");
}

module_init(my_kobj_init);
module_exit(my_kobj_exit);

