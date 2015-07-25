/*
********************************************************************************
*                       USDK213X
*
*   Nand Flash Base Module
*
*File:  nand_flash.h
*By:    mengzh
*Date:  2006-8-3 11:33
*ver:   0.1
********************************************************************************
*/
#ifndef __PHY_SIM__
#define __PHY_SIM__

#include "nand_flash_driver_type.h"

#include "flash_driver_oal.h"

#ifdef NS_USE_FILE
#include <stdio.h>
#endif

/*
#ifndef UINT64
typedef unsigned long long    UINT64;
#endif
*/
typedef int (*HOOK_FUNCPTR)(void);


/*!
 * \brief  NAND Simulator Page
 *
 */
struct NS_Page
{
    UINT8 *Data;         /*!< The main area data in page */
    UINT8 *Spare;        /*!< The spare area data in page */
    UINT8 Empty;         /*!< Is this page empty? */
};

/*!
 * \brief  NAND Simulator Block
 *
 */
struct NS_Block
{
    struct NS_Page *Pages;
    UINT32 EraseCnt;
    UINT32 LastWritePage;         /*!< The last writed page in block */
};

/*!
 * \brief  NAND Simulator Device
 *
 */
struct NS_Chip
{
    struct NS_Block *Blocks;
    UINT32 TotalEraseCnt;

#ifdef NS_USE_FILE
    FILE *DataFp;
    FILE *SpareFp;
#else
    UINT8 *DataBuffer;
    UINT8 *SpareBuffer;
#endif
};

/*!
 * \brief  NAND Simulator Device
 *
 */
struct NS_Device
{
    struct NS_Chip Chips[NAND_MAX_CHIP_NUM];
    UINT32 CurrentChipCE;

    UINT32 BlocksPerChip;
    UINT32 PagesPerBlock;
    UINT32 DataBytesPerPage;
    UINT32 SpareBytesPerPage;
};

#define NS_STATE_PAGEREAD           0x1
#define NS_STATE_PAGEPROGRAM        0x2
#define NS_STATE_BLOCKERASE         0x4


#define NS_CUR_CHIP                 (&NS_Dev.Chips[NS_Dev.CurrentChipCE])

#define NS_GET_BLOCK_PTR(Block) \
    (&NS_CUR_CHIP->Blocks[(Block)])

#define NS_GET_PAGE_PTR(Block, Page) \
    (&NS_CUR_CHIP->Blocks[(Block)].Pages[(Page)])

#define NS_GET_BLOCK_PTR_BY_PAGE_NUM(Page) \
    ((NS_Dev.PagesPerBlock != 0) ? \
    (NS_GET_BLOCK_PTR((Page) / NS_Dev.PagesPerBlock)) \
    : NULL)

#define NS_GET_PAGE_PTR_BY_PAGE_NUM(Page) \
    ((NS_Dev.PagesPerBlock != 0) ? \
    (NS_GET_PAGE_PTR((Page) / NS_Dev.PagesPerBlock, (Page) % NS_Dev.PagesPerBlock)) \
    : NULL)


#ifdef NS_USE_FILE
#define PHY_WRITE_SECTOR(Page, Sector, DataPtr, SparePtr) \
    FILE_WriteSector(Page, Sector, DataPtr, SparePtr)

#define PHY_READ_SECTOR(Page, Sector, DataPtr, SparePtr) \
    FILE_ReadSector(Page, Sector, DataPtr, SparePtr)
#else
#define PHY_WRITE_SECTOR(Page, Sector, DataPtr, SparePtr) \
    MEM_WriteSector(Page, Sector, DataPtr, SparePtr)

#define PHY_READ_SECTOR(Page, Sector, DataPtr, SparePtr) \
    MEM_ReadSector(Page, Sector, DataPtr, SparePtr)
#endif

extern struct NS_Device NS_Dev;
extern unsigned int ns_chipid;
extern unsigned int ns_chipcnt;
extern int ReserveDeviceMemory;

extern HOOK_FUNCPTR hook_funcptr_before_create_table;
extern HOOK_FUNCPTR hook_funcptr_after_create_table;
extern HOOK_FUNCPTR hook_funcptr_after_ftl_init;


#endif /* __PHY_SIM__ */

