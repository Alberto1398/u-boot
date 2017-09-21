/*
 * Copyright 2016 Actions Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ADFU_H_
#define _ADFU_H_

/* usb connect type */
enum {
	CONNECT_NO_DEVICE = 0,
	CONNECT_TO_PC = 1,
	CONNECT_TO_ADAPTER = 2,
	CONNECT_UDISK = 3
};

/* usb3 vbus detect type */
enum {
	VBUS_IC = 1,
	VBUS_PMU = 2
};

/* usb3 vbus detect state; 1: low, 2: high */
enum {
	VBUS_LOW = 1,
	VBUS_HIGH = 2
};

extern int connect_to_pc;
extern int adfu_restart_complete;

int owl_usb_get_connect_type(void);
int do_adfu(void);

#endif /* _ADFU_H_ */
