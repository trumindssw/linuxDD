#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "mdcdd"
#define NUM_DEVICES 4
#define BUFF_SIZE 1024

//structure for each device 
struct mychar_dev {
	char buffer[BUFF_SIZE];
	int size;
	struct cdev cdev;
};

static int num_dev = NUM_DEVICES;
static dev_t base_dev; //(major + first minor)
static struct mychar_dev my_devices[NUM_DEVICES]; // array of structure for devices

/* File operations */
static int my_open(struct inode *inode, struct file *file)
{
    int minor = iminor(inode);   // extract minor number
    struct mychar_dev *dev;

    if (minor >= NUM_DEVICES) {
        printk(KERN_INFO "%s: Invalid minor number %d\n", DEVICE_NAME, minor);
        return -ENODEV;
    }

    dev = &my_devices[minor];        // pick correct device
    file->private_data = dev;       

    printk(KERN_INFO "%s: Device with minor %d opened\n", DEVICE_NAME, minor);
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "%s: Device closed\n", DEVICE_NAME);
    return 0;
}

static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    struct mychar_dev *dev = file->private_data;

    if (*ppos >= dev->size)
        return 0;

    if (count > dev->size - *ppos)
        count = dev->size - *ppos;

    if (copy_to_user(buf, dev->buffer + *ppos, count))
        return -EFAULT;

    *ppos += count;
    printk(KERN_INFO "%s: Read %zu bytes from device\n", DEVICE_NAME, count);
    return count;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct mychar_dev *dev = file->private_data;

    if (count > BUFF_SIZE)
        count = BUFF_SIZE;

    if (copy_from_user(dev->buffer, buf, count))
        return -EFAULT;

    dev->size = count;
    *ppos = count;

    printk(KERN_INFO "%s: Wrote %zu bytes to device\n", DEVICE_NAME, count);
    return count;
}


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
};

//init function
static int __init multi_init(void)
{
	int ret;

	//minor number check 
	if(num_dev <=0 && num_dev >256)
	{
		 printk(KERN_ERR "%s: invalid num_devs %d (must be 1..256)\n", __func__, num_dev);
                 return -EINVAL;
	}

	//allocate major/minor number range
	ret = alloc_chrdev_region(&base_dev,0,num_dev,DEVICE_NAME);
	if(ret)
	{
		printk("%s: alloc_chrdev_region failed: %d\n", __func__, ret);
        	return ret;
	}

	//loop over all devices and initialize cdev and add to kernel
	for(int i=0;i<NUM_DEVICES;i++)
	{
		cdev_init(&my_devices[i].cdev, &fops);
		my_devices[i].cdev.owner = THIS_MODULE;

		ret = cdev_add(&my_devices[i].cdev, base_dev+i, 1);
		if (ret) {
         	   	printk(KERN_ERR "Failed to add cdev for device %d\n", i);
        	} else {
            		printk(KERN_INFO "%s: Registered device with major=%d, minor=%d\n", DEVICE_NAME, MAJOR(base_dev), MINOR(base_dev + i));
        	}
	}	

	printk(KERN_INFO "Multi-Device Char Driver Loaded (using iminor)\n");
    	return 0;
}

//exit function
static void __exit multi_exit(void)
{

	for(int i=0;i<NUM_DEVICES;i++)
	{
		cdev_del(&my_devices[i].cdev);
	}
	unregister_chrdev_region(base_dev, NUM_DEVICES);
	printk(KERN_INFO "Multi-Device Char Driver Unloaded\n");

}

module_init(multi_init);
module_exit(multi_exit);
