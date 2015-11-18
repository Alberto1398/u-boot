/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_RESET_H__
#define __ASM_ARCH_RESET_H__

#if defined(CONFIG_S900)
#include <asm/arch/reset_s900.h>
#else
#error Unknown SoC type
#endif

#ifndef __ASSEMBLY__

void owl_reset_assert(int rst_id);
void owl_reset_deassert(int rst_id);
void owl_reset(int rst_id);

void owl_reset_assert_by_perip_id(int perip_id);
void owl_reset_deassert_by_perip_id(int perip_id);
void owl_reset_by_perip_id(int perip_id);


#endif	/*__ASSEMBLY__ */
#endif	/* __ASM_ARCH_RESET_H__ */
