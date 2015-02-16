#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "brom_api.h"

#define SRAM_START_ADDR 0xb4068000
#define TRD_STAGE_MBRC_START    CONFIG_SYS_TEXT_BASE
#define SECTOR_SIZE             512U

#undef MY_ALIGN
#define MY_ALIGN(x,a) (((x)+(a)-1)&~((a)-1))


/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;



static unsigned int sectors_per_page;
static unsigned int pageAddrPerBlock;
static unsigned int pagePerBlock;


static int _init_nand_global_var(void)
{
    pageAddrPerBlock = *(volatile unsigned short *)(SRAM_START_ADDR+8);
    pagePerBlock = *(volatile unsigned short *)(SRAM_START_ADDR+10);
    sectors_per_page = *(volatile unsigned int *)(SRAM_START_ADDR+0xc);
    return 0;
}

static void _mmc_read(unsigned int lba, unsigned int buffer, unsigned int counts)
{
    int i = 0;
    int loops = 0;

    loops = (counts + 32 - 1) / 32;

    for(i=0; i<loops; i++)
    {
        act_brom_sdmmc_read_lba(lba, 32, buffer);
        lba += 32;
        buffer += 32*512;
    }
}

int act_load_third_mbrc(void)
{
    int i;
    unsigned int trd_stage_nand_offset_sector, trd_stage_sector_size;
    unsigned int total_mbrc_size, mbrc_size, sec_mbrc_size, trd_mbrc_size;
    unsigned int total_mbrc_sector_size_align;

    _init_nand_global_var();

    mbrc_size = 0x400; //sizeof of 1st_mbrc:0x200+0x200(reserved)
    sec_mbrc_size = *(unsigned int *)(SRAM_START_ADDR+0x28);
    total_mbrc_size = *(unsigned int *)(SRAM_START_ADDR+0x2c);
    trd_mbrc_size   = total_mbrc_size - sec_mbrc_size - mbrc_size;
    if(mbrc_size==0 || sec_mbrc_size==0 || total_mbrc_size==0 || trd_mbrc_size==0 ||
       (mbrc_size+sec_mbrc_size) >= total_mbrc_size)
    {
        printf("%s: mbrc hdr err\n", __FUNCTION__);
        return -1;
    }

    trd_stage_nand_offset_sector = (mbrc_size + sec_mbrc_size) / 512U;
    trd_stage_sector_size = (trd_mbrc_size + 511U) / 512U;

    //Only lsb page of mlc/tlc will be used to store mbrc.
    //The aligned mbrc size should be calculated by aligning with useable page count of the boot block.
    total_mbrc_sector_size_align = MY_ALIGN(total_mbrc_size>>9, pagePerBlock*sectors_per_page);
    for(i=0; i<1; i++)
    {
        uint32_t chksum, org_chksum;

        _mmc_read(trd_stage_nand_offset_sector+1 + (i*total_mbrc_sector_size_align), TRD_STAGE_MBRC_START, trd_stage_sector_size);
//        print_int(trd_stage_nand_offset_sector+1 + (i*total_mbrc_sector_size_align));
//        print_int(TRD_STAGE_MBRC_START);
//        print_int(trd_stage_sector_size);

//        print_int(*(unsigned int *)TRD_STAGE_MBRC_START);
//        print_int(*(unsigned int *)(TRD_STAGE_MBRC_START+(trd_stage_sector_size*SECTOR_SIZE)-4));

        chksum = act_calc_checksum32((uint8_t*)TRD_STAGE_MBRC_START, trd_mbrc_size - 4, 0);
        org_chksum = *((uint32_t*)(TRD_STAGE_MBRC_START + trd_mbrc_size - 4));
        if(chksum == org_chksum)
        {
            return 0;
        }

        printf("broken 3rd stage unit #%u, chksum err, org 0x%08x but now 0x%08x\n",
                i, org_chksum, chksum);
    }

    return -1;
}

void act_brom_drv_exit(void)
{
    act_brom_sdmmc_exit();
}

