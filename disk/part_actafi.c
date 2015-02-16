/* add by Actions */
#define DEBUG //TESTCODE
#include <common.h>
#include <malloc.h>
#include <command.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/afinfo.h>
#include <asm/arch/mbr_info.h>

/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;

#define CONFIG_ACTS_ACTAFI_PARTN_CN     12   /* fixed! */

static const char * s_partion_name_tbl[CONFIG_ACTS_ACTAFI_PARTN_CN] =
{
    "boot(phy)",
    "recovery",
    "misc",
    "system",
    "data",
    "cache",
    "_reserved1",
    "vendor_apk",
    "vendor_media",
    "_reserved2",
    "udisk",
    "swap"
};

typedef struct
{
    uint32_t    offset; // in sectors
    uint32_t    cap;    // in sectors
} act_afi_partn_info_entry_t;

typedef struct
{
    uint record_index;
    int interface_type;
    int dev_num;
    uint backing_log2blksz;
    uint partn_cnt;
    act_afi_partn_info_entry_t partn_tbl[CONFIG_ACTS_ACTAFI_PARTN_CN];
    afinfo_t afinfo_backup;
} act_afi_partn_obj_t;

static act_afi_partn_obj_t *s_dev_partn_obj_tbl[16];



//==============================================================================


static act_afi_partn_obj_t * _afi_get_partn_obj(block_dev_desc_t const *dev_desc, uint create_new)
{
    act_afi_partn_obj_t *p;
    uint i, log2blksz;

    for(i=0; i<ARRAY_SIZE(s_dev_partn_obj_tbl); i++)
    {
        p = s_dev_partn_obj_tbl[i];
        if(p == NULL)
        {
            break;
        }
        if(p->interface_type==dev_desc->if_type && p->dev_num==dev_desc->dev)
        {
            return p;
        }
    }
    // not found
    if(create_new)
    {
        BUG_ON(i >= ARRAY_SIZE(s_dev_partn_obj_tbl));

        p = malloc(sizeof(act_afi_partn_obj_t));
        if(p == NULL)
        {
            printf("%s: no mem\n", __FUNCTION__);
            return NULL;
        }
        p->interface_type = dev_desc->if_type;
        p->dev_num = dev_desc->dev;
        if(dev_desc->log2blksz != 0)
        {
            log2blksz = dev_desc->log2blksz;
            BUG_ON((1U <<log2blksz) != dev_desc->blksz);
        }
        else
        {
            log2blksz = LOG2(dev_desc->blksz);
        }
        p->backing_log2blksz = log2blksz;
        p->record_index = i;

        s_dev_partn_obj_tbl[i] = p;
        return p;
    }
    return NULL;
}

static void _afi_destroy_partn_obj(act_afi_partn_obj_t *p)
{
    BUG_ON(s_dev_partn_obj_tbl[p->record_index] != p);
    s_dev_partn_obj_tbl[p->record_index] = NULL;
    free(p);
}



//==============================================================================


