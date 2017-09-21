/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_SMC_H__
#define __ASM_ARCH_SMC_H__

#include <common.h>

#define OWL_COREPLL_RECALC_RATE		0xc3000004
#define OWL_COREPLL_SET_RATE		0xc3000005

#ifndef __ASSEMBLY__

noinline u64 owl_invoke_fn_smc(u64 function_id, u64 arg0, u64 arg1,
					 u64 arg2);

#endif	/*__ASSEMBLY__ */
#endif	/* __ASM_ARCH_SMC_H__ */
