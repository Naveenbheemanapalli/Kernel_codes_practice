#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/device.h>
#include<linux/string.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

#define MEMORY 2048

static dev_t dev = 0;
static struct class *cls = NULL;
static struct device *dev_ret = NULL;
static struct cdev dev_cdev;
static uint8_t *kernel_buf;

static ssize_t dev_read(struct file *filp, char __user * buf, size_t len,
			loff_t * off)
{
	if ((copy_to_user(buf, kernel_buf, MEMORY)) != 0) {
		pr_info("Cannot copy data to the user space...!\n");
		return -1;
	}
	pr_info("Device read function called ...!\n");
	return 0;
}

static ssize_t dev_write(struct file *filp, const char __user * buf, size_t len,
			 loff_t * off)
{
	if ((copy_from_user(kernel_buf, buf, MEMORY)) != 0) {
		pr_info("Cannot copy data to the kernel space...!\n");
		return -1;
	}
	pr_info("Device write function called ...!\n");
	return len;
}

static int dev_open(struct inode *inode, struct file *file)
{
	kernel_buf = kmalloc(MEMORY, GFP_KERNEL);
	if (kernel_buf == NULL) {
		pr_info("Cannot allocate memory in kernel...!\n");
		return -1;
	}
	pr_info("Device file opened ...!\n");
	return 0;
}

static int dev_close(struct inode *inode, struct file *file)
{
	kfree(kernel_buf);
	pr_info("Device file closed ...!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = dev_read,
	.write = dev_write,
	.open = dev_open,
	.release = dev_close,
};

int __init chrdev_init(void)
{
	pr_debug("HI I am in ...\n");

	if ((alloc_chrdev_region(&dev, 0, 1, "chr_dev")) < 0) {
		pr_err("cannot allocate the Major and Minor numbers...\n");
		return -1;
	}
	pr_info("Major = %d and Minor = %d are allocated for ur device...\n",
		MAJOR(dev), MINOR(dev));

	cdev_init(&dev_cdev, &fops);

	if (cdev_add(&dev_cdev, dev, 1) < 0) {
		pr_debug("unable to add the file operations ...\n");
		return -1;
	}

	cls = class_create(THIS_MODULE, "Z_class");
	if (cls == NULL) {
		pr_info("Cannot create class for device file ...\n");
		return -1;
	}
	pr_info("Created class file...\n");

	dev_ret = device_create(cls, NULL, dev, NULL, "chr_dev");
	if (dev_ret == NULL) {
		pr_info("cannot create the device file in the /dev ...\n");
		return -1;
	}
	pr_info("created device file successfully ...\n");

	return 0;
}

void __exit chrdev_exit(void)
{
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	pr_debug("BYe I am out ...\n");

}

module_init(chrdev_init);
module_exit(chrdev_exit);
MODULE_AUTHOR("fgkegbkjs");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0000");
