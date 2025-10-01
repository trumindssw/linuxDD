#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>
#include<linux/slab.h>

MODULE_LICENSE("GPL");

//device info
#define DEVICE_NAME "dcdd"
#define BUFFER_SIZE 1024

static dev_t dev_num;  //device number (major+minor)
static struct cdev my_cdev; //cdev structure
static char *device_buffer; //kernel buffer
static int open_count=0; 

//open function
static int dyndrv_open(struct inode *inode, struct file *file)
{
	open_count++;
	printk(KERN_INFO "device opened %d times\n",open_count);
	return 0;
}

//release function
static int dyndrv_release(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Device closed\n");
	return 0;
}

//read function
static ssize_t dyndrv_read(struct file *file, char __user *buf, size_t len, loff_t *offset )
{
	int bytes_to_copy;
	if(*offset >= BUFFER_SIZE)
	{
		return 0; //EOF
	}

	bytes_to_copy =  min((int)len, BUFFER_SIZE-(int)*offset);
	if(copy_to_user(buf,device_buffer+ *offset, bytes_to_copy))
	{
		return -EFAULT;
	}

	*offset += bytes_to_copy;
	printk(KERN_INFO "%s: Read %d bytes, offset = %lld\n", __func__, bytes_to_copy, *offset);
	return bytes_to_copy;
}

//write function
static ssize_t dyndrv_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
	int bytes_to_copy;
	if(*offset >= BUFFER_SIZE)
	{
		return -ENOMEM;
	}

	bytes_to_copy = min((int)len, BUFFER_SIZE - (int)*offset);
	if(copy_from_user(device_buffer + *offset,  buf, bytes_to_copy))
	{
		return -EFAULT;
	}

	*offset += bytes_to_copy;
	printk(KERN_INFO "%s: Wrote %d bytes, offset = %lld\n", __func__, bytes_to_copy, *offset);
	return bytes_to_copy;

}

//fops structure
static struct file_operations fops={
	.owner = THIS_MODULE,
	.open = dyndrv_open,
	.release = dyndrv_release,
	.read = dyndrv_read,
	.write = dyndrv_write,
};

//init function
static int __init dyn_init(void)
{
	int ret;
	//allocate major/minor number dynamically
	ret=alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if(ret<0)
	{
		printk(KERN_ERR "Failed to allocate major/minor\n");
		return ret;
	}
	printk("%s: Regestred with major=%d and minor=%d\n",__func__,MAJOR(dev_num),MINOR(dev_num));
	
	// Initialize cdev
    	cdev_init(&my_cdev, &fops);
	my_cdev.owner = THIS_MODULE;

	// Add cdev to kernel
	ret = cdev_add(&my_cdev, dev_num, 1);
	if (ret < 0) {
    		unregister_chrdev_region(dev_num, 1);
    		printk(KERN_INFO "%s: Failed to add cdev\n", __func__);
    		return ret;
	}

	// Allocate device buffer
	device_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
	if (!device_buffer) {
    		cdev_del(&my_cdev);
    		unregister_chrdev_region(dev_num, 1);
    		printk(KERN_INFO "%s: Failed to allocate buffer\n", __func__);
    		return -ENOMEM;
	}
	return 0;
}

//exit function
static void __exit dyn_exit(void)
{
	kfree(device_buffer);
    	cdev_del(&my_cdev);
   	unregister_chrdev_region(dev_num, 1);
    	printk(KERN_INFO "%s: Module unloaded\n", __func__);
}

module_init(dyn_init);
module_exit(dyn_exit);

