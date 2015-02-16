#define DEBUG // TESTCODE
#include <common.h>
#include <part.h>
#include <malloc.h>
#include <asm/errno.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/mbr_info.h>


DECLARE_GLOBAL_DATA_PTR;

extern int LDL_DeviceOpReadSectors(unsigned int start, unsigned int nsector, void *buf, int diskNo);
extern int LDL_DeviceOpWriteSectors(unsigned int start, unsigned int nsector, void const *buf, int diskNo);
extern int LDL_DeviceOperateInit(void);
extern void LDL_DeviceOperateRelease(void);
extern void LDL_DeviceOperateFreeze(void);

static block_dev_desc_t s_nandblk_dev;
static uint8_t s_nandblk_initiated = 0;

static unsigned long _nandblk_partn_read(int dev,
            lbaint_t start, lbaint_t blkcnt, void *buffer)
{
    block_dev_desc_t *p_blkdev;
    int ret;

    if(s_nandblk_initiated == 0)
    {
        printf("nandblk has not been initialized\n");
        return -1;
    }
    assert((uint)dev == 0);
    p_blkdev = &s_nandblk_dev;

    if(blkcnt == 0)
    {
        return 0;
    }

    if ((start + blkcnt) >= p_blkdev->lba || buffer == NULL)
    {
        printf("%s: invalid param, dev %d, start=%lu blkcnt=%lu buffer=0x%p partn_cap=%lu\n",
            __FUNCTION__, dev, start, blkcnt, buffer, p_blkdev->lba);
        return -1;
    }

    BUG_ON(((ulong)buffer & 31U) != 0);

    ret = LDL_DeviceOpReadSectors(start, blkcnt, buffer, 0);
    if (ret != 0)
    {
        printf("%s: LDL read err, dev=%d start=%lu blkcnt=%lu buf=0x%p ret=%d\n",
                __FUNCTION__, dev, start, blkcnt, buffer, ret);
        return -1;
    }

    return blkcnt;
}

static unsigned long _nandblk_partn_write(int dev,
        lbaint_t start, lbaint_t blkcnt, const void *buffer)
{
    block_dev_desc_t *p_blkdev;
    int ret;

    if(s_nandblk_initiated == 0)
    {
        printf("nandblk has not been initialized\n");
        return -1;
    }
    assert((uint)dev == 0);
    p_blkdev = &s_nandblk_dev;

    if(blkcnt == 0)
    {
        return 0;
    }

    if ((start + blkcnt) >= p_blkdev->lba || buffer == NULL)
    {
        printf("%s: invalid param, dev %d, start=%lu blkcnt=%lu buffer=0x%p partn_cap=%lu\n",
            __FUNCTION__, dev, start, blkcnt, buffer, p_blkdev->lba);
        return -1;
    }

    BUG_ON(((ulong)buffer & 31U) != 0);

    ret = LDL_DeviceOpWriteSectors(start, blkcnt, buffer, 0);
    if (ret != 0)
    {
        printf("%s: LDL write err, dev=%d start=%lu blkcnt=%lu buf=0x%p ret=%d\n",
                __FUNCTION__, dev, start, blkcnt, buffer, ret);
        return -1;
    }

    return blkcnt;
}

int act_nandblk_init(void)
{
    block_dev_desc_t *p_blkdev;
    int ret;

    debug("init actions NAND.\n");

    ret = LDL_DeviceOperateInit();
    if (ret != 0)
    {
        printf("%s: LDL_DeviceOperateInit() err, ret=%d\n", __FUNCTION__, ret);
        return -1;
    }
    s_nandblk_initiated = 1;

    memset(&s_nandblk_dev, 0, sizeof(s_nandblk_dev));
    p_blkdev = &s_nandblk_dev;
    p_blkdev->if_type = IF_TYPE_ACT_NANDBLK;
    p_blkdev->part_type = PART_TYPE_UNKNOWN;
    p_blkdev->dev = 0;
    p_blkdev->lba = (lbaint_t)(-1); // TODO : use actual NAND capacity.
    p_blkdev->blksz = 512;
    p_blkdev->log2blksz = 9;
#ifdef CONFIG_LBA48
    p_blkdev->lba48 = 1;
#endif
    strcpy(p_blkdev->vendor, "Actions");
    strcpy(p_blkdev->product, "newnand");
    p_blkdev->block_read  = _nandblk_partn_read;
    p_blkdev->block_write = _nandblk_partn_write;

    init_part(p_blkdev);

    return 0;
}

void act_nandblk_exit(void)
{
    debug("exit actions NAND.\n");

    if(s_nandblk_initiated)
    {
        s_nandblk_initiated = 0;
        LDL_DeviceOperateFreeze();
    }
}

#ifdef CONFIG_PARTITIONS
block_dev_desc_t *act_nandblk_get_dev(int dev)
{
    if (dev != 0)
    {
        printf("%s: err dev num %d\n", __FUNCTION__, dev);
        return NULL;
    }
    return &s_nandblk_dev;
}
#endif



//----------------------------------------------------------------
// functions used by nand_drv itself

int asoc_get_dvfslevel(void)
{
    //return act_get_dvfslevel();
    return gd->arch.dvfs_level;
}

