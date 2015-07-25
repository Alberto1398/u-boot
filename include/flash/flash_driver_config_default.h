/********************************************************************************
*                              NAND FLASH DRIVER MODULE
*                            Module: flash_driver_config.h
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved.
*
* History:
*      <author>    <time>           <version >             <desc>
* nand flash group    2007-10-16 9:33     1.0             build this file
********************************************************************************/

#ifndef __FLASH_DRIVER_CONFIG_H__
#define __FLASH_DRIVER_CONFIG_H__

#include "flash_driver_oal.h"

//#define TLC_SUPPORT
#undef TLC_SUPPORT

/* the max chip number that driver support */
#define NAND_MAX_CHIP_NUM                               (4)
/* the normal chip number that driver support */
#define NAND_NORMAL_CHIP_NUM                            (4)

/* the max die number that driver support */
#define NAND_MAX_DIE_NUM                                (8)

/* the max zone number in a die */
#define MAX_ZONE_NUM_IN_DIE                             (16)

/*the num of the DFB and FIB*/
#define	MAX_DFB_CNT					                    (4)
#define	MAX_FIB_CNT					                    (4)

/* zone table cache number that driver used */
#define ZONE_CACHE_NUM                                  (2)

/* page mapping table cache number that driver used */
#define LOG_CACHE_NUM                                   (16)

/* the max log block number that one zone contain */
#define MAX_LOG_BLK_NUM                                 (16) // max 32, because of SeqWriteFlag
#define MAX_LOG_BLK_NUM_PER_DATA						(2)

/* the block number of zone */
//#define BLK_NUM_PER_ZONE                                (1024)
#ifdef TLC_SUPPORT
#define BLK_NUM_PER_ZONE                                (BlkCntPerZone)
#else
#define BLK_NUM_PER_ZONE                                (1024)
#endif




/* max block number of zone */
#define MAX_BLK_NUM_PER_ZONE                            (1024)
/* the max free block number in one zone */
#define MAX_FREE_BLK_PER_ZONE                           (50)

/* the size of DFB data */
#define DFB_DATA_SIZE                                (SECTOR_NUM_PER_SINGLE_PAGE *NAND_SECTOR_SIZE )

/* the size of FIB data */
#define FIB_DATA_SIZE                               	(2048)
#define ZONE_TBL_ITEMS							        (1024)

/* the max data block number in one zone */
#define MAX_DATA_BLK_PER_ZONE                           (ZONE_TBL_ITEMS)


/* the default data block number in one zone */
#define DEFAULT_DATA_BLK_PER_ZONE                       (984)

/* if the driver need support multiplane program */
#define SUPPORT_MULTI_PLANE                             (1)

/* if the driver need support internal interleave */
#define SUPPORT_INTERNAL_INTERLEAVE                     (0)

/* if the driver need support external inter leave */
#define SUPPORT_EXTERNAL_INTERLEAVE                     (1)

/* if the driver need support read-reclaim */
#define SUPPORT_READ_RECLAIM                            (0)

/* if the driver need support wear-levelling */
#define SUPPORT_WEAR_LEVELLING                          (0)

/* if use dma to transfer data on ram */
#define SUPPORT_DMA_TRANSFER_DATA_ON_RAM                (0)

/* if support copy nand page with copy command */
#define SUPPORT_PAGE_COPYBACK                           (0)

/* if support small block flash */
#define SUPPORT_SMALL_BLOCK                             (0)

/* if support event log */
#define SUPPORT_EVENT_LOG								(0)

#ifdef KERNEL_DRIVER
/* if support use dma to transfer data from usb fifo to flash fifo */
#define SUPPORT_USB_DMA_DIRECT_WRITE                    (0)
#else
#define SUPPORT_USB_DMA_DIRECT_WRITE                    (0)
#endif  /* KERNEL_DRIVER */



#if (SUPPORT_USB_DMA_DIRECT_WRITE)
#define USB_FIFO_ADDRESS_WRITE               (0xB0290000 + 0x84)
#define USB_FIFO_ADDRESS_READ                (0xB0290000 + 0x88)
/* max address increase of one write is 128KB*/
#define IS_USB_FIFO_ADDRESS(addr) \
    (((unsigned int)(addr)>=(0xB0290000 -0x20000))  && \
    ((unsigned int)(addr)<(0xB0290000  + 0x20000)))