static int _afi_read_afinfo_from_blk_storage(block_dev_desc_t const *dev_desc, afinfo_t *p_afinfo_buf)
{
    uint32_t dev_blkszlog2, mbrc1_off, mbrc1_blks, retry_cnt;
    uint32_t mbrc2_size, mbrc2_off, mbrc2_blks, mbrc2_base_addr, mbrc2_afinfo_addr;
    uint8_t *p_mbrc_buf, *p_mbrc2;
    afinfo_t *p_card_afinfo;
    int ret;

    p_mbrc_buf = memalign(ARCH_DMA_MINALIGN, 512 * 1024); /* 1st + 2nd MBRC sould not exceed 512KiB */
    if(p_mbrc_buf == NULL)
    {
        printf("%s: no mem\n", __FUNCTION__);
        return -1;
    }

    dev_blkszlog2 = dev_desc->log2blksz;
    BUG_ON((1U<<dev_blkszlog2) != dev_desc->blksz);

    // get & check MBRC1
    mbrc1_off = 0;
    mbrc1_blks = (512U + (1U<<dev_blkszlog2) -1U) >> dev_blkszlog2;
    for(retry_cnt=4; retry_cnt!=0; retry_cnt--,mbrc1_off++)
    {
        ret = dev_desc->block_read(dev_desc->dev, mbrc1_off, mbrc1_blks, p_mbrc_buf);
        if(ret < 0)
        {
            printf("%s: get MBRC1 failed, can not read storage, ret=%d\n",
                    __FUNCTION__, ret);
            goto label_error_1;
        }
        if(memcmp("ActBrm\xaa\x55", p_mbrc_buf+512U-12U, 8) != 0)
        {
            debug("%s: invalid MBRC1 signature @ lba %u\n",
                    __FUNCTION__, mbrc1_off);
            continue;
        }
        if(act_calc_checksum32(p_mbrc_buf, 512U-4U, 0x1234) !=
                *((uint32_t*)(p_mbrc_buf+512U-4U)))
        {
            debug("%s: invalid MBRC1 checksum\n @ lba %u",
                    __FUNCTION__, mbrc1_off);
            continue;
        }
        break;
    }
    if(retry_cnt == 0)
    {
        debug("%s: can not find any MBRC1\n", __FUNCTION__);
        goto label_error_1;
    }
    debug("%s: found valid MBRC1 @ lba %u\n", __FUNCTION__, mbrc1_off);
    // OK, now we find a valid MBRC1

    // get & check MBRC2
    mbrc2_size = *(uint32_t *)(p_mbrc_buf + 0x28);
    mbrc2_blks = (mbrc2_size + (1U << dev_blkszlog2) -1U) >> dev_blkszlog2;
    if(mbrc2_blks == 0 || (mbrc2_blks<<dev_blkszlog2) > (512*1024))
    {
        debug("%s: invalid MBRC1 header\n", __FUNCTION__);
        goto label_error_1;
    }
    mbrc2_off = mbrc1_off + (1024U >> dev_blkszlog2);
    ret = dev_desc->block_read(dev_desc->dev, mbrc2_off, mbrc2_blks, p_mbrc_buf+1024U);
    if(ret < 0)
    {
        printf("%s: get MBRC2 failed, can not read storage, ret=%d\n",
                __FUNCTION__, ret);
        goto label_error_1;
    }
    p_mbrc2 = p_mbrc_buf + 1024U;
    if(*((uint16_t*)(p_mbrc2+mbrc2_size-4U)) != 0x55aaU)
    {
        debug("%s: invalid MBRC2 signature\n", __FUNCTION__);
        goto label_error_1;
    }
    if(act_calc_checksum16(p_mbrc2, mbrc2_size-2U, 0) !=
            *((uint16_t*)(p_mbrc2+mbrc2_size-2U)))
    {
        debug("%s: invalid MBRC2 checksum\n", __FUNCTION__);
        goto label_error_1;
    }

    // check afinfo
    mbrc2_base_addr = *((uint32_t*)(p_mbrc2+32U));
    mbrc2_afinfo_addr = *((uint32_t*)(p_mbrc2+36U));
    if(mbrc2_base_addr==0 || mbrc2_afinfo_addr==0 ||
       (mbrc2_base_addr&3U)!=0 || (mbrc2_afinfo_addr&3U)!= 0 ||
       mbrc2_base_addr>=mbrc2_afinfo_addr)
    {
        debug("%s: invalid MBRC2 header\n", __FUNCTION__);
        goto label_error_1;
    }
    p_card_afinfo = (afinfo_t*)(p_mbrc2 + (mbrc2_afinfo_addr-mbrc2_base_addr));
    if(memcmp("AFI\x00", p_card_afinfo->magic, 4) != 0 ||
       p_card_afinfo->blength!=2 || p_card_afinfo->btype!=6)
    {
        debug("%s: invalid MBRC2 AFINFO header\n", __FUNCTION__);
        goto label_error_1;
    }

    memcpy(p_afinfo_buf, p_card_afinfo, sizeof(afinfo_t));
    free(p_mbrc_buf);
    return 0;

    label_error_1:
    free(p_mbrc_buf);
    return -1;
}

static afinfo_t const * _act_try_get_afinfo_of_dev(block_dev_desc_t const *dev_desc,
        act_afi_partn_obj_t *p_partn_obj)
{
    afinfo_t *p_ret;
    int ret;

    if((dev_desc->if_type==IF_TYPE_ACT_NANDBLK && dev_desc->dev==0 &&
            ACTS_AFINFO_PTR->boot_dev==ACTS_BOOTDEV_NAND) ||
       (dev_desc->if_type==IF_TYPE_MMC && dev_desc->dev==2 &&
            ACTS_AFINFO_PTR->boot_dev==ACTS_BOOTDEV_SD2) ||
       (dev_desc->if_type==IF_TYPE_MMC && dev_desc->dev==0 &&
            ACTS_AFINFO_PTR->boot_dev==ACTS_BOOTDEV_SD0))
    {
        // boot from from this device
        // the AFINFO in used (kept in gd) is from this device.
        memcpy(&(p_partn_obj->afinfo_backup), ACTS_AFINFO_PTR, sizeof(afinfo_t));
        p_ret = &(p_partn_obj->afinfo_backup);
    }
    else
    {
        p_ret = NULL;
        // not boot from this device
        // if this device is a MMC, try to read the AFINFO structure.
        if(dev_desc->if_type==IF_TYPE_MMC || dev_desc->if_type==IF_TYPE_SD)
        {
            ret = _afi_read_afinfo_from_blk_storage(dev_desc, &(p_partn_obj->afinfo_backup));
            if(ret == 0)
            {
                // found AFINFO structure on this device
                p_ret = &(p_partn_obj->afinfo_backup);
            }
        }
    }

    return p_ret;
}

