/********************************************************************************
*                              NAND FLASH DRIVER MODULE
*                            Module: flash_driver_oal.h
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved.
*
* History:
*      <author>    <time>           <version >             <desc>
* nand flash group    2007-10-16 9:33     1.0             build this file
********************************************************************************/

#ifndef __FLASH_ID_H__
#define __FLASH_ID_H__

#include "types.h"

//#define SUPPORT_SHOW_FLASH_NAME
#define NAND_CHIPID_LENGTH              (8)     /* nand flash chipid length */

struct FlashChipInfoTblHeader
{
    UINT16 w_length;                               /* File length (Bytes) */
    UINT8  b_head_length;                          /* File header length (sectors), now fixed at 1 */
    UINT8  b_type;                                 /* File header type£¬now fixed at 9 */
    UINT8  b_id_len;                               /* flash id item length, now fixed at 64 bytes */
    UINT8  b_infolen;                              /* flash info item length, now fixed at 64 bytes */
    UINT8  versiosn;                               /* id define verion */
    UINT8  rsv1[0x10-7];                           /* pad to 16 bytes */
    UINT8  merge_bitmap[0x10];                     /* mergeµÄbitmap, every bit for a bytes in item,
                                                 * '0' means reserve the old value
                                                 * '1' means set to the new value
                                                 */
    UINT8  rsv2[0x200-0x10-0x10];
};

struct SpecialCmdType{
    UINT8      MultiProgCmd[2];                    /* the command for multi-plane program */
    UINT8      MultiCopyReadCmd[3];                /* the command read for multi-plane copyback */
    UINT8      MultiCopyProgCmd[3];                /* the command program for multi-plane copyback */
    UINT16     MultiBlkAddrOffset;                 /* the offset of two blocks for multi-plane */
    UINT8      BadBlkFlagPst;                      /* the bad block flag position, */
    UINT8      ReadMultiOpStatusCmd;               /* the command for multi-plane operation */
    UINT8      InterChip0StatusCmd;                /* the command to read inter-chip0 status for interleave */
    UINT8      InterChip1StatusCmd;                /* the command to read inter-chip1 status for interleave */
    UINT8      Reserved[2];
};


struct FlashChipInfo
{
    UINT8       ChipID[NAND_CHIPID_LENGTH];        /* nand flash id */
    UINT8       DieCntPerChip;                     /* the die count in a chip */
    UINT8       SectNumPerPPage;                   /* page size, based on sector */
    UINT8       SpareBytesPerSector;               /* spare bytes per sector */
    UINT8       Frequence;                         /* frequence parameter, may be based on xMHz */
    UINT16      PageNumPerPBlk;                    /* the page number of physic block */
    UINT16      BlkNumPerDie;                      /* total number of the physic block in a die */
    UINT16      DefaultLBlkNumPer1024PBlk;         /* Logical number per 1024 physical block */
    UINT16      ECCBits;                           /* ECCBits */
    UINT32      OperationOpt;                      /* the operation support bitmap,
                                                 * the upper two bytes are campatible for fw upgrade
                                                 * the lower two bytes are not campatible for fw upgrade
                                                 */
    UINT8       OpType[8];                      	/* [0]: muti plane read type, [1]: muti plane erase type*/

    struct SpecialCmdType SpecialCmdOp;         /* special operation struct, 16 bytes */

    UINT8       Reserved2[13];                     /* reserved */

    UINT8       Version;                           /* flash chipinfo version */
    UINT8       CENum;                             /* default is 0, filled by HWSC */
    UINT8       FlashType;                         /* nand flash type: 321/644/LBA,etc. */

    /* offset: 64 */
#ifdef  SUPPORT_SHOW_FLASH_NAME
    UINT8       FlashMark[64];                     /* reserved */
#endif
} __attribute__ ((packed));

/* NAND Flash Vendor ID */
#define NAND_MFR_SANDISK				0x45
#define NAND_MFR_TOSHIBA                0x98
#define NAND_MFR_SAMSUNG                0xec
#define NAND_MFR_HYNIX                  0xad
#define NAND_MFR_MICRON                 0x2c
#define NAND_MFR_ST                     0x20
#define NAND_MFR_INFINEON               0xc1
#define NAND_MFR_INTEL                  0x89
#define NAND_MFR_RENESSAS               0x07
#define NAND_MFR_PFC                    0x92


