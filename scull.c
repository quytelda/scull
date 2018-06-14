/*
 * main.c -- the bare scull char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "scull.h"

static int scull_major = 0;
static int scull_minor = 0;
static int scull_nr_devs = SCULL_NR_DEVS;

module_param(scull_major, int, 0644);
module_param(scull_minor, int, 0644);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

struct scull_dev *scull_devices; /* Allocated in scull_init. */

static struct file_operations scull_fops = {
	.owner = THIS_MODULE,
};

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	dev_t devno = MKDEV(scull_major, scull_minor + index);
	int err;

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops   = &scull_fops;
	err = cdev_add(&dev->cdev, devno, 1);

	/* Fail gracefully if need be. */
	if(err)
		printk(KERN_NOTICE "scull: Error %d adding scull%d.\n",
		       err, index);
}

static void scull_exit(void)
{
	dev_t devno = MKDEV(scull_major, scull_minor);
	int i;

	printk(KERN_INFO "scull: Unloading SCULL module...\n");

	/* Get rid of our char dev entries. */
	if(scull_devices) {
		for(i = 0; i < scull_nr_devs; i++)
			cdev_del(&scull_devices[i].cdev);

		kfree(scull_devices);
	}

	/* scull_exit is never called if registering failed. */
	unregister_chrdev_region(devno, scull_nr_devs);
}


static int __init scull_init(void)
{
	dev_t dev = 0;
	int i;
	int result;

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

	/* 
	 * Allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	scull_devices = kcalloc(scull_nr_devs, sizeof(struct scull_dev), GFP_KERNEL);
	if(!scull_devices) {
		result = -ENOMEM;
		goto fail; /* Make this more graceful. */
	}

	for(i = 0; i < scull_nr_devs; i++)
		scull_setup_cdev(&scull_devices[i], i);

	return 0; /* Succeed. */

fail:
	scull_exit();
	return result;
}

module_init(scull_init);
module_exit(scull_exit);
