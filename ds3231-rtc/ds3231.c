#include <linux/acpi.h>
#include <linux/bcd.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/rtc/ds1307.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/clk-provider.h>
#include <linux/regmap.h>
#include <linux/delay.h>

#define MEMORY  2048

#define DS3231_REG_SECS			0x00
#define DS3231_BIT_CH				0x80
#define DS3231_REG_MIN			0x01
#define DS3231_REG_HOUR			0x02
#define DS3231_BIT_12HR			0x40
#define DS3231_BIT_PM				0x20
#define DS3231_REG_WDAY			0x03
#define DS3231_REG_MDAY			0x04
#define DS3231_REG_MONTH		0x05
#define DS3231_BIT_CENTURY	0x80
#define DS3231_REG_YEAR			0x06
#define DS3231_REG_CONTROL 	0x0e
#define DS3231_BIT_EOSC		  0x80
#define DS3231_BIT_BBSQW		0x40
#define DS3231_REG_STATUS		0x0f
#define DS3231_BIT_OSF			0x80

#define CHIP_I2C_DEVICE_NAME "rtc-ds3231"

static int dev_configure(void);

static struct i2c_client * chip_i2c_client = NULL;
static struct class *chip_class = NULL;
static struct device *device_chip = NULL;
static int chip_major=0;
static char *kernel_buf = NULL;


/// for 1 process at a time..
static DEFINE_MUTEX(chip_i2c_mutex);

static const unsigned short normal_i2c[] = { 0x68, I2C_CLIENT_END };


static int chip_read_value(struct i2c_client *client, u8 reg){
	struct ds3231_data *data = i2c_get_clientdata(client);
	int val = 0;
	
	dev_info(&client->dev, "%s\n", __FUNCTION__);
	
	//mutex_lock(&data->update_lock);
  val = i2c_smbus_read_byte_data(client, reg);
  //mutex_unlock(&data->lock);
  
	dev_info(&client->dev, "%s : read reg [%02x] returned [%d]\n", __FUNCTION__, reg, val);
	
	
	return val;
}

static int chip_write_value(struct i2c_client *client,u8 reg, u8 value){
	int val = 0;
	dev_info(&client->dev, "%s\n", __FUNCTION__);
	
	val = i2c_smbus_write_byte_data(client, reg, value);
	
	dev_info(&client->dev, "%s : write reg [%02x] returned [%d]\n", __FUNCTION__, reg, val);
	
	return val;
}

///////********************* file operations ***********************************///////////

static int chip_i2c_open(struct inode * inode, struct file *filep)
{
	if (chip_i2c_client == NULL)
		return -ENODEV;
		
	if (!mutex_trylock(&chip_i2c_mutex)){
		pr_err("%s: Device currently in use!\n", __FUNCTION__);
    return -EBUSY;
	}
	
	kernel_buf = kzalloc(MEMORY,GFP_KERNEL);
	if (kernel_buf == NULL) {
		pr_info("Cannot allocate memory in kernel...!\n");
		return -ENOMEM;
	}

	return 0;
}

static int chip_i2c_close(struct inode * inode, struct file *filep)
{
	pr_info("closed ....!\n");
	kfree(kernel_buf);
	mutex_unlock(&chip_i2c_mutex);
	return 0;
}

