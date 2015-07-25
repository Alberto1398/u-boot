/*
 * Asoc matrix keypad scan in boot
 *
 * Copyright (C) 2011 Actions Semiconductor, Inc
 * Author:	chenbo <chenbo@actions-semi.com>
 *
 * Based on Samsung keypad driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/arch/serial.h>
#include <asm/io.h>
#include <input.h>
#include <asm/arch/gpio.h>
#include <asm/arch/cpu.h>


int adckey_scan(void)
{
	unsigned int adckey_no = 0;

	debug("adckey scan ...\n");
	if (ftstc(0) > 0)
		adckey_no = fgetc(0);

	printf("get key value= %d\n", adckey_no);
	return adckey_no;
}


