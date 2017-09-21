/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_SYS_PROTO_H__
#define __ASM_ARCH_SYS_PROTO_H__

#ifdef CONFIG_BOOTDEV_AUTO
#define BOOTDEV_NAND 0x0
#define BOOTDEV_EMMC 0x22
#define BOOTDEV_SD 0x20
void owl_bootdev_init(void);
int owl_get_bootdev(void);
int owl_bootdev_env(void);
#endif

#ifdef CONFIG_SMC
unsigned int __read_data(int Instruction);
#endif

#ifdef CONFIG_ANDROID_RECOVERY
int check_recovery_mode(void);
void setup_recovery_env(void);
#endif

#ifdef CONFIG_CHECK_KEY
int owl_check_key(void);
#endif

u64 __invoke_fn_smc(u64 function_id, u64 arg0, u64 arg1,
					 u64 arg2);

#endif	/* __ASM_ARCH_SYS_PROTO_H__ */
