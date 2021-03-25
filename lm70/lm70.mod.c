#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xc79d2779, "module_layout" },
	{ 0x81316162, "driver_unregister" },
	{ 0x3bdcf1f0, "__spi_register_driver" },
	{ 0xc5850110, "printk" },
	{ 0x591babab, "spi_add_device" },
	{ 0x15c85464, "spi_alloc_device" },
	{ 0x977f511b, "__mutex_init" },
	{ 0x1fbed5b6, "devm_kmalloc" },
	{ 0x1fd3f7ba, "spi_get_device_id" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:ti,lm70");
MODULE_ALIAS("of:N*T*Cti,lm70");
MODULE_ALIAS("of:N*T*Cti,lm70C*");

MODULE_INFO(srcversion, "77E51B08E1FC7B3464F8485");
