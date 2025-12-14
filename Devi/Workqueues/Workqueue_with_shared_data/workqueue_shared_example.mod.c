#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xde338d9a, "_raw_spin_unlock" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xdf4bee3d, "alloc_workqueue" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0xbeb1d261, "__flush_workqueue" },
	{ 0xbeb1d261, "destroy_workqueue" },
	{ 0xd272d446, "__fentry__" },
	{ 0xde338d9a, "_raw_spin_lock" },
	{ 0xe8213e80, "_printk" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xde338d9a,
	0xd272d446,
	0xdf4bee3d,
	0x49733ad6,
	0xbeb1d261,
	0xbeb1d261,
	0xd272d446,
	0xde338d9a,
	0xe8213e80,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"_raw_spin_unlock\0"
	"__x86_return_thunk\0"
	"alloc_workqueue\0"
	"queue_work_on\0"
	"__flush_workqueue\0"
	"destroy_workqueue\0"
	"__fentry__\0"
	"_raw_spin_lock\0"
	"_printk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9491528BA1D1C1A71383CC1");