// API for others
// We provide this function to others in order to reuse the code here.
// return :  0:OK  -1:no_card  -2:no_afinfo
int act_afi_get_afinfo_from_blk_storage(const char *dev_ifname, int dev, afinfo_t *p_afinfo_buf)
{
    block_dev_desc_t const *dev_desc;
    act_afi_partn_obj_t *p_partn_obj;
    int ret;

    // get device (this will also check partition format...)
    dev_desc = get_dev(dev_ifname, dev);
    if (dev_desc == NULL)
    {
        printf("%s: can not get dev %s %d\n", __FUNCTION__, dev_ifname, dev);
        return -1;
    }

    /* if this device is already checked here, use the cached result. */
    p_partn_obj = _afi_get_partn_obj(dev_desc, 0);
    if(p_partn_obj != NULL)
    {
        memcpy(p_afinfo_buf, &(p_partn_obj->afinfo_backup), sizeof(afinfo_t));
        return 0;
    }

    /* else (eg., the device may have AFINFO structure, but applied an MBR partition),
     * raw read the storage. */
    ret = _afi_read_afinfo_from_blk_storage(dev_desc, p_afinfo_buf);
    return (ret == 0) ? 0 : -2;
}

static int _act_init_afi_partn_tbl(block_dev_desc_t const *dev_desc)
{
    afinfo_t const *p_dev_afinfo;
    act_afi_partn_obj_t *p_partn_obj;
    partition_info_t *p_raw_partn_info;
    uint cur_partn_idx, cur_offset, partn_cap;
    uint total_cap, cap_usage, log2blksz;
    int var_partn_idx;

    if(dev_desc->lba == (typeof(dev_desc->lba))(-1))
    {
        printf("%s: it seems the capacity (size) of device(if=%d,dev=%d) is faked, "
                "this may cause problem\n",
                __FUNCTION__, dev_desc->if_type, dev_desc->dev);
    }

    // check block size
    if(dev_desc->blksz > 1024*1024)
    {
        printf("%s: blk_size (%lu) not support\n", __FUNCTION__, dev_desc->blksz);
        return -1;
    }
    log2blksz = dev_desc->log2blksz;

    // get capacity of the device
    if(dev_desc->if_type == IF_TYPE_MMC)
    {
        // for MMC
        total_cap = dev_desc->lba;
        // reserved 1MiB for miscinfo storage.
        total_cap -= (1024 * 1024) >> log2blksz;
    }
    else
    {
        // for NAND
        total_cap = dev_desc->lba;
    }

    p_partn_obj = _afi_get_partn_obj(dev_desc, 1);
    BUG_ON(p_partn_obj ==NULL);

    p_dev_afinfo = _act_try_get_afinfo_of_dev(dev_desc, p_partn_obj);
    if(p_dev_afinfo == NULL)
    {
        // not support device.
        debug("%s: no AFINFO structure in this device, not actafi partition\n",
                __FUNCTION__);
        goto label_err_lv1;
    }
    p_raw_partn_info = (partition_info_t *)(p_dev_afinfo->partition_info);

    var_partn_idx = -1;
    cap_usage = 0;
    cur_partn_idx = 0;
    while(cur_partn_idx < CONFIG_ACTS_ACTAFI_PARTN_CN)
    {
        debug("%s: raw_partn define: flash_ptn=%d partition_num=%d partition_cap=%u\n",
                __FUNCTION__,
                p_raw_partn_info->flash_ptn,
                p_raw_partn_info->partition_num,
                p_raw_partn_info->partition_cap);

        if(p_raw_partn_info->flash_ptn ==
                (typeof(p_raw_partn_info->flash_ptn))(-1))
        {
            break; // end of table
        }
        /* FIXME
         * note: according to the format spec, there should be a -1 entry in the end,
         * but the implement else where is wrong, and the afi partition table here
         * we see is lacking of that -1 entry, so we can only assume the table is
         * fixed 12 entries, this ugliness is cause by others outside u-boot. */

        if(p_raw_partn_info->flash_ptn > 0) // logical ?
        {
            if(p_raw_partn_info->partition_cap ==
                    (typeof(p_raw_partn_info->partition_cap))(-1))
            {
                if(var_partn_idx < 0)
                {
                    var_partn_idx = cur_partn_idx;
                }
                else
                {
                    printf("%s: err, more than 1 vari_partn\n", __FUNCTION__);
                    goto label_err_lv1;
                }
                partn_cap = 0;
            }
            else
            {
                partn_cap = (p_raw_partn_info->partition_cap * 1024U*1024U) >> log2blksz;
            }
        }
        else
        {
            if(dev_desc->if_type == IF_TYPE_ACT_NANDBLK)
            {
                partn_cap = 0; // no access, not in logical area
            }
            else
            {
                partn_cap = (p_raw_partn_info->partition_cap * 1024U*1024U) >> log2blksz;
            }
        }

        p_partn_obj->partn_tbl[cur_partn_idx].cap = partn_cap;
        cap_usage += partn_cap;
        cur_partn_idx++;
        p_raw_partn_info++;
    }

    p_partn_obj->partn_cnt = cur_partn_idx;

    // check total_cap
    if(cap_usage > total_cap)
    {
        printf("%s: partition overflow, need %u but only have %u\n",
                __FUNCTION__, cap_usage, total_cap);
        goto label_err_lv1;
    }

    // adjust the variable partition
    if(var_partn_idx >= 0)
    {
        if(total_cap <= cap_usage)
        {
            printf("%s: err, no more space left for vari_partition #%d\n",
                    __FUNCTION__, var_partn_idx);
            goto label_err_lv1;
        }
        p_partn_obj->partn_tbl[var_partn_idx].cap = total_cap - cap_usage;
    }

    // calculate the offset (in blocks)
    cur_offset = 0;
    for(cur_partn_idx=0; cur_partn_idx < p_partn_obj->partn_cnt; cur_partn_idx++)
    {
        p_partn_obj->partn_tbl[cur_partn_idx].offset = cur_offset;
        cur_offset += p_partn_obj->partn_tbl[cur_partn_idx].cap;

        debug("%s: found partition, lba=%u len=%u\n", __FUNCTION__,
                p_partn_obj->partn_tbl[cur_partn_idx].offset,
                p_partn_obj->partn_tbl[cur_partn_idx].cap);
    }

    return 0;

    label_err_lv1:
    _afi_destroy_partn_obj(p_partn_obj);
    return -1;
}

