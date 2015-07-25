/********************************************************************************
*							  NAND FLASH DRIVER MODULE
*							Module: Nand_Flash_Driver_Struct.h
*				 Copyright(c) 2001-2008 Actions Semiconductor,
*							All Rights Reserved.
*
* History:
*	  <author>	<time>		   <version >			 <desc>
* nand flash group  2007-6-11 10:30	 1.0			 build this file
********************************************************************************/

#ifndef _NAND_FLASH_DRIVER_STRUCT_H_
#define _NAND_FLASH_DRIVER_STRUCT_H_
#include "rr_nand_id_tbl.h"

#define  SUPPORT_READ_RETRY	1
#ifndef OS_LINUX

//#define  printk  adfu_printf
#define  printk
#endif

#include "flash_driver_config.h"
#include "types.h"
#include "flash_id.h"
#include "nand_dma.h"

#define PLATFORM_IS_32BIT

/* ================================================================ */

/* define result value */
/* ================================================================ */
#define NAND_OP_TRUE	(0)
#define NAND_OP_FALSE   (1)

#undef  TRUE
#undef  FALSE
#define TRUE			(0)
#define FALSE		   (1)

//#define NULL			(0)
/* ================================================================ */

#define DATA_BLK_BITS		14
#define ERASE_CNT_BITS		(16 - DATA_BLK_BITS)
#define INVALID_DATA_BLK	((0x1 << DATA_BLK_BITS) - 1)
#define MAX_ERASE_CNT		((0x1 << ERASE_CNT_BITS) - 1)

#define DATA_PAGE_BITS		12
#define LOG_INDEX_SHIFT		DATA_PAGE_BITS
#define INVALID_DATA_PAGE	((0x1 << DATA_PAGE_BITS) - 1)
#define PAGE_NUM(x)		((x) & INVALID_DATA_PAGE)
#define LOG_INDEX(x)	(((x) >> LOG_INDEX_SHIFT) & 0x3)
#define LOG_INDEX_MASK	(0x3 << LOG_INDEX_SHIFT)


/* define management data structure for nand driver */
/* ================================================================================================================== */
struct NandDev
{
	struct StorageInfo		  *NandFlashInfo;					 //nand system information
	struct FIB_INFO			*fib_info;			//FIB info
	struct ZoneInfoType		 *ZoneInfo;						  //zone information
	struct ZoneTblCacheInfo	 *ZoneTblCacheInfo;				  //the zone table cache information
	struct PageTblCacheInfo	 *PageTblCacheInfo;				  //the page mapping table cache information
	struct SpecialCmdType	   *SpecialCommand;					//nand flash multi-plane program command
	struct LogicOrganizeType	*LogicOrganizePar;				  //logic organise parameters
	void						*PageCache;						 //the pointer to page buffer
	void						*logic_mgmt_buffer;
};

struct StorageInfo
{
	UINT8					   ChipEnableInfo;					 //chip connect information
	UINT8					   ChipCnt;							//the count of chip current connecting nand flash
	UINT8					   BankCntPerChip;					 //the banks count in chip
	UINT8					   DieCntPerChip;					  //the die count in a chip
	UINT8					   PlaneCntPerDie;					 //the count of plane in one die
	UINT32						FirstBlockOfDie;
	UINT8					   SectorNumPerPage;				   //page size,based on 0.5k
	UINT16					  PageNumPerPhyBlk;				   //the page number of physic block
	UINT16					  AlignedPageNumPerPhyBlk;			//the page number of physic block
	UINT16					  BlkNumPerDie;				  //total number of the physic block in a die
	UINT32					  OperationOpt;					   //the operation support bitmap
	UINT8					   FrequencePar;					   //frequence parameter,may be based on xMHz
	UINT8					   NandChipId[5];					  //nand flash id
	UINT8					   SpareBytesPerSector;				//spare bytes per sector
	UINT8					   Reserved;
	UINT32					  UserDataSectorBitmap;			   // the user data sector bitmap
	UINT32					  NandCEConfig;					   //the page number of physic block
	UINT32					  ECCUnitNumPerPage;				  //number of ecc unit per page
	struct ECCUnitParam			*SysECCUnitParam;
	struct ECCUnitParam			*BromECCUnitParam;
	struct rr_flash_attr_info_s RrStorageInfo;
	struct extra_flash_attr_info_s ExtraStorageInfo;
};


