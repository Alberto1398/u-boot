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


typedef enum
{
    slcMode_sharePage = 1,
}  Enum_slcModeType;

#define FlashPageIndex_2      (ACTS_AFINFO_PTR->lsb_tbl)

static unsigned char ecc_config;
static unsigned char ud_cnt;
static unsigned int sectors_per_page;
//static unsigned short size_per_page;
static unsigned char slc_mode;
static unsigned char tlc_enhanced_cmd;
static unsigned int pageAddrPerBlock;
static unsigned int pagePerBlock;


static int _init_nand_global_var(void)
{
    pageAddrPerBlock = *(volatile unsigned short *)(SRAM_START_ADDR+8);
    pagePerBlock = *(volatile unsigned short *)(SRAM_START_ADDR+10);
    sectors_per_page = *(volatile unsigned int *)(SRAM_START_ADDR+0xc);
    ecc_config = *(volatile unsigned char *)(SRAM_START_ADDR+0x18);
    ud_cnt = *(volatile unsigned char *)(SRAM_START_ADDR+0x19);
    slc_mode = *(volatile unsigned char *)(SRAM_START_ADDR+0x30);
    tlc_enhanced_cmd = *(volatile unsigned char *)(SRAM_START_ADDR+0x31);

    return 0;
}

static void _calc_rw_addr(act_brom_nand_read_param_t *param, unsigned int lba)
{
    unsigned int sec_num_in_page=0;
    unsigned int page_num_in_logic=0, page_num_in_blk, page_num_in_global;
    unsigned int block_num;

    sec_num_in_page = lba % sectors_per_page;
    param->col_addr = (sec_num_in_page << 9);

    page_num_in_logic = lba / sectors_per_page;
    page_num_in_blk = page_num_in_logic % pagePerBlock;

    /*get mapped page if nand is MLC or TLC*/
    if (slc_mode == slcMode_sharePage)
    {
        page_num_in_blk = FlashPageIndex_2[page_num_in_blk];
    }

    block_num = page_num_in_logic / pagePerBlock;

    page_num_in_global = block_num * pageAddrPerBlock + page_num_in_blk;

    param->row_addr_h = (page_num_in_global >> 16) & 0xffU;
    param->row_addr_l = page_num_in_global & 0xffffU;
}

static int _nand_page_read(unsigned int lba, unsigned int buffer, unsigned int secs)
{
    act_brom_nand_read_param_t param;
    int ret = 0;
    unsigned char * tmp;
    unsigned int row, col;
    unsigned int dma_ctl, reg_val;
    unsigned int loops, tmp_val;

    if (lba % sectors_per_page != 0)    //must aligned page!
        return -1;

    if(tlc_enhanced_cmd != 0)
    {
        act_writel(tlc_enhanced_cmd, NAND_CMD_FSM0);
        act_writel(0x60, NAND_CMD_FSM_CTL0);
        act_writel(1, NAND_FSM_START);
        while((act_readl(NAND_STATUS) & (1 << 31)));
    }

    _calc_rw_addr(&param, lba);
    tmp = (unsigned char *)&param;

    //1. set address.
    col = (tmp[1] << 8) | tmp[0];//main col
    row = (tmp[4] << 16) | (tmp[3] << 8) | tmp[2];
    act_writel(row, NAND_ROWADDR0);
    act_writel(0, NAND_ROWADDR1);   //
    col |= (sectors_per_page << 9) << 16;

    act_writel(col, NAND_COLADDR);

    //2. set dma
    dma_ctl = act_readl(NAND_DMA_CTL);
    dma_ctl |= (1 << 30);   //clear irq pending
    dma_ctl &= ~(1 << 29);  //disable irq
    dma_ctl &= ~(1 << 24);  //set axi increase
    dma_ctl &= ~(0xFFFF);
    if (ecc_config == 0)
    {
        act_writel(0x200, NAND_BC);
        loops = sectors_per_page;
    }
    else
    {
        act_writel(0x400, NAND_BC);
        loops = sectors_per_page / 2;
    }
    dma_ctl |= (sectors_per_page << 9);
    act_writel(dma_ctl, NAND_DMA_CTL);
    loops --;
    act_writel(buffer, NAND_DMA_ADDR);
    //3. bus2axi
    act_writel(act_readl(NAND_CTL) | (1 << 8), NAND_CTL);
    //4. set ecc type and userdata bytes.
    tmp_val = act_readl(NAND_CONFIG) & ~((3)| (7 << 4) | (7 << 8));
    act_writel(tmp_val | ((ecc_config) | (2 << 4) | (ud_cnt << 8)), NAND_CONFIG);
    //5. config row address in NAND_CONFIG.
    //6.1 00 30 in fsm0
    act_writel(0x3000, NAND_CMD_FSM0);
    act_writel(0x626C, NAND_CMD_FSM_CTL0);
    //6.2 05 E0 in fsm1
    act_writel(0xE005E005, NAND_CMD_FSM1);
    act_writel(0x71746164, NAND_CMD_FSM_CTL1);
    //6.3 ecc en/dis and randomizer choose.
    //6.4 start fsm
    //reg_val = (loops << 8) | (3 << 3) | (1 << 2) | 1;
    reg_val = (loops << 8) | (3 << 6) |(3 << 3) | (1 << 2) | 1;
    act_writel(reg_val, NAND_FSM_START);
    //6.5 start dma
    act_writel(act_readl(NAND_DMA_CTL) | (1 << 31), NAND_DMA_CTL);
    //wait transfer over here.
    while((act_readl(NAND_DMA_CTL) & (1 << 31)));
    while((act_readl(NAND_STATUS) & (1 << 31)));
    //7 axi2bus.
    act_writel(act_readl(NAND_CTL) & ~(1 << 8), NAND_CTL);
    //mini_serial_putw(('g'<< 8) | ('p'));

    return ret;
}

