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


static struct lm70 {
	struct spi_device *spi_dev;
	struct mutex mutex_lock;
};

static int lm70_probe(struct spi_device *spi)
{ 
	return 0;
}


static const struct of_device_id of_device_ids[] = {
	{ .compatible = "ti,lm70",
		.data	=	0
	},
	{},
};
MODULE_DEVICE_TABLE(of, of_device_ids);

static const struct spi_device_id device_ids[] = {
	{ "ti,lm70", 0},
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
};

module_spi_driver(lm70_driver);