struct ZoneTblCacheType
{
	UINT8					   DieNum;								//the number of the DIE which this zone is in
	UINT8					   ZoneNum;					   		//the number of the zone which is cached in ram
	UINT8					   LogBlkNum;						  //the number of Log blocks in current cached table
	UINT8					   FreeBlkNum;						 //the number of free blocks in current cached table
	UINT8					   TblNeedUpdate;					  //the flag,marked if table need be updated to nand
	UINT16 			   AccessCount;
	struct DataBlkTblType	   *DataBlkTbl;						//the pointer to data block table
	struct LogBlkTblType		*LogBlkTbl;						 //the pointer to Log block table
	struct DataBlkTblType	   *FreeBlkTbl;						//the pointer to Free block table
};

struct ZoneTblCacheInfo
{
	struct ZoneTblCacheType	 *CurZoneTbl;						//the pointer to the current operation zone table
	struct ZoneTblCacheType	 ZoneTblCache[ZONE_CACHE_NUM];	   //the array of the zone table
};


struct PageMapTblCacheType
{
	//the flag which marked if the page mapping table need be updated to nand
	UINT8					   TblNeedUpdate;
	//the zone number of the page mapping table belonged to
	UINT8					   ZoneNum;
	//the log block number in Log blockk tabel of the page mapping table belonged to
	UINT16					  LogBlkNum;
	UINT16						AccessCount;
	//the pointer to page mapping table
	struct PageMapTblType	   *PageMapTbl;
};

struct PageTblCacheInfo
{
	struct PageMapTblCacheType  *CurPageTbl;						//the pointer to current operation pagemaptbl
	struct PageMapTblCacheType  PageMapTblCache[LOG_CACHE_NUM];	 //the array of the pagemaptbl
};

struct LogicOrganizeType
{
	UINT16					  DataBlkNumPerZone;				  //the number of data blocks in one zone
	UINT16					  PageNumPerLogicBlk;				 //the page number of logic block
	UINT8					   SectorNumPerLogicPage;			  //the sector number per super page
	UINT8					   ZoneNumPerDie;					  //the zone number per die
	UINT32					DieCntPerBank;
	UINT32					BankCntPerDie;
	UINT32					BlocksPerDie;
	UINT32					TotalDies;
	UINT32					TotalBanks;
	UINT32					ChipsToInterleave;
#ifdef  PLATFORM_IS_32BIT
	UINT8					   Reserved[2];						//reserved
#endif
};

struct ZoneInfoType
{
	UINT16					  PhyBlkNum;						  //the block No. whice saved zone table in a chip
	UINT16					  TablePosition:12;				   //the page No. which saved zone table
	UINT16					  Reserved:4;						 //reserved
};

struct DataBlkTblType
{
	UINT16					  PhyBlkNum:DATA_BLK_BITS;			 //physical block No.mapping to the logical block
	UINT16					  EraseCnt:ERASE_CNT_BITS;			 //erase count of the physic block
}__attribute__((packed));


struct LogBlkTblType
{
	UINT16					  LogicBlkNum;						//the logic block number of the log block
	struct DataBlkTblType	   PhyBlk;						  //the physic block number of the log block
	struct DataBlkTblType	   PhyBlk_log[MAX_LOG_BLK_NUM_PER_DATA];
	UINT16                    ValidPage;
	UINT16					  LastUsedPage:DATA_PAGE_BITS;	 //the number of the page which used last time
	UINT16					  log_index:2;
	UINT16					  reserved:(16 - 2 - DATA_PAGE_BITS);
}__attribute__((packed));


struct PageMapTblType
{
	UINT16					  PhyPageNum:DATA_PAGE_BITS;		 //the physic page number for the logic page
	UINT16					  log_index:2;
	UINT16					  Reserved:(16 - DATA_PAGE_BITS - 2);//reserved
}__attribute__((packed));

struct LogicBlkInfoType
{
	UINT16					  LogicBlkNum:10;					 //logic block number
	UINT16					  ZoneInDie:4;						//zone number in die
	UINT16					  Reserved:1;						 //reserved
	UINT16					  BlkType:1;						  //block type, brec code block, table or data block
}__attribute__((packed));

union LogPageStatus
{
	//log block age for update sequence
	UINT8					   LogBlkAge;
	//page status, 0xff-EmptyPage/0x55-UserDataPage/0xaa-PageMappingTblPage
	UINT8					   PhyPageStatus;
};
struct SpareData0
{
	UINT8					   BadFlag;							//bad block flag
	UINT16					  LogicBlkInfo;					   //logic block information for mapping manage
}  __attribute__((packed));

struct SpareData1
{
	//the logic page number which this physic page mapping to
	UINT16					  LogicPageNum;
	//log status, may be LogBlkAge or PhysicPageStatus
	union LogPageStatus		 LogStatus;
}  __attribute__((packed));

