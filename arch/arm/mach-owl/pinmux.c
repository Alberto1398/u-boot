/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/periph.h>
#include <fdtdec.h>



__weak int pin_funcmux_select(int periph_id, int mfp)
{
	return 0;
}

int pinmux_select(int periph_id, int mfp)
{
	return pin_funcmux_select(periph_id, mfp);
}


#ifdef CONFIG_OF_CONTROL
int pinmux_decode_periph_id(const void *blob, int node)
{
	return fdtdec_get_int(blob, node, "clk_id", PERIPH_ID_NONE);
}
#endif


