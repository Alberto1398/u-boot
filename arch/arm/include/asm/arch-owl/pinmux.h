/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __ASM_ARM_ARCH_PINMUX_H__
#define __ASM_ARM_ARCH_PINMUX_H__

#define MFP_MAXREG_NUM 12
#define DEVICE_MAX_MFP 5
struct periph_reg_config {
	unsigned long reg;
	unsigned int mask;
	unsigned int val;
};

struct periph_mfp_config {
	int mfp;
	struct periph_reg_config reg_cfg[MFP_MAXREG_NUM];
};

struct device_mfp_config {
	int periph_id;
	struct periph_mfp_config  mfp_cfg[DEVICE_MAX_MFP];
};


int pinmux_select(int periph_id, int mfp);

int pinmux_decode_periph_id(const void *blob, int node);


#endif /* __ASM_ARM_ARCH_PINMUX_H__ */