struct SpareData2
{
	UINT8					  df_BadFlag;						//downgrade flash bad block flag
	UINT8					  reserved[1];					   //reserved
} __attribute__((packed));
struct SpareData
{
	struct SpareData0		   SpareData0;		/*for sector0 spare area*/
	struct SpareData1		   SpareData1;		/*for sector1 spare area*/
	struct SpareData2			SpareData2;		/*for sector2 spare area*/
};

struct NandSpareData
{
	struct SpareData			 UserData[2];						//the spare area for 2 pages
};

/* ================================================================================================================== */


/* ===================================================================================================================*/
struct PhysicOpParameter
{
	//Bank number
	UINT8   BankNum;
	//page number in physic block,the page is based on multi-plane if support
	UINT16   PageNum;
	//physic block number in bank,the physic block is based on multi-plane if support
	UINT16  PhyBlkNumInBank;
	//page sector bitmap
	UINT32  SectorBitmapInPage;
	//pointer to main data buffer
	void	*MainDataPtr;
	//pointer to spare data buffer
	void	*SpareDataPtr;
};

struct LogicPagePar
{
	//the logic block number in zone which this global page belonged to
	UINT16					  LogicBlkNumInZone;
	//the page number in block
	UINT16					  LogicPageNum;
	//the bitmap of the page data
	UINT32					  SectorBitmap;
	//the zone number which this global page number belonged to
	UINT8					   ZoneNum;
#ifdef  PLATFORM_IS_32BIT
	//reserved
	UINT8					   Reserved[3];
#endif
};


struct AdaptProcPagePar
{
	//the page number of the page that the adaptor processing page
	UINT32					  PageNum;
	//the sector bitmap of the page that the adaptor processing page
	UINT32					  SectorBitmap;
#ifdef  PLATFORM_IS_32BIT
	//reserved
	UINT8					   Reserved[2];
#endif
};


struct NandRWPar
{
	//global logical secter No.
	UINT32					  lba;
	//read or write sector number
	UINT16					  length;
	//reserved0,different use for different platform
	UINT8					   reserved0;
	//reserved1
	UINT8					   reserved1;
	//sram address
	void						*sramaddr;
};

#define TOTAL_MAX_ZONE	(NAND_MAX_DIE_NUM * MAX_ZONE_NUM_IN_DIE)
struct NandLogicCtlPar
{
	UINT8					   LastOp;							 //last operation type,read or write
	UINT8					   ZoneNum;							//the number of the last access zone
	UINT16					  SuperBlkNum;						//the number of the logic block last acess
	UINT16					  SuperPageNum;					   //the number of the page last operation
	UINT16					  LogPageLastPageNum;				 //the page of current log page last position
	struct DataBlkTblType	   LogPageLastBlk;					 //the block of current log page last position
	UINT32					  LastWBufPageNum;					//last write buffer page number
	struct DataBlkTblType	   PhyDataBlk;						 //the super physic block number of data block
//	struct DataBlkTblType	   PhyLogBlk;						  //the super physic block number of log block
	int 						HasLog;
	UINT16					  LogPageNum;						 //last operate page number in log block
	UINT32					  LogPageBitmap;					  //the bitmap of the log page last write
	UINT8					   LogBlkAccessAge[TOTAL_MAX_ZONE][MAX_LOG_BLK_NUM];   //record every log block access time,for merge
	UINT8					   AccessTimer[TOTAL_MAX_ZONE];						//the timer for log page write access,for merge
};

struct NandDevPar
{
	UINT8	   NandID[4];						  //nand flash id
	UINT8	   DieCntPerChip;					  //the die count in a chip
	UINT8	   SectNumPerPPage;					//page size,based on 0.5k
	UINT8	   SpareBytesPerSector;				//spare bytes per sector
	UINT16	  PageNumPerPBlk;					 //the page number of physic block
	UINT8	   Frequence;						  //frequence parameter,may be based on xMHz
	UINT16	  BlkNumPerDie;					   //total number of the physic block in a die
	UINT16	  DataBlkNumPerZone;				  //the number of data blocks in one zone
	UINT32	  OperationOpt;					   //the operation support bitmap
	struct SpecialCmdType *SpecOp;				  //special operation struct
};

struct ECCUnitParam
{
	UINT32 ECCBitsPerECCUnit;
	UINT32 SectorsPerECCUnit;
	UINT32 SpareBytesPerECCUnit;
	UINT32 UserDataBytesPerECCUnit;
	UINT32 ECCUnitParity;
};

