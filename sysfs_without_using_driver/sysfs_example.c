/*Brief Description:
 *
 * This Simple kernel module to demo interfacing with userspace via sysfs.
 * Sysfs is one of several available user<->kernel interfaces; the others
 * include sysfs, debugfs, netlink sockets and the ioctl.
 * In order to demonstrate (and let you easily contrast) between these
 * user<->kernel interfaces, 
 * this module creates a file called training in the path /sys/sasken/training
 * where we communicate to and from the kernel
 *
 * TO KNOW HOW TO RUN THIS MODULE PLEASE GO-THROUGH :- README FILE
 */

#include<linux/init.h>
#include<linux/module.h>
#include<linux/uaccess.h>			//copy_to/from_user functions
#include<linux/kobject.h>
#include<linux/kernel.h>
#include<linux/sysfs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/kdev_t.h>
#include<linux/slab.h>			//kmalloc() function
#include<linux/string.h>

#define MODULE_NAME "sysfs_example"			//degining the name of the kernel module

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bheemanapalli naveen Kumar");
MODULE_DESCRIPTION("A simple device driver for Describing working of the Sysfs filesystem");
MODULE_VERSION("1.0");

static struct kobject	*sysfs_kobj=NULL;
static volatile char training[20];

/*show function of the training file which will be called when read from the file*/
static ssize_t training_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf){
	return sprintf(buf,"%s\n\n",training);
}

/*store function of the training file which will be called when writting is done to the file*/
static ssize_t training_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count){
	sscanf(buf,"%s",training);
	return count;
}

/*setting the file attributes and permissions of the training file*/
static struct kobj_attribute attribute = __ATTR(training,0660,training_show,training_store);

/*sysfs_device_init() will be called when we initilizes(insmod) the kernel module */
static int __init sysfs_device_init(void){
	int error=0;
	pr_info("%s : module registered successfully...\n",MODULE_NAME);
	
	sysfs_kobj = kobject_create_and_add("sasken",NULL);		//creating a directory in the name of sasken in the /sys/ folder and obtaining the kernel object for it
	
	if(!sysfs_kobj){																		//checking if kernel object is created or not
		pr_info("unable create directory sasken folder in /sys/...\n");
		return -ENOMEM;
	}
	else{
		pr_info("sysfs_kobject created successfully ...\n");
	}
	
	error = sysfs_create_file(sysfs_kobj,&attribute.attr);	//creating the training file with attributes and permissions defined in the struct attribute
	if(error){
		pr_info("Unable to create file in the /sys/sasken/ folder...\n");
		return error;
	}
	return 0;
}


/*exiting and cleanup function which will be called when we remove the kernel module*/
static void __exit sysfs_device_exit(void){
	sysfs_remove_file(sysfs_kobj,&attribute.attr);	//removing the training file from the /sys/sasken folder
	kobject_put(sysfs_kobj);		//decrementing the kernel object and releasing the resource own by that kernel object
	kobject_del(sysfs_kobj);
	pr_info("%s : module unregistered successfully..\n",MODULE_NAME);
	
}


module_init(sysfs_device_init);			//intilization and exiting function registeration
module_exit(sysfs_device_exit);

