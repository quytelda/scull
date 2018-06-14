#include <linux/fs.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = SCULL_NR_DEVS;

module_param(scull_major, int, 0644);
module_param(scull_minor, int, 0644);

static int __init scull_init(void)
{
	int result;
	dev_t dev = 0;

	printk(KERN_INFO "scull: Loading SCULL module...\n");

	/*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} else {
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs,
					     "scull");
	}

	if(result < 0) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

	return 0;
}

static void __exit scull_exit(void)
{
	dev_t devno = MKDEV(scull_major, scull_minor);

	unregister_chrdev_region(devno, scull_nr_devs);

	printk(KERN_INFO "scull: Unloading SCULL module...\n");
}

module_init(scull_init);
module_exit(scull_exit);