struct NandState_t
{
	UINT32 ftlr;
	UINT32 ftlw;
	UINT16 ecc_min;
	UINT16 ecc_avg;
	UINT16 ecc_max;
	UINT16 rr_total;
	UINT16 rr_failed;
	UINT32 pmt_switch;
	UINT32 bmt_switch;
	UINT32 swap_merge;
	UINT32 move_merge;
	UINT32 simple_merge;
};

/* for PHY_ReadNandId */
#define NAND_IDTYPE_NORMAL				  0	   /* normal: send 0x90 with 0x00 address command */
#define NAND_IDTYPE_TOSHIBA_EXTEND		  1	   /* toshiba lba: send 0x92 command */
#define NAND_IDTYPE_ONFI					2	   /* onfi standard: send 0x90 with 0x20 address command */

#define DEFAULT_NAND_CHIPID_LENGTH		  6	   /* nand flash chipid length */

/*use Die num to access physical block data in page algorithm*/
/*DFB info definition*/
struct DFB_location
{
	UINT16	die_num;			/*die num where DFB is in, whole Flash arrange*/
	UINT16	phy_blk_num;		/*phy blk num in this DIE*/
};

struct	DFB_location_in_chip
{
	UINT8	dfb_cnt_in_chip;					/*DFB count actually*/
	struct DFB_location	dfb_location[MAX_DFB_CNT];
};


struct DFB_INFO
{
	UINT8	chip_cnt;							/*chip count actually*/
	struct	DFB_location_in_chip	dfb_location_in_chip[NAND_MAX_CHIP_NUM];
	UINT16	*bad_blk_tbl[NAND_MAX_CHIP_NUM];	/*bad block table*/
};

struct DFB_items
{
	UINT16 bd_blk_index;		//坏块块号
	UINT8 flag;					//坏块性质，目前都为0xff，不区分坏块性质
	UINT8 reserve1;
}  __attribute__((packed));

/*FIB info definition*/
struct FIB_location
{
	UINT16	die_num;			/*die num where FIB is in*/
	UINT16	phy_blk_num;		/*phy blk num in this DIE*/
	UINT16	page_num;			/*page num in this blk*/
};

struct PHY_DIE_INFO
{
	UINT8	chip_num;
	UINT8	reserve1[3];
	UINT32	good_blk_cnt_in_die;
	UINT32	bad_blk_cnt_in_die;
	UINT8	reserve2[4];
};

struct LOG_DIE_INFO
{
	UINT8	zone_cnt_in_die;
	UINT8	reserve1[3];
	UINT32	data_blk_cnt_in_zone;
	UINT32	free_blk_cnt_in_zone;
	UINT32	log_blk_cnt_in_zone;
};

struct DIE_INFO
{
	struct PHY_DIE_INFO	phy_die_info;
	struct LOG_DIE_INFO	log_die_info;
};

struct FIB_FLASH_INFO
{
	UINT8	die_cnt_in_flash;
	UINT8	reserve1[3];
	struct 	DIE_INFO	 die_info[NAND_MAX_CHIP_NUM * NAND_MAX_DIE_NUM];
	UINT32  OperationOpt; 		//the operation support bitmap
	UINT8	reserve2[4];
};

struct FIB_INFO
{
	UINT8	fib_cnt;
	UINT8	reserve1[3];
	struct 	FIB_location	fib_location[MAX_FIB_CNT];
	struct 	FIB_FLASH_INFO	fib_flash_info;
};

extern struct NandDev NandDevInfo;
extern struct StorageInfo		  NandStorageInfo;
extern struct SpecialCmdType	   SpecialCommand;
extern struct LogicOrganizeType	LogicOrganisePara;
extern struct ZoneInfoType		 ZoneInfo[NAND_MAX_DIE_NUM * MAX_ZONE_NUM_IN_DIE];
extern struct ZoneTblCacheInfo	 ZoneTblCache;
extern struct PageTblCacheInfo	 PageMapTblCache;

/* ================================================================================================================== */

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

// for 5007/5009, need read 4 sectors to get 8 bytes spare userdata
/* sector bitmap defines */
#define GENSECTORBITMAP(lbitmap, rbitmap) \
		((UINT32)((lbitmap) | ((rbitmap) << SECTOR_NUM_PER_SINGLE_PAGE)))

#if 1
#define SECTOR_BITMAP_SPAREDATA \
		((UINT32)((NAND_SUPPORT_MULTI_PLANE!=0) ? \
	  (GENSECTORBITMAP(NandStorageInfo.UserDataSectorBitmap, NandStorageInfo.UserDataSectorBitmap)) : \
	  (GENSECTORBITMAP(NandStorageInfo.UserDataSectorBitmap, (UINT32)0x0))))
#endif

