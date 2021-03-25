#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/kernel.h>

#define DEVICE_NAME "zero_device"
#define CLASS_NAME "zero_class"

static dev_t device = 0;
static struct cdev device_cdev;
static struct class *zero_class = NULL;
static struct device *zero_device = NULL;

static int zero_open(struct inode *inode,struct file *filp){
	pr_info("Device file opened ..!\n");
	return 0;
}

static int zero_release(struct inode *inode, struct file *filp){
	pr_info("Device file closed ..!\n");
	return 0;
}

static loff_t zero_lseek(struct file *file,loff_t offset,int orig){
	return file->f_pos = 0;
}

static ssize_t null_write(struct file *filp,const char __user *buf,size_t count,loff_t *pos){
	pr_info("Written to Driver file ..!\n");
	return count;
}

static ssize_t null_read(struct file *filp,char __user *buf,size_t count,loff_t *pos){
	pr_info("Read to the Driver file ..!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = zero_read,
	.write = zero_write,
	.open = zero_open,
	.release = zero_release,
	.llseek = zero_lseek,
	//.ioctl = zero_ioctl,
};


static int __init zero_device_init(void){
	int status = 0;
	pr_info("Init started ..!\n");
	
	status = alloc_chrdev_region(&device, 0, 1, DEVICE_NAME);
	if(status < 0){
		pr_err("Unable to allocate the chrdev region ..!\n");
		return status;
	}
	pr_info("Allocated Major = %d and Minor = %d \n",MAJOR(device),MINOR(device));
	
	cdev_init(&device_cdev, &fops);
	status = cdev_add(&device_cdev, device, 1);
	if(status < 0){
		pr_err("Unable to allocate the cdev and adding it ..!\n");
		unregister_chrdev_region(device, 1);
		return status;
	}
	pr_info("Done creating the cdev and adding it ..!\n");
	
	zero_class = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(zero_class)){
		pr_err("Unable to create the class for device ..!\n");
		unregister_chrdev_region(device, 1);
		return PTR_ERR(zero_class);
	}
	pr_info("Created the class for the device ..!\n");
	
	zero_device = device_create(zero_class, NULL, device, NULL, DEVICE_NAME);
	if(IS_ERR(zero_device)){
		pr_err("Unable to create device file ..!\n");
		class_destroy(zero_class);
		unregister_chrdev_region(device, 1);
		return PTR_ERR(zero_device);
	}
	pr_info("created the deive file ..!\n");
	
	
	return 0;
}

static void __exit zero_device_exit(void){
	pr_info("exit started ..!\n");
	device_destroy(zero_class, device);
	class_destroy(zero_class);
	unregister_chrdev_region(device, 1);
}


module_init(zero_device_init);
module_exit(zero_device_exit);

MODULE_AUTHOR("Loosofer");
MODULE_DESCRIPTION("Simple Zero Device Driver");

MODULE_LICENSE("GPL");
