/*
********************************************************************************
*                       usdk130---Boot
*                (c) Copyright 2007-2009, Actions Co,Ld. 
*                        All Right Reserved 
*
* FileName: address.h     
* Author:   Liguoping
* Date:     2008.07.02
* Description:  address macro used in boot module
* Others:       
* History:     
*      <author>    <time>          <version >    <desc>
*      liguoping   2008.07.02      1.0         build this initial file
********************************************************************************
*/
#ifndef __ADDRESS_H__
#define __ADDRESS_H__

/* MBRC first stage */
#define SECTOR_SIZE			512

#define MBRC_SECTOR_SIZE		80 /* mbrc 2 + u-boot-spl 48 sectors */
#define MBRC_SIZE			(MBRC_SECTOR_SIZE * SECTOR_SIZE)

#define TRD_STAGE_MBRC_START		0x02000000   /*0x80E00000  start address of third mbrc stage ′?·??ú14Mμ?μ?·?*/

#define SEC_STAGE_NAND_OFFSET		0x400
#define MBRC_SECOND_STAGE_SIZE		78	     /* 48 sectors, 24KB */

#define AFINFO_BACK_OFFSET_IN_MBRC	0x800
#define AFINFO_OFFSET_IN_SEC_MBRC	(MBRC_SIZE - 0x400 - AFINFO_BACK_OFFSET_IN_MBRC)	//14K	
#define AFINFO_OFFSET_IN_MBRC_BIN	(MBRC_SIZE - AFINFO_BACK_OFFSET_IN_MBRC)
//#define AFINFO_OFFSET_IN_RAM		(AFINFO_OFFSET_IN_MBRC + 0x400)	//0x400~0x800用作其他用途

//#define FLASH_OFFSET_IN_AFINFO	0x100
//#define FLASH_BACK_OFFSET_IN_MBRC	(AFINFO_BACK_OFFSET_IN_MBRC-FLASH_OFFSET_IN_AFINFO)

#if defined(CONFIG_ATM7059TC) || defined(CONFIG_ATM7059A)
#define MBRC_START_IN_RAM	0xb4060000
#define SEC_MBRC_LOAD_IN_RAM	0xb4066000
#define SEC_MBRC_TEXT_IN_RAM	0xb4066900
#else
#define MBRC_START_IN_RAM	0xb4068000
#define SEC_MBRC_LOAD_IN_RAM	0xb406e000
#define SEC_MBRC_TEXT_IN_RAM	0xb406e900
#endif

#define PagesPerBlockinSB	64	/* pages per block in small block nand flash */
#define SectorsPerPageinSB	1	/* pages per block in small block nand flash */

/* BROM nand config address */
#define BRECLAUNCHER_DATA_OFFSET		0xb4060600
#define BRECLAUNCHER_DATA_SIZE			(48 + 32)

#define NAND_DATA_OFFSET			(BRECLAUNCHER_DATA_OFFSET + BRECLAUNCHER_DATA_SIZE) //0xb4060650

#define SRAM_FLASH_DISABLE_ECC_ADDR		(NAND_DATA_OFFSET)
#define SRAM_FLASH_LB_SPARE_START_BYTE_ADDR	(NAND_DATA_OFFSET + 2)

/* move brec and kernel to kseg0 */
//#define BREC_RUNNING_ADDRESS    0x81030000
//#define BREC_PHY_ADDRESS        0x1030000
//#define NAND_PHY_ADDRESS        0x1020000
//#define NAND_HEAP_PHY_ADDRESS   0x1100000
//#define NAND_RESTORE_BUF_START  0x1050000
//#define NAND_KERNEL_PHY_ADDRESS 0x0000000


//#define BREC_RUNNING_ADDRESS    0x80030000
//#define BREC_PHY_ADDRESS        0x00030000
//#define NAND_PHY_ADDRESS        0x00020000
//#define NAND_HEAP_PHY_ADDRESS   0x00100000
//#define NAND_RESTORE_BUF_START  0x00050000
//#define NAND_KERNEL_PHY_ADDRESS 0x01000000


#define BREC_RUNNING_ADDRESS	0x80104270
#define BREC_PHY_ADDRESS	0x00100000
#define BREC_PHY_HEAD_ADDRESS	0x80100000

#define NAND_PHY_ADDRESS	0x01020000
#define NAND_HEAP_PHY_ADDRESS	0x01100000
#define NAND_RESTORE_BUF_START	0x01050000
#define NAND_KERNEL_PHY_ADDRESS	0x00100000

#define BROM_ENTER_ADFU_ADDR	0xbfc00194

#define KERNEL_ADDRESS		0x00008000
#define PROBATCH_IMG_ADDR	0x30008000
										 
#endif /* __ADDRESS_H__ */