/* Operation Options Bitmap, upgrade incompatible options */
#define OPT_MULTI_PAGE_WRITE            0x0001
#define OPT_MULTI_PAGE_READ             0x0002
#define OPT_INTERNAL_INTERLEAVE         0x0004
#define OPT_EXTERNAL_INTERLEAVE         0x0008
#define OPT_SMALL_BLOCK                 0x0010
#define OPT_ONFI_10                     0x0040
#define OPT_ONFI_20                     0x0080
#define OPT_DOWNGRADE_FLASH             0x0100
#define OPT_ECC_REQ_12BITS              0x0200

/* Operation Options Bitmap, upgrade compatible options */
#define OPT_PAGE_CACHE_PROGRAM          (0x8000U << 16)
#define OPT_PAGE_CACHE_READ             (0x4000U << 16)
#define OPT_PAGE_COPYBACK               (0x2000U << 16)
#define OPT_NEED_READ_RETRY             (0x0008U << 16) /* 2010-5-27 */
#define OPT_NEED_RANDOMIZER             (0x0004U << 16) /* 2010-4-19 */
#define OPT_ALIGN_PAGE                  (0x0002U << 16) /*2009-11-12 */
#define OPT_ALIGN_BLOCK                 (0x0001U << 16) /*2009-11-12 */
#define OPT_BUILD_IN_ECC                (0x0010U << 16) /*2011-2-28 BUILD IN ECC */

#define OPT_NEED_EXTRA                	(0x0020U << 16)


/* ECC bits base */
#define ECCBITS_PER_512B                (0x00U << 8)
#define ECCBITS_PER_1KB                 (0x02U << 8)

/* Bad block falg postion */
#define BBF_POS_PAGE0                   (0x0)
#define BBF_POS_PAGE0_1                 (0x1)
#define BBF_POS_LASTPAGE0               (0x2)
#define BBF_POS_LASTPAGE0_1             (0x3)
#define BBF_POS_LASTPAGE0_2             (0x4)

/* ChipInfoFile Version define */
#define FLASH_CHIP_INFO_VERSION         (0x02)

/* FlashType define */
/* major types */
#define NAND_INTERFACE_TYPE_SHIFT       (0x6)
#define NAND_INTERFACE_TYPE_MASK        (0x3 << NAND_INTERFACE_TYPE_SHIFT)
#define __INTERFACE_SDR                   (0) /*the same sequence with FSM regs*/
#define __INTERFACE_DDR                   (1)
#define __INTERFACE_TOGGLE                (2)
#define NAND_INTERFACE_SDR              (__INTERFACE_SDR << NAND_INTERFACE_TYPE_SHIFT)
#define NAND_INTERFACE_DDR              (__INTERFACE_DDR << NAND_INTERFACE_TYPE_SHIFT)
#define NAND_INTERFACE_TOGGLE           (__INTERFACE_TOGGLE << NAND_INTERFACE_TYPE_SHIFT)
/* minor types */
#define FLASH_TYPE_SMALL_BLOCK          (0x00)
#define FLASH_TYPE_SPI_NOR              (0x01)
#define FLASH_TYPE_LARGE_BLOCK          (0x02)
#define FLASH_TYPE_CARD                 (0x03)
#define FLASH_TYPE_LBA_NAND             (0x0c)
/*---------------------------------------------------------*/
/* deprecate types, don't use in new coding */
#define FLASH_TYPE_READ_RETRY           (0x0d)
#define FLASH_TYPE_READ_RETRY_SAMSUNG   (0x0d)   //samsung
#define FLASH_TYPE_READ_RETRY_HYNIX     (0x0e)   //hynix
/*---------------------------------------------------------*/

enum {
	MUTI_READ_606030 = 1,
	MUTI_READ_000030,
	MUTI_READ_00320030,
};

enum {
	MUTI_SELECT_00 = 1,
	MUTI_SELECT_06E0,
};

enum {
	MUTI_ERASE_6060D0 = 1,
	MUTI_ERASE_60D160D0,
};

#define GET_NAND_TYPE(major, minor) ((major << NAND_INTERFACE_TYPE_SHIFT) | (minor & (~NAND_INTERFACE_TYPE_MASK)))

#include "extra_nand_id_tbl.h"

#endif /* __FLASH_ID_H__ */
