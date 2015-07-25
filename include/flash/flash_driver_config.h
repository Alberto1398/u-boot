/********************************************************************************
*							  USDK130
*							ucOS + MIPS,Nand Module
*				 Copyright(c) 2001-2008 Actions Semiconductor,
*							All Rights Reserved.
*
* History:
*	  <author>	<time>		   <version >			 <desc>
*	   mengzh	 2007-10-17 9:22	 1.0			 build this file
********************************************************************************/

#ifndef _FLASH_DRIVER_CONFIG_
#define _FLASH_DRIVER_CONFIG_

#define KERNEL_DRIVER

#include "flash_driver_config_default.h"
#include "event_log.h"

#define NAND_DMA_CHANNEL_NUM							 (0)
#define NAND_DMA_CHANNEL1_NUM							(4)
#define NAND_DMA_CHANNEL2_NUM							(5)


//#define PHY_DBG_MESSAGE_ON
#ifdef PHY_DBG_MESSAGE_ON
#define		PHY_DBG(x...)		   PRINT(x)
#else
#define	 PHY_DBG(x...)			do{}while(0)
#endif

#define PHY_INFO_MESSAGE_ON
#ifdef PHY_INFO_MESSAGE_ON
#define		PHY_INFO(x...)		  PRINT("[NAND]" x)
#else
#define	 PHY_INFO(x...)			do{}while(0)
#endif

#define PHY_ERR_MESSAGE_ON
#ifdef PHY_ERR_MESSAGE_ON
#define	 PHY_ERR(x...)		   PRINT("[NAND]" x)
#else
#define	 PHY_ERR(x...)			do{}while(0)
#endif

#define FTL_ERR_MESSAGE_ON
#ifdef FTL_ERR_MESSAGE_ON
//#define		FTL_ERR(x...)			LOG_INFO(x)
#define		FTL_ERR(x...)			PRINT("[NAND]" x)
#else
#define	 FTL_ERR(x...)			do{}while(0)
#endif

//#define FTL_DBG_MESSAGE_ON
#ifdef FTL_DBG_MESSAGE_ON
#define	 FTL_DBG(x...)		   PRINT(x)
#else
#define	 FTL_DBG(x...)			do{}while(0)
#endif

//#define	 F_PRINTF(x...)		   PRINT(x)
#define	 F_PRINTF(x...)			do{}while(0)

/*test flash speed macro*/
//#define TEST_FLASH_SPEED

//#define INIT_DBG_MESSAGE_ON
#ifdef INIT_DBG_MESSAGE_ON
#define	 INIT_DBG(x...)		  PRINT(x)
#else
#define	 INIT_DBG(x...)			do{}while(0)
#endif

#define INIT_INFO_MESSAGE_ON
#ifdef INIT_INFO_MESSAGE_ON
#define	 INIT_INFO(x...)		  PRINT("[NAND]" x)
#else
#define	 INIT_INFO(x...)			do{}while(0)
#endif

#define INIT_ERR_MESSAGE_ON
#ifdef INIT_ERR_MESSAGE_ON
#define	 INIT_ERR(x...)		  PRINT("[NAND]" x)
#else
#define	 INIT_ERR(x...)			do{}while(0)
#endif

#if (SUPPORT_EVENT_LOG)
#define EVENT_ERR_MESSAGE_ON
#define EVENT_INFO_MESSAGE_ON
//#define EVENT_DBG_MESSAGE_ON
#endif /*SUPPORT_EVENT_LOG*/

#ifdef EVENT_ERR_MESSAGE_ON
#define	 EVENT_ERR(x...)		  PRINT("[NAND]" x)
#else
#define	 EVENT_ERR(x...)			do{}while(0)
#endif /*EVENT_ERR_MESSAGE_ON*/


#ifdef EVENT_DBG_MESSAGE_ON
#define	 EVENT_DBG(x...)		  PRINT(x)
#else
#define	 EVENT_DBG(x...)			do{}while(0)
#endif /*EVENT_DBG_MESSAGE_ON*/


#ifdef EVENT_INFO_MESSAGE_ON
#define	 EVENT_INFO(x...)		  PRINT("[NAND]" x)
#else
#define	 EVENT_INFO(x...)		do{}while(0)
#endif /*EVENT_INFO_MESSAGE_ON*/


#define RESERVED_SIZE				(8 * 1024 * 1024) ////in unit Byte

#define FREE_BLK_CNT_PER_ZONE		 60
#define BOOT_CODE_AREA_BLK_CNT		4
#define BOOT_BLK_REV_CNT			  4

#ifdef OS_LINUX
#define NAND_BLK_MAX_SEGMENTS 	(BLK_MAX_SEGMENTS + 1)
#define NAND_USE_CTL_MUTEX
//#define NAND_USE_CTL_SPINLOCK

#define NAND_CONFIG_SYNC_INTF
#define OS_LINUX_IRQDMA
#define OS_LINUX_RBIRQ

#define NAND_W_CACHE_EN

#else

#define DMA_FROM_DEVICE 1
#define DMA_TO_DEVICE   0

#endif

#define NAND_USE_READ_ECC_ACC
#define NAND_USE_READ_ECC_DECODE_ACC

//#define NAND_USE_WRITE_ECC_ACC
//#define NAND_USE_WRITE_ECC_DECODE_ACC


#endif//the end of #ifndef _FLASH_DRIVER_CONFIG_
