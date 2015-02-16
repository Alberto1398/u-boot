/*
 * brom_drv.h
 *
 *  Created on: 2014-7-15
 *      Author: liangzhixuan
 */

#ifndef __BROM_DRV_H__
#define __BROM_DRV_H__

#include <linux/types.h>

extern int act_load_third_mbrc(void);
extern void act_brom_drv_exit(void);
extern void __attribute((noreturn)) act_enter_adfu_launcher(void);

#endif /* __BROM_DRV_H__ */