//#define SECTOR_BITMAP_ZONETBL   (GENSECTORBITMAP(MAX(0xf, NandStorageInfo.UserDataSectorBitmap), 0x0))
//#define SECTOR_BITMAP_DIRTYFLAG (GENSECTORBITMAP(MAX(0xf, NandStorageInfo.UserDataSectorBitmap), 0x0))
//#define SECTOR_BITMAP_PMTBL	 (GENSECTORBITMAP(MAX(0xf, NandStorageInfo.UserDataSectorBitmap), 0x0))
//#define SECTOR_BITMAP_SPAREDATA GENSECTORBITMAP(0xf, 0xf)
#define SECTOR_BITMAP_ZONETBL   GENSECTORBITMAP(0xf, 0x0)
#define SECTOR_BITMAP_DIRTYFLAG GENSECTORBITMAP(0xf, 0x0)
#define SECTOR_BITMAP_PMTBL	 GENSECTORBITMAP(0xf, 0x0)

#define ECCUNIT_NUM_PER_SINGLE_PAGE		 (NandStorageInfo.ECCUnitNumPerPage)
#define SECTORS_PER_ECCUNIT				 (NandStorageInfo.SysECCUnitParam->SectorsPerECCUnit)
#define UDBYTES_PER_ECCUNIT				 (NandStorageInfo.SysECCUnitParam->UserDataBytesPerECCUnit)
#define SPARE_BYTES_PER_ECCUNIT			 (NandStorageInfo.SysECCUnitParam->SpareBytesPerECCUnit)


/* ================================================================================================================== */
/* define the page status */
#define DATA_PAGE_FLAG			  0x55
#define TABLE_PAGE_FLAG			 0xaa
#define EMPTY_PAGE_FLAG			 0xff
/* define the page mapping table need update flag */
#define NEED_UPDATE_PAGE_MAP_TBL	0x01
/* define get the free block type, lowest erase count or highest erase count */
#define LOWEST_EC_TYPE			  0x00
#define HIGHEST_EC_TYPE			 0x01

/* page state in spare area defines */
#define PAGE_STATE_FREE		 (0xff)
#define PAGE_STATE_USED		 (0x55)
#define PAGE_STATE_PMTBL		(0xaa)

/* block state in PhyBlkArray */
#define BLOCK_STATE_NIL		 (0xfffd)
#define BLOCK_STATE_BAD		 (0xfffe)
#define BLOCK_STATE_FREE	 (0xffff)

/* block type in spare area */
#define BLOCK_TYPE_SPECIAL	  	(0x8000)
#define BLOCK_TYPE_MASK		 	(0x3ff)
#define BLOCK_TYPE_ZONETBL	  	(0xaa)
#define BLOCK_TYPE_BREC		 	(0xbb)
#define BLOCK_TYPE_ELOG			(0xEE)	/*event log block*/
#define BLOCK_TYPE_BBT			0x99
#define BLOCK_TYPE_DFB			0xcc
#define BLOCK_TYPE_FIB			0xdd

/*define DFB and FIB*/
#define BLOCK_BBT		(BLOCK_TYPE_SPECIAL | BLOCK_TYPE_BBT)
#define BLOCK_DFB		(BLOCK_TYPE_SPECIAL | BLOCK_TYPE_DFB)
#define BLOCK_FIB		(BLOCK_TYPE_SPECIAL | BLOCK_TYPE_FIB)
#define BLOCK_EVENT		(BLOCK_TYPE_SPECIAL | BLOCK_TYPE_ELOG)
#define BLOCK_ZONETBL	(BLOCK_TYPE_SPECIAL | BLOCK_TYPE_ZONETBL)
#define BLOCK_BREC		(BLOCK_TYPE_SPECIAL | BLOCK_TYPE_BREC)

#define GET_BLOCK_TYPE(LogicalBlkInfo)	 ((LogicalBlkInfo) & 0x3ff)
#define GET_LOG_BLOCK_NUM(LogicalBlkInfo)   ((LogicalBlkInfo) & 0x3ff)
#define GET_ZONETBL_NUM(LogicalBlkInfo)	(((LogicalBlkInfo) >> 10) & 0x0f)

/* define merge type */
#define NORMAL_MERGE				0x00		/* normal merge type, can't do move merge */
#define SPECIAL_MERGE			   0x01		/* special merge type, can do move merge */
/* ================================================================================================================== */

/* randomize the zone table checksum, avoid all 0x00 passed the check operation */
#define TBL_CHECKSUM_XOR_VALUE	  0xaa55

/* ================================================================ */
//Operation Options Bitmap
// move to flash_id.h

