/*
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Copyright 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __F_FASTBOOT_H_
#define __F_FASTBOOT_H_

#include <linux/compiler.h>
#include <linux/usb/composite.h>

/* configuration-specific linkup */
 int fastboot_add(struct usb_configuration *c);
#endif /* __F_DFU_H_ */
