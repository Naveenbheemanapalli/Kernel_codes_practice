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



static struct spi_driver lm70_driver = {
	.driver = {
		.name = DRVNAME,
		.of_match_table = of_match_ptr(of_device_ids),
	},
	.id_table = device_ids,
	.probe = lm70_probe,
};

module_spi_driver(lm70_driver);
