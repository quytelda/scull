/*
 * scull.h -- definitions for the char module
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
 * $Id: scull.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 */

#ifndef _SCULL_H_
#define _SCULL_H_

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4
#endif

struct scull_dev {
	size_t size;
	struct cdev cdev;
};

#endif /* _SCULL_H_ */
