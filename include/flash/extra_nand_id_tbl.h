
#ifndef __EXTRA_NAND_ID_TBL_H__
#define __EXTRA_NAND_ID_TBL_H__


#include "types.h"

enum {
	_OPT_SYNC_SDR_TOGGLE,
	_OPT_SYNC_ONLY,
	_OPT_SYNC_JEDEC_TOGGLE, //has jedec means has toggle interface
};

#define OPT_SYNC_SDR_TOGGLE			(0x1 << _OPT_SYNC_SDR_TOGGLE)
#define OPT_SYNC_ONLY		(0x1 << _OPT_SYNC_ONLY)
#define OPT_SYNC_JEDEC_TOGGLE		(0x1 << _OPT_SYNC_JEDEC_TOGGLE)


#define		SYNC_NAND_TYPE_TOGGLE_1				1
#define		SYNC_NAND_TYPE_TOGGLE_2				2
#define		SYNC_NAND_TYPE_DDR					3

#define		MAX_INTF_CHANGE_FEAT_SET			4

struct intf_change_feat_set {
	UINT8 addr;
	UINT32 value;
};

typedef struct extra_flash_attr_info_s{
    UINT8   NandChipId[8];
    UINT32  OperationOpt;	//Behavior Properties
	UINT32	ChipType;
	UINT32	Frequence;
	UINT8	feat_sets_count;
	struct intf_change_feat_set feat_sets[MAX_INTF_CHANGE_FEAT_SET];
	UINT8	clr_feat_sets_count;
	struct intf_change_feat_set clr_feat_sets[MAX_INTF_CHANGE_FEAT_SET];
	UINT32	ExtraType;		//vendor specific
} __attribute__((packed))  extra_flash_attr_info_t;


#endif /* __EXTRA_NAND_ID_TBL_H__ */

