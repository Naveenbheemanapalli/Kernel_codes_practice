#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/kernel.h>

#define DEVICE_NAME "null_device"
#define CLASS_NAME "null_class"

static struct cdev device_cdev;
static struct class *null_class = NULL;
static struct device *null_device = NULL;
static dev_t device=0;

static int null_open(struct inode *inode,struct file *filp){
	pr_info("Device file opened ..!\n");
	
	return 0;
}

static int null_release(struct inode *inode, struct file *filp){
	pr_info("Device file closed ..!\n");
	return 0;
}

static ssize_t null_write(struct file *filp,const char __user *buf,size_t count,loff_t *pos){
	/*pr_info("Written to Driver file ..!\n");
	pr_info("-----------------------------------------------\n");
	pr_info("writting data is as follows : %s\n",buf);
	pr_info("count = %d\n",(int)count);
	pr_info("-----------------------------------------------\n");*/
	return count;
}

static ssize_t null_read(struct file *filp,char __user *buf,size_t count,loff_t *pos){
	/*pr_info("Read to the Driver file ..!\n");
	pr_info("-----------------------------------------------\n");
	pr_info("Readed data is as follows : %s\n",buf);
	pr_info("count = %d\n",(int)count);
	pr_info("-----------------------------------------------\n");*/
	return 0;
}

static loff_t null_lseek(struct file *file,loff_t offset,int orig){
	return file->f_pos = 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.llseek = null_lseek,
	.read = null_read,
	.write = null_write,
	.open = null_open,
	.release = null_release,
	//.ioctl = null_ioctl,
};

static int __init null_device_init(void)
{
	int status = 0;
	pr_info("init done ..!\n");
	
	status = alloc_chrdev_region(&device,0,1,DEVICE_NAME);
	if(status < 0){
		pr_err("unable to allocate the Major and Minor numbers..! \n");
		return -ENOMEM;
	}
	else{
		pr_info("Allocate Major = %d and Minor = %d ..!\n",MAJOR(device),MINOR(device));
	}
	
	cdev_init(&device_cdev,&fops);
	
	status = cdev_add(&device_cdev,device,1);
	if(status < 0){
		pr_err("unable to add the cdev structure ..!\n");
		unregister_chrdev_region(device, 1);
		return status;
	}
	pr_info("Allocated the cdev and added it successfully ..!\n");
	
	null_class = class_create(THIS_MODULE,CLASS_NAME);
	if(IS_ERR(null_class)){
		pr_err("Unable to alloacte the class in /sys/class ..!\n");
		unregister_chrdev_region(device, 1);
		return PTR_ERR(null_class);
	}
	pr_info("Created class file...\n");	
	
	null_device = device_create(null_class,NULL,device,NULL,DEVICE_NAME);
	if(IS_ERR(null_device)){
		pr_err("Unable to create device ..!\n");
		class_destroy(null_class);
		unregister_chrdev_region(device, 1);
		return PTR_ERR(null_device);
	}
	pr_info("Created Device file successfully ..!\n");
	
	
	return 0;
}

static void __exit null_device_exit(void)
{
	pr_info("exit done ..!\n");
	device_destroy(null_class,device);
	class_destroy(null_class);
	unregister_chrdev_region(device,1);

}

module_init(null_device_init);
module_exit(null_device_exit);

MODULE_AUTHOR("Loosofer");
MODULE_DESCRIPTION("Simple Null Device Driver");

MODULE_LICENSE("GPL");