//DMA transfer direction
#define DMA_WRITE_TO_FLASH			   1
#define DMA_READ_FROM_FLASH			  2
#define DMA_RAM_TO_RAM				   3
#define DMA_USB_TO_RAM				   4

//nand status define
#define	 NAND_OPERATE_FAIL		   0x0001			//This bit is set if the most recently finished operation on the selected die
#define	 NAND_OPERATE_FAILC		  0x0002			//This bit is set if the previous operation on the selected die (LUN) failed
#define	 NAND_CACHE_READY			0x0020			//ARDY, ARRAY READY
#define	 NAND_STATUS_READY		   0x0040			//This bit indicates that the selected die (LUN) is not available to accept new commands,,
#define	 NAND_WRITE_PROTECT		  0x0080			//Write Protect




/* ================================================================ */

/*ECC type*/
#if 0

#define	 ECC_HM					   0x0
#define	 ECC_BCH_8					0x1
#define	 ECC_BCH_12				   0x2
#define	 ECC_BCH_16				   0x3

/*Spare area byte count per sector*/
#define	 BCH_8_SPARE_LEN		 16
#define	 BCH_12_SPARE_LEN		23
#define	 BCH_16_SPARE_LEN		27

/*ECC parity length*/
#define	 BCH_8_PARITY_LEN		 13
#define	 BCH_12_PARITY_LEN		20
#define	 BCH_16_PARITY_LEN		26

#endif

/*User data byte count*/
#define	 USER_DATA_1				  0x1
#define	 USER_DATA_2				  0x2
#define	 USER_DATA_3				  0x3
#define	 USER_DATA_4				  0x4
#define	 USER_DATA_5				  0x5
#define	 USER_DATA_6				  0x6
#define	 USER_DATA_7				  0x7


/* ================================================================ */
//Error type define
#define	 PHYSICAL_LAYER_ERROR		0x1000
#define	 LOGICAL_LAYER_ERROR		 0x2000
#define	 MAPPING_MODULE_ERROR		0x4000
#define SCAN_LAYER_ERROR		(0x8000)

#define SCAN_ERR_NO_MEM		 (SCAN_LAYER_ERROR | 0x1)
#define SCAN_ERR_TRY_AGAIN	  (SCAN_LAYER_ERROR | 0x2)

#define	 BUFFER_ADDR_ERROR			(0x1 << 0)
#define	 DATA_ECC_ERROR				(0x1 << 1)
#define	 OPERATE_ERROR				(0x1 << 2)
#define  DMA_TIMEOUT				(0x1 << 3)
#define	 DATA_ECC_LIMIT			  	(0x1 << 7)

#define	 PHY_READ_ERR_MASK	0x7f

#define	 PHY_LAYER_BUF_ERR		   (PHYSICAL_LAYER_ERROR | BUFFER_ADDR_ERROR)
#define	 NAND_DATA_ECC_ERROR		 (PHYSICAL_LAYER_ERROR | DATA_ECC_ERROR)
#define	 NAND_DATA_ECC_LIMIT		 (PHYSICAL_LAYER_ERROR | DATA_ECC_LIMIT)
#define	 NAND_OP_ERROR			   (PHYSICAL_LAYER_ERROR | OPERATE_ERROR)

#define	 PHY_READ_ERR(err)	((err) & PHY_READ_ERR_MASK)
#define	 PHY_READ_ECC_TO_LIMIT(err)	((err) & DATA_ECC_LIMIT)

/* ================================================================================================================== */



/* ================================================================================================================== */
/* define the full bitmap of the single page */
#define FULL_BITMAP_OF_SINGLE_PAGE          ((UINT32)((NandStorageInfo.SectorNumPerPage  == 32) ? \
        ((UINT32)~0) : ((UINT32)((1<<NandStorageInfo.SectorNumPerPage)-1))))
/* define the full bitmap of the super page */
#define FULL_BITMAP_OF_SUPER_PAGE           ((UINT32)((SECTOR_NUM_PER_SUPER_PAGE == 32) ?\
        ((UINT32)~0) :((UINT32)((1<<SECTOR_NUM_PER_SUPER_PAGE) - 1))))
/* define the full bitmap of the buffer page */
#define FULL_BITMAP_OF_BUFFER_PAGE          ((UINT32)((PAGE_BUFFER_SECT_NUM == 32) ? \
        ((UINT32)~0) :((UINT32)((1<<PAGE_BUFFER_SECT_NUM) - 1))))
