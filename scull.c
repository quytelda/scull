#include <linux/module.h>
#include <linux/kernel.h>

static int scull_major = 0;
static int scull_minor = 0;

module_param(scull_major, int, 0644);
module_param(scull_minor, int, 0644);

static int __init scull_init(void)
{
	printk(KERN_INFO "scull: Loading SCULL module...\n");
	return 0;
}

static void __exit scull_exit(void)
{
	printk(KERN_INFO "scull: Unloading SCULL module...\n");
}

module_init(scull_init);
module_exit(scull_exit);
