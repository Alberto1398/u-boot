/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_REGS_H__
#define __ASM_ARCH_REGS_H__

#if defined(CONFIG_S900)
#include <asm/arch/regs_s900.h>
#elif defined(CONFIG_S700)
#include <asm/arch/regs_s700.h>
#else
#error Unknown SoC type
#endif

#endif /* __ASM_ARCH_REGS_H__ */
