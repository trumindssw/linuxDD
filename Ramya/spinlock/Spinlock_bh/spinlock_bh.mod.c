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
	{ 0xe8213e80, "_printk" },
	{ 0xde338d9a, "_raw_spin_unlock_bh" },
	{ 0x67628f51, "msleep" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x8a545344, "kthread_create_on_node" },
	{ 0xa31d2ab6, "wake_up_process" },
	{ 0x2dc71874, "kthread_stop" },
	{ 0xd272d446, "__fentry__" },
	{ 0x6c0dfe1a, "const_pcpu_hot" },
	{ 0xde338d9a, "_raw_spin_lock_bh" },
	{ 0xab006604, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xe8213e80,
	0xde338d9a,
	0x67628f51,
	0xd272d446,
	0x8a545344,
	0xa31d2ab6,
	0x2dc71874,
	0xd272d446,
	0x6c0dfe1a,
	0xde338d9a,
	0xab006604,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"_printk\0"
	"_raw_spin_unlock_bh\0"
	"msleep\0"
	"__x86_return_thunk\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"kthread_stop\0"
	"__fentry__\0"
	"const_pcpu_hot\0"
	"_raw_spin_lock_bh\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "2E50C55640A75AAA81F0EC8");
