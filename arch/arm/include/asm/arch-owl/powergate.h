/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_POWERGATE_H__
#define __ASM_ARCH_POWERGATE_H__

#if defined(CONFIG_S900)
#include <asm/arch/powergate_s900.h>
#elif defined(CONFIG_S700)
#include <asm/arch/powergate_s700.h>
#else
#error Unknown SoC type
#endif

#ifndef __ASSEMBLY__

int owl_powergate_power_on(int pg_id);
int owl_powergate_power_off(int pg_id);
int owl_powergate_is_powered(int pg_id);
int owl_powergate_power_raw(int pg_id, int on);

#endif	/*__ASSEMBLY__ */
#endif	/* __ASM_ARCH_POWERGATE_H__ */