static ssize_t chip_i2c_write(struct file *filep, const char __user * buf, size_t count, loff_t * offset)
{
	int retval=0,i=0,flag=1,val=0;
	pr_info("write ....!\n");
	if(copy_from_user(kernel_buf,buf,MEMORY) != 0){
		pr_err("%s: Unable to copy from user space..!\n",__FUNCTION__);
		retval = -EFAULT;
		goto err;
	}
	
	for(i=0;kernel_buf[i];){
		if(kernel_buf[i] >= '0' && kernel_buf[i] <= '9'){
			if(flag != 6){
				val = val*10 + kernel_buf[i]-'0';
				val = val*10 + kernel_buf[i+1]-'0';
				i = i+2;
				flag++;
			}
			else{
				val = val*10 + kernel_buf[i]-'0';
				val = val*10 + kernel_buf[i+1]-'0';
				val = val*10 + kernel_buf[i+2]-'0';
				val = val*10 + kernel_buf[i+3]-'0';
				flag++;
				i = i+4;
			}
			switch(flag-1){
			case 1 :
				chip_write_value(chip_i2c_client,DS3231_REG_HOUR,bin2bcd(val));
				break;
			case 2 :
				chip_write_value(chip_i2c_client,DS3231_REG_MIN,bin2bcd(val));	
				break;
			case 3 :
				chip_write_value(chip_i2c_client,DS3231_REG_SECS,bin2bcd(val));	
				break;
			case 4 :
				chip_write_value(chip_i2c_client,DS3231_REG_MDAY,bin2bcd(val));
				break;
			case 5 :
				chip_write_value(chip_i2c_client,DS3231_REG_MONTH,bin2bcd(val));	
				break;
			case 6 :
				chip_write_value(chip_i2c_client,DS3231_REG_YEAR,bin2bcd(val));
				break;
			default :
				break;			
			}
			val = 0;
		}
	}
	return count;
err :
	return retval;
}

static ssize_t chip_i2c_read(struct file *filep,char __user *buf,size_t count,loff_t *offset){
	u8 regs[8];
	int ret=0;
	memset(regs,0,8);
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_SECS);
	if(ret < 0){
		pr_info("%s: error in reading the value of seconds ..!\n",__FUNCTION__);
		goto err;
	}
	regs[0] = bcd2bin(ret & 0x7f); ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_MIN);
	if(ret < 0){
		pr_info("%s: error in reading the value of minutes ..!\n",__FUNCTION__);
		goto err;
	}
	regs[1] = bcd2bin(ret & 0x7f); ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_HOUR);
	if(ret < 0){
		pr_info("%s: error in reading the value of hours ..!\n",__FUNCTION__);
		goto err;
	}
	regs[2] = bcd2bin(ret & 0x1f); ret=0;
	pr_info("Time : %d - %d -%d ...!\n",regs[2],regs[1],regs[0]);
	
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_WDAY);
	if(ret < 0){
		pr_info("%s: error in reading the value of Week day ..!\n",__FUNCTION__);
		goto err;
	}
	regs[3] = bcd2bin(ret & 0x07); ret=0;	
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_MDAY);
	if(ret < 0){
		pr_info("%s: error in reading the value of hours ..!\n",__FUNCTION__);
		goto err;
	}
	regs[4] = bcd2bin(ret & 0x3f); ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_MONTH);
	if(ret < 0){
		pr_info("%s: error in reading the value of hours ..!\n",__FUNCTION__);
		goto err;
	}
	regs[5] = bcd2bin(ret & 0x1f); ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_YEAR);
	if(ret < 0){
		pr_info("%s: error in reading the value of hours ..!\n",__FUNCTION__);
		goto err;
	}
	regs[6] = bcd2bin(ret); ret=0;
	pr_info("Date : %d - %d -%d ...!\n",regs[4],regs[5],regs[6]);
	
	sprintf(kernel_buf,"Time = %d-%d-%d \n Date = %d-%d-%d \n",regs[2],regs[1],regs[0],regs[4],regs[5],regs[6]);
	if(copy_to_user(buf,kernel_buf,MEMORY) != 0){
		pr_err("%s: Unable to copy to the user..!\n",__FUNCTION__);
		return -EFAULT;
	}
		
err:
	return ret;
}
/********************************* file operations completed ***************************************************/

/********************************* sysfs file operatons ********************************************************/