static int _act_get_partition_info(
        block_dev_desc_t const *dev_desc, int partn_idx, disk_partition_t * info,
        uint keep_silence)
{
    act_afi_partn_obj_t *p_partn_obj;

    p_partn_obj = _afi_get_partn_obj(dev_desc, 0);
    if(p_partn_obj == NULL)
    {
        if(!keep_silence)
        {
            printf("%s: partition not found: device(if=%d,dev=%d) partn #%u\n",
                    __FUNCTION__, dev_desc->if_type, dev_desc->dev, partn_idx);
        }
        return -1;
    }
    if(partn_idx >= p_partn_obj->partn_cnt)
    {
        if(!keep_silence)
        {
            printf("%s: partition not found: device(if=%d,dev=%d) partn #%u\n",
                    __FUNCTION__, dev_desc->if_type, dev_desc->dev, partn_idx);
        }
        return -1;
    }

    info->start = p_partn_obj->partn_tbl[partn_idx].offset;
    info->size  = p_partn_obj->partn_tbl[partn_idx].cap;
    info->blksz = (1U << p_partn_obj->backing_log2blksz);
    strncpy((char*)(info->name), s_partion_name_tbl[partn_idx], sizeof(info->name)-1);
    strncpy((char*)(info->type), BOOT_PART_TYPE, sizeof(info->type)-1);
    info->bootable = 0;
#ifdef CONFIG_PARTITION_UUIDS
    info->uuid[0] = 0;
#endif

    return 0;
}

int get_partition_info_actafi(block_dev_desc_t * dev_desc, int part_num, disk_partition_t * info)
{
    assert(part_num > 0);
    return _act_get_partition_info(dev_desc, part_num-1U, info, 0);
}

void print_part_actafi(block_dev_desc_t * dev_desc)
{
    disk_partition_t info;
    uint i;

    puts("part#  offset(blk)     cap(blk)  blksz(byte)   name\n");
    puts("------------------------------------------------------------\n");
    printf("   #0   %10lu   %10lu   %10lu   whole_dev(if=%d,dev=%d)\n",
            0lu, dev_desc->lba, dev_desc->blksz, dev_desc->if_type, dev_desc->dev);

    i = 0;
    while(_act_get_partition_info(dev_desc, i , &info, 1) == 0)
    {
        printf("   %2x   %10lu   %10lu   %10lu   %s\n",
                i+1, info.start, info.size, info.blksz, info.name);
        i++;
    }
    puts("notes :\n");
    puts("1. partition number starts from 1 in u-boot, "
            "and #0 is for the entire device\n");
    puts("2. in nand device, physical partition is not accessible, "
            "and its size is marked as 0\n");
}

int test_part_actafi(block_dev_desc_t *dev_desc)
{
    return _act_init_afi_partn_tbl(dev_desc);
}
