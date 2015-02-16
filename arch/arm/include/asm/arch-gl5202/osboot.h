/***************************************************************************************
*                    GL5202 BROM include file-osboot.h
*                (c) Copyright 2012, Actions Co,Ld. 
*                        All Right Reserved 
*
* Description: Memory Address Definition
*
* History     :
*      <author>     <time>       <version >      <description>
*       GJ        2012/02/01       V1.0         create this file 
*       GJ        2012/03/01       V1.1         BROM Address modify
*       GJ        2012/03/08       V1.2         BROM Address modify
*       GJ        2012/06/15       V1.3         ADD SRAM_CPU0_SP_USR and SRAM_CPU0_SP_SVC Address 
******************************************************************************************/
/*测试宏定义*/ 
/* #define brom_debug */

#ifndef __OSBOOT_H__
#define __OSBOOT_H__

#include <config.h>     /* include/config.h */

#if !defined(CONFIG_ACTS) || !defined(CONFIG_ACTS_GL520X)
#error "CONFIG_ACTS not defined!"
#endif

#if defined(CONFIG_ACTS_GL5202)
#include "asm/arch/7029/osboot_7029.h"
#elif defined(CONFIG_ACTS_GL5207)
#include "asm/arch/7021/osboot_7021.h"
#endif

#endif  /*  __OSBOOT_H__ */
