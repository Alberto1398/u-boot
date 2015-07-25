/********************************************************************************
*							  NAND FLASH DRIVER MODULE
*							Module: flash_driver_oal.h
*				 Copyright(c) 2001-2008 Actions Semiconductor,
*							All Rights Reserved.
*
* History:
*	  <author>	<time>		   <version >			 <desc>
* nand flash group	2007-10-16 9:33	 1.0			 build this file
********************************************************************************/

#ifndef __OAL_H__
#define __OAL_H__

#ifdef PC_NANDSIM
#include <stdio.h>
#include <stdlib.h>

#define FREE(x)							 free(x)
#define MALLOC(x)						 malloc(x)
#define MEMSET(x,y,z)					 memset(x,y,z)
#define MEMCPY(x,y,z)					 memcpy(x,y,z)
#define PRINT(x...)						 printf(x)
#define MEMCMP(s1, s2, count)			 memcmp(s1, s2, count)

#define STRCAT(x,y)						strcat(x,y)
#define SPRINTF(x...)					sprintf(x)
#define RAND()							rand()
#define SRAND(x)						srand(x)

#define NULL							((void *)0)

#else   /* PC_NANDSIM */


#define SPECIAL_DMA_START	   		0x04
#define ISSPECIALDMA(nDMANum)			((nDMANum) >= SPECIAL_DMA_START)


#ifdef OS_LINUX

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>


#define FREE(x)						 	kfree(x)
#define MALLOC(x)					   	kmalloc(x, GFP_KERNEL|__GFP_REPEAT)
#define VMALLOC(x)						vmalloc(x)
#define VFREE(x)						vfree(x)
#define MEMSET(x,y,z)				   	memset(x,y,z)
#define MEMCPY(x,y,z)				   	memcpy(x,y,z)
#define PRINT(x...)					 	printk(x)
#define MEMCMP(s1, s2, count)			memcmp(s1, s2, count)

#define OS_INT_SAVE_AREA
#define OS_ENTER_CRITICAL()
#define OS_EXIT_CRITICAL()

#else
#ifdef MBREC_DRIVER
#include <string.h>
#endif
extern int printf(const char* fmt, ...);
extern void *malloc(unsigned int size);
extern void free(void*);

#define FREE(x)						 	free(x)
#define MALLOC(x)					   	malloc(x)
#define VMALLOC(x)						malloc(x)
#define VFREE(x)						free(x)
#define MEMSET(x,y,z)				   	memset(x,y,z)
#define MEMCPY(x,y,z)				   	memcpy(x,y,z)
#define PRINT(x, args...)				printf(x, ## args)
#define MEMCMP(s1, s2, count)			memcmp(s1, s2, count)

#ifndef	NULL
#define NULL				   			((void *)0)
#endif

#endif  /* OS_LINUX */

#endif /* PC_NANDSIM */

#endif /* __OAL_H__ */