/* the multi block offset */
#define MULTI_BLOCK_ADDR_OFFSET			 (NandDevInfo.SpecialCommand->MultiBlkAddrOffset)
/* the bad block flag position */
#define BAD_BLOCK_FLAG_PST				  (NandDevInfo.SpecialCommand->BadBlkFlagPst)
/* check if nand flash can suport internal interleave */
#define NAND_SUPPORT_INTERNAL_INTERLEAVE	(OPT_INTERNAL_INTERLEAVE & NandStorageInfo.OperationOpt)
/* check if nand flash can suport external interleave */
#define NAND_SUPPORT_EXTERNAL_INTERLEAVE	(OPT_EXTERNAL_INTERLEAVE & NandStorageInfo.OperationOpt)
/* check if nand flash can support multi-plane operation */
#define NAND_SUPPORT_MULTI_PLANE			(OPT_MULTI_PAGE_WRITE & NandStorageInfo.OperationOpt)
#define NAND_SUPPORT_MULTI_PLANE_READ \
	(((OPT_MULTI_PAGE_WRITE | OPT_MULTI_PAGE_READ) \
		& NandStorageInfo.OperationOpt) == (OPT_MULTI_PAGE_WRITE | OPT_MULTI_PAGE_READ))
/* check if nand flash can support cache program operation */
#if(SUPPORT_CACHE_PROGRAM)
#define NAND_SUPPORT_CACHE_PROGRAM		  (OPT_PAGE_CACHE_PROGRAM & NandStorageInfo.OperationOpt)
#else
#define NAND_SUPPORT_CACHE_PROGRAM		  (0)
#endif
/* check if nand flash can support copy back operation */
#define NAND_SUPPORT_PAGE_COPYBACK		  (OPT_PAGE_COPYBACK & NandStorageInfo.OperationOpt)
/* check if nand is downgrade flash or not */
#define NAND_IS_DOWNGRADE		  			(OPT_DOWNGRADE_FLASH & NandStorageInfo.OperationOpt)

/* check if flash is small block */
#define NAND_IS_SMALL_BLOCK				 (OPT_SMALL_BLOCK & NandStorageInfo.OperationOpt)

#define NAND_IS_12BITSECC_FLASH			 (OPT_ECC_REQ_12BITS & NandStorageInfo.OperationOpt)
/* check if there is chip gap and need align block/page address */
#define NAND_NEED_BLOCK_ALIGN			   (OPT_ALIGN_BLOCK & NandStorageInfo.OperationOpt)	//
#define NAND_NEED_PAGE_ALIGN				(OPT_ALIGN_PAGE & NandStorageInfo.OperationOpt)

//#define NAND_IS_NEED_RANDOMIZER			 (OPT_NEED_RANDOMIZER & NandStorageInfo.OperationOpt)
#define NAND_IS_NEED_RANDOMIZER			 (1)
#define NAND_NEED_READ_RETRY				(OPT_NEED_READ_RETRY & NandChipInfo.OperationOpt)

/* if nand support secure rw 2010-9-13*/
#ifdef TLC_SUPPORT
#define NAND_SUPPORT_SECURE_RW			  (NAND_NEED_READ_RETRY)
#else
#define NAND_SUPPORT_SECURE_RW			  (0)
#endif


#define NAND_HAS_BUILDIN_ECC				(0)

#define ZONE_NUM_PER_DIE		(NandDevInfo.LogicOrganizePar->ZoneNumPerDie)
#define DIE_CNT_PER_CHIP		(NandStorageInfo.DieCntPerChip)
#define BNK_CNT_PER_CHIP		(NandStorageInfo.BankCntPerChip)
#define DIE_CNT_PER_BANK		(NandDevInfo.LogicOrganizePar->DieCntPerBank)
#define BANK_CNT_PER_DIE		(NandDevInfo.LogicOrganizePar->BankCntPerDie)
#define CHIPS_TO_INTERLEAVE		(NandDevInfo.LogicOrganizePar->ChipsToInterleave)
#define TOTAL_BNK_CNT		   	(NandDevInfo.LogicOrganizePar->TotalBanks)
#define BLK_CNT_PER_DIE		 	(NandStorageInfo.BlkNumPerDie)
#define PLANE_CNT_PER_DIE	   	(NandStorageInfo.PlaneCntPerDie)
#define PAGE_CNT_PER_LOGIC_BLK  (NandDevInfo.LogicOrganizePar->PageNumPerLogicBlk)
#define PAGE_CNT_PER_PHY_BLK	(NandStorageInfo.PageNumPerPhyBlk)
#define DIENUM_IN_FLASH			(NandDevInfo.LogicOrganizePar->TotalDies)
#define ZONENUM_PER_DIE			(NandDevInfo.LogicOrganizePar->ZoneNumPerDie)
#define SUBLK_PER_DIE			(NandDevInfo.LogicOrganizePar->BlocksPerDie)
#define PAGES_PER_SUBLK			(NandDevInfo.LogicOrganizePar->PageNumPerLogicBlk)