static void _nand_read(unsigned int lba, unsigned int buffer, unsigned int counts)
{
    act_brom_nand_read_param_t param;
    unsigned int bytes_cnt_per_read = 0x200;
    unsigned int i, lba_tmp;
    unsigned int ecc_sec = 1;

    if(ecc_config != 0)
    {
        bytes_cnt_per_read = 0x400;
        ecc_sec = 2;
    }

    /*init buffer and address*/
    param.buf_addr = buffer;
    lba_tmp = lba;
    for (i = 0; i < counts;)
    {
        if ((lba_tmp % sectors_per_page) || ((i + sectors_per_page) > counts))
        {
            //not page algined or no enough buffer for page.
            _calc_rw_addr(&param, lba_tmp);
            if(tlc_enhanced_cmd == 0xa2)
                act_brom_nand_lb_ttlc_read_unit(&param);
            else
                act_brom_nand_lb_read_unit(&param);
            lba_tmp += bytes_cnt_per_read >> 9;
            param.buf_addr += bytes_cnt_per_read;
            i += ecc_sec;
        }
        else //page read will speed up!
        {
            _nand_page_read(lba_tmp, param.buf_addr, sectors_per_page);
            lba_tmp += sectors_per_page;
            param.buf_addr += sectors_per_page << 9;
            i += sectors_per_page;
        }
    }
}

int act_load_third_mbrc(void)
{
    int i;
    unsigned int trd_stage_nand_offset_sector, trd_stage_sector_size;
    unsigned int total_mbrc_size, mbrc_size, sec_mbrc_size, trd_mbrc_size;
    unsigned int total_mbrc_sector_size_align;

    _init_nand_global_var();

    mbrc_size       = 0x400; //sizeof of 1st_mbrc:0x200+0x200(reserved)
    sec_mbrc_size   = *(unsigned int *)(SRAM_START_ADDR+0x28);
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
    //The aligned mbrc size should be calculated by aligning with usable page count of the boot block.
    total_mbrc_sector_size_align = MY_ALIGN(total_mbrc_size>>9, pagePerBlock*sectors_per_page);
    for(i=0; i<4; i++)
    {
        uint32_t chksum, org_chksum;

        _nand_read(trd_stage_nand_offset_sector + (i*total_mbrc_sector_size_align), TRD_STAGE_MBRC_START, trd_stage_sector_size);
        //ret = brom_mbrc_checksum(TRD_STAGE_MBRC_START, Brom_NANFBrecCheckSum, trd_stage_sector_size*SECTOR_SIZE);

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
    act_brom_nand_exit();
}
