
//#include "owlxx_ioctl.h"



#define LOGTBL          	0
#define DATATBL          	1
#define FREETBL          	2

#define MBRINFO_TYPE    0
#define MBRC_TYPE       1
#define VMLINUX_TYPE    2

#define FORCE_FLUSH_FLASH

struct dump_tbl_type {
	int phy_num;
	int zone_num;
	int tbl_type;
	unsigned char * tbl_buf;
};

struct ZoneInfoInPhy {
	unsigned char ZoneNumInPhy;
	unsigned short DataBlkNumInPhy;
};
#if 0	//do not use this anymore since 20121026
typedef struct BootPagePar {
	unsigned short PageNum;
	unsigned int   SectorBitmapInPage;
} BootPagePar ;

typedef struct BootCmdParam {
	unsigned char two_plane_flag;
	/* sublk num */ 
	unsigned int BootBlkNum; 
	/* page param */
	BootPagePar PagePar; 
	unsigned char * Buf;
	unsigned char write_type;
} BootCmdParam ;

typedef struct BootErsParam {
	unsigned char two_plane_flag;
	unsigned int BootBlkNum;
} BootErsParam ;

typedef struct BootFlashInfo {
	unsigned char LB_nand_flag;
	unsigned char PlaneCntPerDie;
	unsigned short PlaneOffset;
	unsigned int PageNumPerBlk;
	unsigned int SectNumPerPage;
	unsigned int Brom_ECCBitsPerECCUnit;
	unsigned int Brom_UserDataBytesPerECCUnit; 
	unsigned int read_try; 
	unsigned char nandchipid[64];
	unsigned char lsb_tbl[128];
	unsigned char bad_blk_tbl[80];
	unsigned int DataBlkNumInBoot;
	
} BootFlashInfo ;

struct DbgFlashInfo {
	unsigned char PlaneCntPerDie;
	unsigned char ChipCnt;
	unsigned short PlaneOffset;
	unsigned int PageNumPerBlk;
	unsigned int SectNumPerPage;
	unsigned long TotalSublkNum;
};

struct PageDumpParam {
	unsigned char BankNum;
	unsigned int SuBlkNum;
	BootPagePar PagePar;
	unsigned char * MainBuf;
	unsigned char * SpareBuf;
};
#endif /*0*/

typedef struct __boot_medium_info_t
{
	unsigned int medium_type;		//0x1 large block nand; 0x0 small block nand.
	unsigned int page_per_blk;
	unsigned int sec_per_page;
	unsigned int ecc_bits;			//boot ecc config-bchX.
	unsigned int ud_bytes;			//userdata bytes count per ecc unit.
	unsigned int readretry;			//need readretry?
	unsigned int data_blk_per_zone;	//
	unsigned char chipid[64];		//chipid item without Mark.
	unsigned char lsb_tbl[128];		//useable when nand need readretry.
	unsigned char badblk_tbl[80];	//bad block table.
} boot_medium_info_t;

typedef struct _boot_op_t
{
	unsigned int blk;
	unsigned int page;
	unsigned char * buffer;
}boot_op_t;

#if 0
typedef struct __mbr_struct_t
{
	u32 code_0[2];	//
	u32 pages_per_blk;
	u32 sector_per_page;
	u32 vmlinux_size;
	u32 best_delay;
	u32 ecc_config;
	u32 rw_addr[3];
	u32 second_mbr_size;
	u32 all_mbr_size;
	u8 readretry;
	u8 reserve[3];
	
	u8 
	
}mbr_struct_t;
#endif /*0*/

