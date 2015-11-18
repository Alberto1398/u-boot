/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_SYS_PROTO_H__
#define __ASM_ARCH_SYS_PROTO_H__

#ifdef CONFIG_ANDROID_RECOVERY
int check_recovery_mode(void);
void setup_recovery_env(void);
#endif

#endif	/* __ASM_ARCH_SYS_PROTO_H__ */