#define PAGE_CNT_PER_PHY_BLK_IN_SECURE_MODE  (NandStorageInfo.PageNumPerPhyBlk/2)
#define DATA_BLK_NUM_PER_ZONE   (NandDevInfo.LogicOrganizePar->DataBlkNumPerZone)

#define BMT (NandDevInfo.ZoneTblCacheInfo->CurZoneTbl)
#define DTBL (BMT->DataBlkTbl)			/*data block map table of current zone*/
#define LTBL (BMT->LogBlkTbl)			/*log block map table of current zone*/
#define FTBL (BMT->FreeBlkTbl)			/*free block map table of current zone*/

#define PMT	(NandDevInfo.PageTblCacheInfo->CurPageTbl)
#define L2P (PMT->PageMapTbl)		 	/*page map table of current log block*/

#define CURZONE (BMT->ZoneNum) 		/*current accessed zone*/
#define CURDIE (BMT->DieNum) 			/*current accessed Die*/

#define PAGE_COPYBACK_BUFFER				(NandDevInfo.logic_mgmt_buffer)
//#define LOGIC_READ_PAGE_CACHE				NandDevInfo.logic_mgmt_buffer)
#define LOGIC_WRITE_PAGE_CACHE				(NandDevInfo.logic_mgmt_buffer)
#define LOGIC_ADAPTER_CACHE					(NandDevInfo.PageCache)
#define TMPBUF								(NandDevInfo.logic_mgmt_buffer)

/*FIB info struct*/
#define FIB_LOCATION_INFO	(NandDevInfo.fib_info->fib_location)
#define FIB_DIE_INFO			(NandDevInfo.fib_info->fib_flash_info.die_info)
#define	FIB_DIE_CNT		(NandDevInfo.fib_info->fib_flash_info.die_cnt_in_flash)
#define PHY_GOOD_BLK_CNT_IN_DIE(die_num)			(FIB_DIE_INFO[die_num].phy_die_info.good_blk_cnt_in_die)
#define PHY_BAD_BLK_CNT_IN_DIE(die_num)			(FIB_DIE_INFO[die_num].phy_die_info.bad_blk_cnt_in_die)
#define ZONE_CNT_IN_DIE(die_num)					(FIB_DIE_INFO[die_num].log_die_info.zone_cnt_in_die)
#define	DATA_BLK_CNT_IN_ZONE_PER_DIE(die_num)	(FIB_DIE_INFO[die_num].log_die_info.data_blk_cnt_in_zone)
#define	FREE_BLK_CNT_IN_ZONE_PER_DIE(die_num)	(FIB_DIE_INFO[die_num].log_die_info.free_blk_cnt_in_zone)
#define	LOG_BLK_CNT_IN_ZONE_PER_DIE(die_num)		(FIB_DIE_INFO[die_num].log_die_info.log_blk_cnt_in_zone)

/*for 4CE config*/
#define GPIO_LOW_LEVEL					  (0)
#define GPIO_HIGH_LEVEL					 (1)

#define GPIO_OUT_DISABLE					(0)
#define GPIO_OUT_ENABLE					 (1)

#define NAND_CETYPE_GPIO					(1)
#define NAND_CETYPE_MUX					 (2)

#define NAND_CECONFIG_TYPE(CEConfig)		(((CEConfig) >> 24) & 0xff)
#define NAND_CECONFIG_CE3(CEConfig)		 (((CEConfig) >> 8) & 0xff)
#define NAND_CECONFIG_CE4(CEConfig)		 ((CEConfig) & 0xff)
#define NAND_CECONFIG_CHIP1_SEL_CE0_ENABLE(CEConfig)		 (((CEConfig) >> 16) & 0x01)


/* CE3 - GPIOA7, GPIO_I2C1_SDA, CE4 - GPIOB31, GPIO_SIRQ1 */
#define NAND_CECONFIG					   (NandStorageInfo.NandCEConfig) //0x0100073f
/*Nand manufactory company ID. This is used in read retry flow*/
#define NAND_MFC_ID							(NandChipInfo.ChipID[0])

#define CHIP_MUTI_READ_TYPE					(NandChipInfo.OpType[0])
#define CHIP_MUTI_SEL_TYPE					(NandChipInfo.OpType[1])
#define CHIP_MUTI_ERASE_TYPE				(NandChipInfo.OpType[2])

#endif  //#ifdef _NAND_FLASH_DRIVER_STRUCT_H_

