/********************************************************************************
*                              GL5202
*                            Module: BROM
*                 Copyright(c) 2008-2012 Actions Semiconductor,
*                            All Rights Reserved. 
*
* History:         
*      <author>    <time>           <version >             <desc>
*       YuJing     2010-03-01          1.0             build this file 
*       YuJing     2011-12-29          1.1             modify for 5002 from 5009
*       YuJing     2012-05-02          1.2             modify 
********************************************************************************/
/*!
* \file     adfu_brom_version.h
* \brief    adfu中PID等brom版本信息地址定义
* \author   YuJIng
* \version  1.0
* \date  2011/12/29
*******************************************************************************/

#ifndef __ADFU_BROM_VERSION_H__
#define __ADFU_BROM_VERSION_H__

//brom version information
#define     BROM_RELEASEVERSION            0xffff0808
#define     BROM_DEBUGVERSION              0xffff080a
#define     BROM_RELEASEDATA               0xffff080c
#define     BROM_BROMVID                   0xffff0810
#define     BROM_BROMPID                   0xffff0812
#define     BROM_BACKUPID                  0xffff0820
#define     BROM_ICVERSIONINDEX            0xffff0850
#define     BROM_ICVERSIONTBL              0xffff0854



#endif /* __ADFU_BROM_VERSION_H__ */
