/*
=================================================================================
*
* FileName: _GL5202_DDR_init_main.c
* parameter: freq,
             ddr3L,
             boot,
             single_dram_cap, 0:512Mbit ;1:1Gbit;2:2Gbit;4:4Gbit;8:8Gbit
* Return value : r0(init complete or not)
* Author  : zhongxu
* Version : v1.0
* Chip    : GL5202
* Date    : 2012-10-15
* Description:
* History :
*   v1.0    create this file    2012-10-15
=================================================================================
*/

#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_atc2603.h>
#include <asm/arch/actions_reg_rw.h>
#include "_ddr_timing_parameter.h"
#include "gl5202_ddr.h"

/* 依赖于外部的函数 */
extern uint act_260x_reg_read(uint addr);
extern void act_260x_reg_write(uint addr, uint data);
extern void adfu_printf(const char *fmt, ...);
#define adfu_printf(stuff...)		do{}while(0)

/* for DCU_FEA */
static const unsigned int ddr_cap[] =
    {32, 64, 128, 256, 512, 1024, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static inline int fls(int x)
{
	int ret;

	asm("clz\t%0, %1" : "=r" (ret) : "r" (x));
       	ret = 32 - ret;
	return ret;
}

#define ffs(x) ({ unsigned long __t = (x); fls(__t & -__t); })
#define __ffs(x) (ffs(x) - 1)

/* int fix_ddr_cap(void)
{
    int bond_opt;
    unsigned int tmp_reg_value;
    
    bond_opt = Read_reg(0xb01c00e0) & 0xf;
    switch (bond_opt)
    {
    case 0xf:
        break;
    case 0xe:   //up to 512M
        {
            tmp_reg_value = (Read_reg(DCU_FEA) & 0xf0) >>4;    
            tmp_reg_value = tmp_reg_value > 4 ? 4:tmp_reg_value;
            Write_reg(DCU_FEA, (Read_reg(DCU_FEA) & ~0xf0) | (tmp_reg_value<<4));
        }
        break;        
    default:
        break;
    }

    return 0;
} */

int _DDR_init(ddr_param_t const *cfg, unsigned int boot_mode, unsigned int *p_total_cap)
{
    unsigned int width,dram_num, single_dram_cap,tmp_reg_value;
    //unsigned int freq_init;
    unsigned int total_cap_mbyte;
    unsigned char ddrmisc;
    unsigned short vddr_val=0x80af;
    unsigned short DC2_VOL=0;  
    unsigned char fix_width_16bit;
    int ret;
    if ( cfg->ddr_auto_scan & 0x01)// if config for ddr scan.
    {// add by liaotianyang , 2013-11-9
        boot_mode = 0;
        //cfg->rd_dqs = 0;
        //cfg->wr_dqs = 0; 
    }
    
    //single_ddr_width = (cfg->single_ddr_width);     /* width: 0:8bit,1:16bit,2:4bit */
    fix_width_16bit = (cfg->ddr_misc & 0x2)>>1;    //fix_width_16bit 2014-6-6
    //fix_width_16bit = cfg->dram_num;
    ddrmisc = cfg->ddr_misc & 0x1;

    adfu_printf("ddr_type: %d,freq: %d, single_ddr_width %d, fix_width_16bit %d, ddr_drv0 %d, ddr_drv1 %d\n", \
        cfg->ddr_type,cfg->ddr_clk, cfg->single_ddr_width, fix_width_16bit,cfg->ddr_drv0,cfg->ddr_drv1);   

    //0:probatch,1:s2,2:cold boot
	
    /* default single ddr chip capacity, Gbit */
    single_dram_cap = 8;

#if 1
/**************************************set ddr3:vddr = 1.55v;ddr3L vddr=1.35v*******************************************/
    if(cfg->ddr_vddr == 0)
    {
        DC2_VOL = 0x5; //ddr3 vddr=1.55v
        if(cfg->ddr_type == 1)
        {
            DC2_VOL = 0x1; //ddr3L vddr=1.35v
        }
    }
    else
    {
        DC2_VOL = cfg->ddr_vddr - 1;  //ddr_vddr<=[1,14]:1.3+(ddr_vddr-1)*0.05v;ddr_vddr=15,2.05v;ddr_vddr=16,2.15v
    }
    vddr_val |= DC2_VOL<<8;
    act_260x_reg_read(atc2603_PMU_DC2_CTL0);
    act_260x_reg_write(atc2603_PMU_DC2_CTL0, vddr_val);
#endif
  	ret = _GL5202_DDR3_init(cfg->ddr_type,cfg->ddr_clk,single_dram_cap,cfg->ddr_drv0,cfg->ddr_drv1,boot_mode,cfg->freq_init,ddrmisc);//add by zhongxu 
    if( ret != 0 ) {
    	adfu_printf("First time ddr3 init error\n");
    	return ret;
    }

    /*****************************caculate the dram num*****************************/
    
    width = Read_reg(DCU_FEA) & (0x01000000);
    if(width==0x01000000)
    {
    	dram_num = (cfg->single_ddr_width)?2:4;
    	if(fix_width_16bit==1)
    		{
    			dram_num = dram_num>>1;
    			tmp_reg_value = Read_reg(DCU_FEA);
      		tmp_reg_value = (tmp_reg_value & ~0x01000006) | (Width_16+PAGE_2KB);
      		Write_reg(DCU_FEA,tmp_reg_value);
    		}
    }
    else
    {
    	 if((cfg->single_ddr_width)==2)
    		{
    			dram_num = 4;
    			tmp_reg_value = (Read_reg(DCU_FEA)&0xfffffff9)|0x4;
    			Write_reg(DCU_FEA,tmp_reg_value);
    		}
    	else if((cfg->single_ddr_width)==0)
    		{
    			dram_num = 2;
    		}
    	else
    		{
    			dram_num =1;
    		}
        //dram_num = (cfg->single_ddr_width)?1:2;
    }

    /*****************************caculate the dram num*****************************/

	//s2 or cold boot
    if((boot_mode==1) || (boot_mode==2)){
		ret = _GL5202_DelayChain_Set(cfg);
        if( ret != 0 ) {
            adfu_printf("%s %d: ret %d\n", __FUNCTION__, __LINE__, ret);
            return -2;
        }

        tmp_reg_value = (Read_reg(DCU_FEA) & (~0xf0)) | ((__ffs(cfg->dram_cap) - 5) << 4);
        Write_reg(DCU_FEA, tmp_reg_value);
        
        goto out;
	  }

    if ((cfg->wr_dqs == 0 && cfg->rd_dqs == 0)||( cfg->ddr_auto_scan & 0x01)) {//add cfg->ddr_auto_scan & 0x01 by liaotianyang
        //probatch
        ret = _GL5202_DelayChain_Scan(0x0000,0x8000,0x8000,cfg->ddr_clk);
        if( ret != 0 ) {
            adfu_printf("%s %d: ret %d\n", __FUNCTION__, __LINE__, ret);
            return -3;
        }
    } else {
        //fixed cdc by user
		ret = _GL5202_DelayChain_Set(cfg);
        if( ret != 0 ) {
            adfu_printf("%s %d: ret %d\n", __FUNCTION__, __LINE__, ret);
            return -2;
        }
    }
        
    ret = _GL5202_DDR_CAP_DETECT();
    if( ret != 0 ) {
        adfu_printf("%s %d\n", __FUNCTION__, __LINE__);
        return -4;
    }

out:    
    /* get single ddr chip capacity, Gbit */
    tmp_reg_value = (Read_reg(DCU_FEA) & 0xf0) >>4;
    total_cap_mbyte = ddr_cap[tmp_reg_value];
    single_dram_cap = total_cap_mbyte * 8 / dram_num / 1024;

    /* reinit the timing by the detected capacity */
    ret = cap_timing_set(cfg->ddr_clk,single_dram_cap);
    if( ret != 0 ) {
        adfu_printf("%s %d\n", __FUNCTION__, __LINE__);
        return -5;
    }

    if(p_total_cap != 0) {
        *p_total_cap = total_cap_mbyte;
    }

    return 0;
}
