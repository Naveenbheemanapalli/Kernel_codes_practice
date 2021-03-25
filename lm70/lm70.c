#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/of_device.h>
#include <linux/acpi.h>

#define DRVNAME		"lm70"

enum chips_lm70{
	LM70_TEMPERATURE,
};

struct lm70 {
	struct spi_device *spi_dev;
	struct mutex mutex_lock;
	int chip;
};

static ssize_t temp1_input_show(struct device *dev,struct device_attribute *attr, char *buf) {
	return 0;
}










static DEVICE_ATTR_RO(temp1_input);

static struct attribute *lm70_attrs[] = {
	&dev_attr_temp1_input.attr,
	NULL
};

ATTRIBUTE_GROUPS(lm70);


static int lm70_probe(struct spi_device *spi)
{ 
	const struct of_device_id *match;
	struct spi_device *device;
	struct device *hwmon;
	struct lm70 *spi_lm70;
	int chip,ret;
	
	match = of_match_device(of_device_ids, &spi->dev);
	if(match){
		pr_err("Device of table registration success ..!\n");
		chip = (int)(uintptr_t)match->data;
	}
	else {
		chip = spi_get_device_id(spi)->driver_data;
	}

	spi_lm70 = devm_kzalloc(&spi->dev,sizeof(struct lm70),GFP_KERNEL);
	if(spi_lm70 == NULL)
		return -ENOMEM;
		
	mutex_init(&spi_lm70->mutex_lock);	
	spi_lm70->spi_dev = spi;
	spi_lm70->chip = chip;
	
/*
	device = spi_alloc_device(spi->controller);
	if(device == NULL)
		return -ENODEV;
	
	ret = spi_add_device(spi);
	if(!ret){
		pr_err("Error on adding the device..!\n");
		return ret;
	}*/
	
	hwmon = devm_hwmon_device_register_with_groups(&spi->dev,spi->modalias,spi_lm70, lm70_groups);
	
	if(hwmon == NULL)
		return PTR_ERR_OR_ZERO(hwmon_dev);

	return 0;
}


static const struct of_device_id of_device_ids[] = {
	{ .compatible = "ti,lm70",
		.data	=	LM70_TEMPERATURE,
	},
	{},
};
MODULE_DEVICE_TABLE(of, of_device_ids);

static const struct spi_device_id device_ids[] = {
	{ "ti,lm70", LM70_TEMPERATURE},
	{},
};
MODULE_DEVICE_TABLE(spi, device_ids);



static struct spi_driver lm70_driver = {
	.driver = {
		.name = DRVNAME,
		.of_match_table = of_match_ptr(of_device_ids),
	},
	.id_table = device_ids,
	.probe = lm70_probe,
	//.remove = lm70_remove,
};

module_spi_driver(lm70_driver);
