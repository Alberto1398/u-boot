/********************************************************************************
*                              USDK130
*                            Module: TYPES
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved.
*
* History:
*      <author>    <time>           <version >             <desc>
*       hmwei     2008-6-12 9:42     1.0             build this file
********************************************************************************/

#ifndef   __NAND_FLASH_TYPES_H__
#define   __NAND_FLASH_TYPES_H__

/* define data type */
/* ================================================================ */
#ifndef UINT64
typedef unsigned long long    UINT64;
#endif
#ifndef UINT32
typedef unsigned int    UINT32;
#endif
#ifndef INT32
typedef signed int      INT32;
#endif
#ifndef  UINT16
typedef unsigned short  UINT16;
#endif
#ifndef INT16
typedef signed short    INT16;
#endif
#ifndef UINT8
typedef unsigned char   UINT8;
#endif
#ifdef  INT8
typedef signed char     INT8;
#endif

#ifndef s8
 typedef signed char s8;
#endif

#ifndef u8
 typedef unsigned char u8;
#endif

#ifndef s16
 typedef signed short s16;
#endif

#ifndef u16
 typedef unsigned short u16;
#endif

#ifndef s32
 typedef signed int s32;
#endif

#ifndef u32
 typedef unsigned int u32;
#endif

#ifndef s64
 typedef signed long long s64;
#endif

#ifndef u64
 typedef unsigned long long u64;
#endif

#endif /* __NAND_FLASH_TYPES_H__ */
