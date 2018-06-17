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

static int scull_major = SCULL_MAJOR;
static int scull_minor = 0;
static int scull_nr_devs = SCULL_NR_DEVS;
static int scull_quantum = SCULL_QUANTUM;
static int scull_qset    = SCULL_QSET;

module_param(scull_major, int, 0644);
module_param(scull_minor, int, 0644);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

struct scull_dev *scull_devices; /* Allocated in scull_init_module(). */

static int scull_trim(struct scull_dev *dev)
{
	int i;
	size_t qset = dev->qset;

	struct scull_qset *dptr;
	struct scull_qset *next;

	for(dptr = dev->data; dptr; dptr = next) { /* all the list items */
		if(dptr->data) {
			for(i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}

		next = dptr->next;
		kfree(dptr);
	}

	dev->size    = 0;
	dev->quantum = scull_quantum;
	dev->qset    = scull_qset;
	dev->data    = NULL;

	return 0;
}

int open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* Now trim to 0 the length of the device if open was write-only. */
	if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
		scull_trim(dev); /* Ignore errors. */
	}

	return 0; /* Success. */
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

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

static void scull_cleanup_module(void)
{
	dev_t devno = MKDEV(scull_major, scull_minor);
	int i;

	printk(KERN_INFO "scull: Unloading SCULL module...\n");

	/* Get rid of our char dev entries. */
	if(scull_devices) {
		for(i = 0; i < scull_nr_devs; i++) {
			scull_trim(&scull_devices[i]);
			cdev_del(&scull_devices[i].cdev);
		}

		kfree(scull_devices);
	}

	/* scull_cleanup_module() is never called if registering failed. */
	unregister_chrdev_region(devno, scull_nr_devs);
}

static int __init scull_init_module(void)
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
		scull_major = MAJOR(dev);
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
	scull_cleanup_module();
	return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