#define IS_USB_HS_MODE() \
    (*(volatile unsigned char *)(USB_USBCS) & (0x1 << 1))
#endif  /* SUPPORT_USB_DMA_DIRECT_WRITE */


#define SUPPORT_CACHE_PROGRAM							(0)

/* if support 12 bit ECC flash*/
#define SUPPORT_12BITS_ECC_FLASH                        (1)

/*if support down grade flash*/
#define SUPPORT_DOWN_GRADE_FLASH			            (1)

/* define after how many erase times should do one wear-levelling */
#define WEAR_LEVELLING_FREQUENCE                        (15)

#define NAND_SECTOR_SHIFT								9
/* define the nand flash sector size */
#define NAND_SECTOR_SIZE                                (1 << NAND_SECTOR_SHIFT)


/* define the nand flash spare area size, contain user data and ecc parity */
//#define SPARE_SIZE_PER_SECTOR                           	(16)
#define SPARE_SIZE_PER_SECTOR                           	(NandStorageInfo.SpareBytesPerSector)

/* define the spare size for store user data for ervery sector */
#define NAND_SPARE_SIZE                                 		(3)
/*define the byte count of user data in a page*/
#define	NAND_SPARE_COUNT						                (7)

/* page buffer size can be how many times of sigle page size, the value can only be 1 or 2 */
#define PAGE_BUFFER_SIZE                                (2)

#if ((PAGE_BUFFER_SIZE != 1) && (PAGE_BUFFER_SIZE != 2))
#error Configuration Error! Page buffer size config is valid!!!
#endif

/* boot chip number */
#define BOOT_CHIP_NUM                                   (0)

/* the sector nuber of a single page */
#define SECTOR_NUM_PER_SINGLE_PAGE                      (NandStorageInfo.SectorNumPerPage)

/* the sector number of a super page */
#define SECTOR_NUM_PER_SUPER_PAGE                       \
(NandStorageInfo.SectorNumPerPage * NandStorageInfo.PlaneCntPerDie)

#define PAGE_NUM_PER_SUPER_BLK \
	(NandStorageInfo.PageNumPerPhyBlk * NandStorageInfo.BankCntPerChip)

/*the spare area byte count per sector*/
#define SPARE_BYTES_CNT_PER_SECTOR                      (NandStorageInfo.SpareBytesPerSector)

/* the spare area start address in a page */
#define SPARE_AREA_START_ADDR                           (NAND_SECTOR_SIZE * SECTOR_NUM_PER_SINGLE_PAGE)

#if((PAGE_BUFFER_SIZE == 1))
#define PAGE_BUFFER_SECT_NUM                            (SECTOR_NUM_PER_SINGLE_PAGE)

#define OPCYCLE                                         (NandStorageInfo.PlaneCntPerDie)
/*times of phyical read or physical write function was called when read or write one super page*/

#endif

#if((PAGE_BUFFER_SIZE == 2))
#define PAGE_BUFFER_SECT_NUM                            (SECTOR_NUM_PER_SINGLE_PAGE * NandStorageInfo.PlaneCntPerDie)

#define OPCYCLE                                         (1)
/*times of phyical read or physical write function was called when read or write one super page*/

#endif

/*logic page must located in corresponding bank if 1.
for example: 2 bank , page (0,2,4...) - bank0, page(1,3...)-bank1 */
#define SUPPORT_ALIGN_BANK 					            (1)

/*restore  data from  bad block before delete it if 1*/
#define SUPPORT_BAD_BLOCK_DATA_SAVE			            (1)

/* add by linjingchao */
#define NAND_CHIPID_CHECK_LENGTH        (5)     /* nand flash chipid check length */

/*cache layer usage*/
#define NAND_CACHE_PAGE					(0)
#define NAND_CACHE_PBLRU				(1)
#define NAND_CACHE_BLK					(8)
#define NAND_CACHE_METHOD				(NAND_CACHE_BLK)

#endif //#ifndef __FLASH_DRIVER_CONFIG_H__

