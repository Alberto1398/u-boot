/********************************************************************************
*                              NAND FLASH DRIVER MODULE
*                            Module: logic.h
*                 Copyright(c) 2001-2008 Actions Semiconductor,
*                            All Rights Reserved.
*
* History:
*      <author>    <time>           <version >             <desc>
* nand flash group    2007-10-16 9:33     1.0             build this file
********************************************************************************/

#ifndef _LOGIC_H_
#define _LOGIC_H_

#include "nand_flash_driver_type.h"

#ifdef OS_UCOS_KERNEL_DRIVER
#include "pm.h"
#include "ucos/os.h"
#endif

extern UINT16 _GetCheckSum(UINT16 *buf,UINT32 len);
extern UINT32  FTL_Init(void);
extern UINT32  FTL_Exit(void);
extern UINT32  FTL_FlushPageCache(void);
extern UINT32  FTL_Read(UINT32 Lba, UINT32 Length, void* SramBuffer);
extern UINT32  FTL_Write(UINT32 Lba, UINT32 Length, void* SramBuffer);

extern UINT32 FTL_LogicPageRead(UINT32 BufPageNum, UINT32 Bitmap, void * Buf);
extern UINT32 FTL_LogicPageWrite(UINT32 BufPageNum, UINT32 Bitmap, void * Buf);
extern UINT32  FTL_CalPhyOpPar(struct PhysicOpParameter *PhyOpPar, UINT32 DieNum, UINT32 SupBlkNum, UINT32 PageNum);
extern UINT32  FTL_BadBlkManage(struct DataBlkTblType *NewFreeBlk,struct DataBlkTblType *BadBlk, UINT32 ErrPage );
extern UINT32 _CloseLastWritePage(void);
extern UINT32  _GetFreeBlk(UINT32 *FreeBlkPst, UINT32 GetType);
extern void  _WriteBadBlkFlag(UINT32 ZoneNum, UINT32 SuperBlkNum);
extern UINT32 PMM_InitPageMapTblCache(void);
extern UINT32 BMM_InitBlkMapTblCache(void);
extern UINT32 BMM_ExitBlockMapTblCache(void);
extern UINT32 PMM_ExitPageMapTblCache(void);
extern UINT32  BMM_WriteBackAllMapTbl(void);
extern UINT32  BMM_SwitchBlkMapTbl(UINT8 ZoneNum);
extern void PMM_CalAccessCount(void);
extern void BMM_CalAccessCount(void);
extern UINT32 BMM_TrySetDirtyFlag(void);
extern UINT32  PMM_SwitchPageMapTbl(UINT32 LogicBlockNum);
extern UINT32 FTL_MergeLogBlk(UINT8 type);
extern void kmemset(void* Buf, unsigned char Value, unsigned long Length);
#if SUPPORT_DMA_TRANSFER_DATA_ON_RAM
extern void DmaTransferDataForRam(void* SrcAddr, void* DstAddr, UINT32 ByteCnt, UINT32 DMAChannel);
#endif

#if SUPPORT_READ_RECLAIM
extern UINT32 FTL_ReadReclaim(UINT32 BufPageNum, UINT32 Bitmap, void * Buf);
#endif

#ifdef OS_UCOS_KERNEL_DRIVER
extern void * malloc_dma(unsigned int size);
extern void free_dma(void * ptr);
extern int nand_flash_exit(void);
extern int nand_pm_adjust_clock(unsigned int clk, curclk_t *curclk,
    void *dev_id, unsigned int cmd, unsigned int direction);

extern int check_nand_status(void);
#endif

extern UINT8 glb_blk_num_to_zone_num(UINT32 glb_blk_num);
extern UINT32 glb_blk_to_local_blk(UINT32 glb_blk_num);
extern UINT8 glb_zone_num_to_zone_num_in_die(UINT8 glb_zone_num);
extern UINT8 glb_zone_num_to_die_num(UINT8 glb_zone_num);

extern void kmemset(void* Buf, unsigned char Value, unsigned long Length);
extern void kmemcpy(char* desti_buffer, char* source_buffer, int buffer_size);

extern UINT32 volatile PhyBlkEraseCnt;
extern struct NandLogicCtlPar  LogicRWCtlPar;
extern struct SpareData WriteSpareData[2];

#ifdef OS_UCOS_KERNEL_DRIVER
extern  os_event_t  *NAND_DRIVER_SEM;
extern  UINT8 NAND_DRIVER_SEM_ERR;
extern UINT32 Logic_Partitions_Cap[16];
extern UINT32 flash_driver_api_table[];
#endif

extern UINT32 Partitions_Offset[16];
extern int bflash_end_data;
extern int bflash_start;
extern int bflash_end;
extern unsigned int _fksym,_eksym;
extern struct FIB_INFO	fib_info;

extern int init_loglayer(void);
extern void  exit_loglayer(void);

extern int ftl_pre_read(UINT32 start_page, int count);
extern int _ftl_pre_read(UINT32 start_page, int count);

#ifdef NAND_W_CACHE_EN
extern INT32 NAND_CacheWrite(UINT32 blk, UINT32 nblk, void *buf);
extern INT32 NAND_CacheRead(UINT32 blk, UINT32 nblk, void *buf);
extern INT32 NAND_CacheOpen(void);
extern INT32 NAND_CacheClose(void);
extern INT32 NAND_CacheFlush(void);
extern void NAND_CacheFlushRange(UINT32 sector, UINT32 n_sectors);
#else
static inline INT32 NAND_CacheWrite(UINT32 blk, UINT32 nblk, void *buf){return 0;}
static inline INT32 NAND_CacheRead(UINT32 blk, UINT32 nblk, void *buf){return 0;}
static inline INT32 NAND_CacheOpen(void){return 0;}
static inline INT32 NAND_CacheClose(void){return 0;}
static inline INT32 NAND_CacheFlush(void){return 0;}
static inline void NAND_CacheFlushRange(UINT32 sector, UINT32 n_sectors){}
#endif /* NAND_W_CACHE_EN */

#endif

