/********************************************************************************
*							  NAND FLASH DRIVER MODULE
*							Module: phy_functions.h
*				 Copyright(c) 2001-2008 Actions Semiconductor,
*							All Rights Reserved.
*
* History:
*	  <author>	<time>		   <version >			 <desc>
* nand flash group  2007-6-11 10:30	 1.0			 build this file
********************************************************************************/

#ifndef _PHY_FUNCTIONS_H
#define _PHY_FUNCTIONS_H

#include "nand_flash_driver_type.h" /*放在include"all_kmod_calls.h"之前，其中对TRUE/FALSE的定义正好相反*/


//#include "upgrade_dev.h"	/* for FlashScanInfo_t */

#ifndef PC_NANDSIM
//#include "all_kmod_calls.h"
#endif

#ifndef OS_LINUX
extern void udelay(int num);   //2012-6-14 11:37
extern void mdelay(int num);
#endif

#ifndef KERNEL_DRIVER
extern struct NandDevPar   SamsungNandTbl[];
extern struct NandDevPar   HynixNandTbl[];
extern struct NandDevPar   ToshibaNandTbl[];
extern struct NandDevPar   MicronNandTbl[];
extern struct NandDevPar   IntelNandTbl[];
extern struct NandDevPar   StNandTbl[];
extern struct NandDevPar   RenessasNandTbl[];
extern struct NandDevPar   UserCreateNandTbl[];

extern struct SpecialCmdType SpecialType_Samsung_SLC_512;
extern struct SpecialCmdType SpecialType_Samsung_SLC_2K;
extern struct SpecialCmdType SpecialType_Samsung_SLC_4K;
extern struct SpecialCmdType SpecialType_Samsung_MLC_2K;
extern struct SpecialCmdType SpecialType_Samsung_MLC_4K;
extern struct SpecialCmdType SpecialType_Micron;
extern struct SpecialCmdType SpecialType_Toshiba_SLC_2K;
extern struct SpecialCmdType SpecialType_Toshiba_MLC_2K_1024;
#endif

extern UINT32 g_nand_ceconfig;
extern struct FlashChipInfo		NandChipInfo;
extern UINT32 ECCErrCheck;
extern UINT32 SeqWriteFlag[];
extern UINT8 ExceptionFlag;	 //2010-6-18 15:53
//extern struct ECCUnitParam ECCUnitParam_8bitsECC;
//extern struct ECCUnitParam ECCUnitParam_8bitsECC_BROM;
extern struct ECCUnitParam ECCUnitParam_24bitsECC;
extern struct ECCUnitParam ECCUnitParam_24bitsECC_TOSHIBA;
extern struct ECCUnitParam ECCUnitParam_24bitsECC_BROM;
extern struct ECCUnitParam ECCUnitParam_40bitsECC;
extern struct ECCUnitParam ECCUnitParam_40bitsECC_BROM;
extern struct ECCUnitParam ECCUnitParam_60bitsECC;
extern struct ECCUnitParam ECCUnitParam_60bitsECC_BROM;
extern struct ECCUnitParam ECCUnitParam_70bitsECC;
extern struct ECCUnitParam ECCUnitParam_70bitsECC_BROM;
extern struct ECCUnitParam ECCUnitParam_50bitsECC;
extern struct ECCUnitParam ECCUnitParam_50bitsECC_BROM;
extern struct ECCUnitParam ECCUnitParam_NoECC;
extern struct ECCUnitParam ECCUnitParam_NoECC_BROM;
extern void SetECCParam(struct ECCUnitParam *ECCParam);
extern void InitECCParam(struct StorageInfo *NandInfo);
extern void SetBusInterfaceType(UINT32 busType);		//2012-7-14 bus type
extern UINT32 GetBusInterfaceType(void);
extern UINT32 NandDmaParam;
extern UINT32 BlkCntPerZone;	//2010-11-15, tlc support
extern struct NandState_t NandState;