static ssize_t show_time(struct device *dev,struct device_attribute *attr, char *buf)
{
	int len = 0;
	pr_info("show function..!\n");
	u8 regs[3];
	int ret=0;
	memset(regs,0,8);
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_SECS);
	if(ret < 0){
		pr_info("%s: error in reading the value of seconds ..!\n",__FUNCTION__);
		goto err;
	}
	regs[0] = bcd2bin(ret & 0x7f); ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_MIN);
	if(ret < 0){
		pr_info("%s: error in reading the value of minutes ..!\n",__FUNCTION__);
		goto err;
	}
	regs[1] = bcd2bin(ret & 0x7f); ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_HOUR);
	if(ret < 0){
		pr_info("%s: error in reading the value of hours ..!\n",__FUNCTION__);
		goto err;
	}
	regs[2] = bcd2bin(ret & 0x1f); ret=0;
	pr_info("Time : %d - %d -%d ...!\n",regs[2],regs[1],regs[0]);
	return len;
}

static ssize_t store_time(struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
		pr_info("get time ...!\n");
    return count;
}

static ssize_t show_date(struct device *dev,struct device_attribute *attr, char *buf)
{
	int len = 0;
	pr_info("show function..!\n");
	return len;
}

static ssize_t store_date(struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
{
		pr_info("get time ...!\n");
    return count;
}

/************************************* sysfs file operations completed*******************************************/

struct ds3231_data {
	struct device 		*dev;
	struct cdev				cdev;
	//struct regmap 		*regmap;
	struct mutex			update_lock;
	//struct rtc_device 	*rtc; 
};

static const struct file_operations chip_i2c_fops = {
    .owner = THIS_MODULE,
    .llseek = no_llseek,
    .write = chip_i2c_write,
    .read	=	chip_i2c_read,
    .open = chip_i2c_open,
    .release = chip_i2c_close
};

/*************************************** sysfs declarations *****************************/

static DEVICE_ATTR(time,0660, show_time, store_time);
static DEVICE_ATTR(date, 0660, show_date, store_date);

/******************************************IDs ****************************************/
static const struct i2c_device_id ds3231_id[] = {
	{ "ds3231", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ds3231_id);

#ifdef CONFIG_OF
static const struct of_device_id ds3231_of_match[] = {
	{
		.compatible = "maxim,ds3231",
		.data = (void *)0,
	},
	{ }
};
MODULE_DEVICE_TABLE(of, ds3231_of_match);
#endif
/********************************************** IDs copied from old code ************************/
static int dev_configure(void){
	int ret=0;
	unsigned int tmp=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_CONTROL);
	if(ret < 0){
			pr_err("%s: error in the control reading..!\n",__FUNCTION__);
			return ret;	
	}	
	if( ret & DS3231_BIT_EOSC)
		ret &= ~DS3231_BIT_EOSC;
	
	ret = chip_write_value(chip_i2c_client,DS3231_REG_CONTROL,ret);
	if(ret < 0){
		pr_err("%s: error in the control register writing..!\n",__FUNCTION__);
		return ret;
	} ret=0;
	/*TODO checking need to done here for correction of time */
	ret = chip_read_value(chip_i2c_client,DS3231_REG_STATUS);
	if(ret < 0){
			pr_err("%s: error in the control reading..!\n",__FUNCTION__);
			return ret;	
	}
	if(ret & DS3231_BIT_OSF){
		chip_write_value(chip_i2c_client,DS3231_REG_STATUS,ret & ~DS3231_BIT_OSF);
	} ret=0;
	
	ret = chip_read_value(chip_i2c_client,DS3231_REG_HOUR);
	if(ret < 0){
		pr_err("%s:  error in reading the DS3231_REG_HOUR register..!\n",__FUNCTION__);
		return ret;
	}
	tmp = ret;
	if((ret & DS3231_BIT_12HR));
	else{
		ret = bcd2bin(ret & 0x1f);
		if(ret == 12)
			ret = 0;
		if(tmp & DS3231_BIT_PM)
			ret += 12;
		chip_write_value(chip_i2c_client,DS3231_REG_HOUR,bin2bcd(ret));
	
	}
	return 0;		
}



//////////////////////////******************* probing ******************************/////////////////////

static int ds3231_probe(struct i2c_client *client,const struct i2c_device_id *id){
	int retval=0;
	struct device * dev = &client->dev;
  struct ds3231_data *data = NULL;
	
    printk("chip_i2c: %s\n", __FUNCTION__);
    
    data = devm_kzalloc(&client->dev, sizeof(struct ds3231_data), GFP_KERNEL);
    if(!data)
        return -ENOMEM;
        
    i2c_set_clientdata(client, data);
    
    mutex_init(&data->update_lock);
    chip_i2c_client = client;
    retval = dev_configure();
    if(retval < 0){
    	pr_info("%s: Error in configuring the device..!\n",__FUNCTION__);
    	goto err;
    }
    
    chip_major = register_chrdev(0, CHIP_I2C_DEVICE_NAME,&chip_i2c_fops);
    
    if(chip_major < 0){
    	retval = chip_major;
    	pr_err("Unable to register the device and allocate major number..! in function %s \n",__FUNCTION__);
    	goto err;    
    }
    
    chip_class = class_create(THIS_MODULE, CHIP_I2C_DEVICE_NAME);
    if(IS_ERR(chip_class)){
        retval = PTR_ERR(chip_class);
        pr_info("%s: Failed to create class!\n", __FUNCTION__);
        goto err1;
    }
    
    device_chip = device_create(chip_class,NULL,MKDEV(chip_major,0),NULL,CHIP_I2C_DEVICE_NAME);
    if(IS_ERR(device_chip)){
    	retval = PTR_ERR(device_chip);
    	pr_info("%s: Failed to create device file for chip ..!\n", __FUNCTION__);
    	goto err2;
    }
    
    mutex_init(&chip_i2c_mutex);
    
    device_create_file(dev,&dev_attr_time);
    device_create_file(dev,&dev_attr_date);
    
    return 0;
err2 :
	class_destroy(chip_class);   
err1 :
	unregister_chrdev(chip_major, CHIP_I2C_DEVICE_NAME);
err :
	return retval;
}

/***************************************** Removing ****************************************************/

static int ds3231_remove(struct i2c_client * client)
{
	struct device *dev = &client->dev;
    pr_info("chip_i2c: %s\n", __FUNCTION__);

    chip_i2c_client = NULL;
    
    device_remove_file(dev,&dev_attr_time);
    device_remove_file(dev,&dev_attr_date);

	  device_destroy(chip_class, MKDEV(chip_major, 0));
    class_destroy(chip_class);
    unregister_chrdev(chip_major, CHIP_I2C_DEVICE_NAME);

    return 0;
}

/******************************************* DEtection *************************************************/
static int ds3231_detect(struct i2c_client * client, struct i2c_board_info * info)
{
    struct i2c_adapter *adapter = client->adapter;
    int address = client->addr;
    const char * name = NULL;

    pr_info("chip_i2c: %s!\n", __FUNCTION__);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -ENODEV;

    if (address == 0x68)
    {
        name = CHIP_I2C_DEVICE_NAME;
        dev_info(&adapter->dev,
            "Chip device found at 0x%02x\n", address);
    }
    else
        return -ENODEV;

    strlcpy(info->type, name, I2C_NAME_SIZE);
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct i2c_driver ds3231_driver = {
	.driver = {
		.name	= CHIP_I2C_DEVICE_NAME,
		.of_match_table = of_match_ptr(ds3231_of_match),
	},
	.probe		= ds3231_probe,
	.remove		= ds3231_remove,
	.id_table	= ds3231_id,
	.detect		=	ds3231_detect,
	.address_list	= normal_i2c,
};

module_i2c_driver(ds3231_driver);

MODULE_DESCRIPTION("RTC driver for DS3231 and similar chips");
MODULE_LICENSE("GPL");
