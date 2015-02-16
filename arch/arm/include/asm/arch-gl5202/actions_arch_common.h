/*
 * actions_arch_common.h
 *
 *  Created on: 2014-6-20
 *      Author: liangzhixuan
 */

#ifndef __ACTIONS_ARCH_COMMON_H__
#define __ACTIONS_ARCH_COMMON_H__

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/afinfo.h>

#include <asm/arch/actions_arch_funcs.h>

/* NULL */
#ifndef NULL
#define NULL 0
#endif

/* ACT BOOT MODE */
#define ACTS_BOOT_MODE_NORMAL       0
#define ACTS_BOOT_MODE_RECOVERY     1
#define ACTS_BOOT_MODE_CHARGER      2

/* ACT BOOT DEVICE */
#define ACTS_BOOTDEV_NAND       (0x00)
#define ACTS_BOOTDEV_SD0        (0x20)
#define ACTS_BOOTDEV_SD1        (0x21)
#define ACTS_BOOTDEV_SD2        (0x22)
#define ACTS_BOOTDEV_SD02SD2    (0x31)   //emmc for cardburn



#endif /* __ACTIONS_ARCH_COMMON_H__ */