extern  UINT32  CheckResetResult(UINT32 ChipNum);
extern  UINT32  PHY_WaitNandFlashReady(UINT32 BankNum);
extern  UINT32  PHY_SyncNandOperation(UINT32 BankNum);
extern  UINT32  PHY_EraseNandBlk(struct PhysicOpParameter *NandOpPar);
extern  UINT32  PHY_PageRead(struct PhysicOpParameter *NandOpPar);
extern  UINT32  PHY_PageWrite(struct PhysicOpParameter *NandOpPar);
extern  UINT32  PHY_CopyNandPage(struct PhysicOpParameter *NandOpSrcPar, struct PhysicOpParameter *NandOpDstPar);
extern 	UINT32  PHY_EraseSuperBlk(struct PhysicOpParameter *PhyOpPar);
extern 	UINT32  PHY_CheckWriteProtect(UINT32 ChipNum);
extern 	UINT32  PHY_NandReset(UINT32 ChipNum);
extern 	UINT32  _ReadNandStatus(UINT32 ReadStatusCmd, UINT32 BankInChip);
//extern 	UINT32  PHY_ReadNandId(UINT32 ChipNum, void *NandChipID);
extern  UINT32  PHY_ReadNandId(UINT32 ChipNum, UINT32 ChipIDType, void *NandChipID, UINT32 Length);
extern 	void 	SettingNandFreq(UINT32 NandFreq, UINT32 CpuClock);
extern void nand_set_normal_freq(void);
extern int wait_fifo_ready_timeout(int us);
extern UINT32 phy_read_id(UINT32 ChipNum, UINT8 cmd, UINT8 addr, void *buf, UINT32 Length);
extern int decode_data(unsigned char * data, unsigned char * udata, unsigned char * parity);
//extern void r4k_dma_cache_wback_inv(unsigned long addr, unsigned long size);


extern  void _NandDmaReadConfig(UINT32 nDmaNum, void* Mem_Addr, UINT32 ByteCount);
extern  void _NandDmaWriteConfig(UINT32 nDmaNum, void* Mem_Addr, UINT32 ByteCount);

extern void inline nand_bus_to_axi(void);
extern void inline nand_bus_to_normal(void);

extern void panic_nand(void);
extern int	init_phylayer(void);
extern void	exit_phylayer(void);
extern int	init_tblcreat(void);
extern void	exit_tblcreat(void);
void dump_mem(void * startaddr, int size, unsigned int showaddr, int show_bytes);

extern  UINT32 wait_dma_stopped_with_timout(unsigned int timeout_ms);
extern  UINT32 _NandReleaseDevice(void);
extern  UINT32 _NandGetDevice(UINT32 ChipNum);
extern  UINT32 _RsEccCorrect(UINT8 *MBuf, UINT8 *SBuf);

//extern  int bch_ecc_correct(UINT32 ecc_type, UINT32 ud_byte_cnt_per_sector,UINT32 sector_read_cnt);
extern UINT32 bch_auto_ecc_correct(UINT32 sector_read_cnt);

extern  UINT32 _CalPhyBlkNumByBlkNumInBank(UINT32 *ChipNum, UINT32 *BlkNumInChip, UINT32 BankNum, UINT32 BlkNumInBank);

extern void kmemset(void* Buf, unsigned char Value, unsigned long Length);
extern void kmemcpy(char* desti_buffer, char* source_buffer, int buffer_size);

extern int nand_hw_init(void);
extern void nand_hw_exit(void);
//extern UINT32 INIT_ScanFlashInfo(FlashScanInfo_t *ScanInfo);
extern UINT32 INIT_ScanNandStorage(struct FlashChipInfo *ChipInfo, int scantime);
extern UINT32 CheckNandDriverArch(void);

#ifdef OS_LINUX
extern UINT32 nand_get_cap(UINT8 partition);
#else
extern UINT32 nand_get_cap(void);
#endif

extern UINT32 nand_get_phy_cap(void);

extern int nand_phy_erase_single(struct PhysicOpParameter *param);
extern int nand_phy_read_single_page(struct PhysicOpParameter *param);
extern int nand_phy_write_single_page(struct PhysicOpParameter *param);
extern int nand_phy_erase_all(int force);
extern int nand_scan_module_main(void);
extern unsigned int chars_to_userdata_reg(volatile UINT32 * startaddr, UINT8 * pSpare, UINT32 byte_cnt);
extern unsigned int userdata_reg_to_chars(volatile UINT32 * startaddr, UINT8 * pSpare, UINT32 byte_cnt);
extern UINT32 PHY_S_PageWrite(struct PhysicOpParameter *NandOpPar);
extern UINT32 PHY_S_PageRead(struct PhysicOpParameter *NandOpPar);

extern int PHY_PagePreRead(struct PhysicOpParameter *NandOpPar);

#endif
