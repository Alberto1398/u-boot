/*
=================================================================================     
*                        
* FileName: _GL5202_DDR3_32bits_init_DLL_ON.c   parameter: freq
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
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"
#include "_ddr_timing_parameter.h"


extern void adfu_printf(const char *fmt, ...);
#define adfu_printf(stuff...)		do{}while(0)

int _GL5202_DDR3_init(unsigned int ddr_type,unsigned int freq,unsigned int single_dram_cap,unsigned int ddr_drv0,unsigned int ddr_drv1,unsigned int boot_mode,unsigned int freq_init,unsigned char ddrmisc)
{
	struct timing_parameter timing,timing_init;
	struct timing_parameter *pTiming,*pTiming_init;
	unsigned int tmp_reg_value,reg_value,freq_init_use;
	//unsigned int tmp_cyc_ie;
	//unsigned int freq_init = 300;
	//unsigned int freq_init = 240;
	//single_dram_cap:   0:512Mbit ;1:1Gbit;2:2Gbit;4:4Gbit;8:8Gbit
	
	pTiming = &timing;
	pTiming_init = &timing_init;
/*
=================================================================================
*   power init
=================================================================================
*/  	
//	init_atc2603_power();
//	init_DDR_power();
/*
=================================================================================
*   config CMU register
=================================================================================
*/ 
    switch(freq_init)
	{
		case 0:
			{
				freq_init_use = 240;
				break;
			}
		case 1:
			{
				freq_init_use = 300;
				break;
			}
		default:
			{
				freq_init_use = 240;
				break;
			}
	}


  adfu_printf("_GL5202_DDR3_init start");

  _timing_parameter_genarate_ddr3(freq_init_use,pTiming_init,single_dram_cap,ddr_drv0,ddr_drv1);
  _timing_parameter_genarate_ddr3(freq,pTiming,single_dram_cap,ddr_drv0,ddr_drv1);
	tmp_reg_value = Read_reg(CMU_DEVCLKEN0);
	tmp_reg_value = tmp_reg_value|0x1004000A;
	Write_reg(CMU_DEVCLKEN0,tmp_reg_value);
	
	//_delay_func(0x100);
	udelay(20);
	
	tmp_reg_value = freq_init_use/6+DDRPLLEN;
	Write_reg(CMU_DDRPLL,tmp_reg_value);
	
	//delay_func(0x1000);
	udelay(500);
	
	tmp_reg_value = Read_reg(CMU_DEVRST0);
	tmp_reg_value = tmp_reg_value&0xfffffffb;
	Write_reg(CMU_DEVRST0,tmp_reg_value);
	
	//_delay_func(0x800);
	udelay(100);
	
	tmp_reg_value = Read_reg(CMU_DEVRST0);
	tmp_reg_value = tmp_reg_value|0x00000004;
	Write_reg(CMU_DEVRST0,tmp_reg_value);
	
	//_delay_func(0x100);
	udelay(50);
	
/*
=================================================================================
*   config DCU register
=================================================================================
*/	
	
	tmp_reg_value = Read_reg(DCU_EN);
	tmp_reg_value = tmp_reg_value&0x10;
	Write_reg(DCU_EN,tmp_reg_value);
	
	if(boot_mode != 1)
	{
		/* normal mode */
		tmp_reg_value = Read_reg(DCU_EN);
		tmp_reg_value = tmp_reg_value&0x01;
		Write_reg(DCU_EN,tmp_reg_value);
	
		//_delay_func(0x2000);
		udelay(500);
  	}
/****************************modified by zhongxu 2013-01-17****************************************/	
	Write_reg(DCU_PAD_DRV0,pTiming_init->DRV0);
	Write_reg(DCU_PAD_DRV1,pTiming_init->DRV1);
	
	//dcu_cmd_apply(CMD_ZQC_IN);	
	
	//_delay_func(0x1000);
	udelay(200);
	
/****************************modified by zhongxu 2013-01-17****************************************/	
	
	
	tmp_reg_value = (pTiming_init->CLK_DLY)|(pTiming_init->IE_CEXT<<8)|(pTiming_init->OE_CDLY<<12)|(pTiming_init->OE_CEXT<<16)|(pTiming_init->WR_ODT_EXT<<20)|(pTiming_init->RD_ODT_EXT<<24);
	Write_reg(DCU_CLK_DLY,tmp_reg_value);
	
	
	Write_reg(DCU_CAL_CTRL,0x2000ffff);
	
	Write_reg(DCU_DQS_EN_DLY,0x80000000);
	
/*
=================================================================================
*   config write delay chain register
=================================================================================
*/	
	Write_reg(DCU_WR_DQS_DLY,(pTiming_init->WR_DQS_DLY));
	Write_reg(DCU_WR_DM_DLY,0x03030303);
	Write_reg(DCU_WR_DQ_DLY0,0x33333333);
	Write_reg(DCU_WR_DQ_DLY1,0x33333333);
	Write_reg(DCU_WR_DQ_DLY2,0x33333333);
	Write_reg(DCU_WR_DQ_DLY3,0x33333333);
	
/*
=================================================================================
*   config read delay chain register
=================================================================================
*/	
	Write_reg(DCU_RD_DQS_FIX,0x00007777);
	Write_reg(DCU_RD_DQS_DLY,(pTiming_init->RD_DQS_DLY));
	Write_reg(DCU_RD_DQ_DLY0,0x77777777);
	Write_reg(DCU_RD_DQ_DLY1,0x77777777);
	Write_reg(DCU_RD_DQ_DLY2,0x77777777);
	Write_reg(DCU_RD_DQ_DLY3,0x77777777);
	
	
	tmp_reg_value = VDD_DET_EN+CAP_16Gbit+BANK8+ODT_EXEN+AUPD;//+AUSR;// +Width_32+PAGE_4KB+CMD_REORDER
	switch(ddr_type)
	{
		case 0:
			{
				tmp_reg_value = tmp_reg_value+DDR3_SDRAM;//+AUSR;// +Width_32+PAGE_4KB+CMD_REORDER
				break;
			}
		case 1:
			{
				tmp_reg_value = tmp_reg_value+DDR3L_SDRAM;//+AUSR;// +Width_32+PAGE_4KB+CMD_REORDER
				break;
			}
		default:
			{
				tmp_reg_value = tmp_reg_value+DDR3_SDRAM;//+AUSR;// +Width_32+PAGE_4KB+CMD_REORDER
				break;
			}
	}
	
//default set Width_32 and PAGE_4KB
	tmp_reg_value = tmp_reg_value|(Width_32+PAGE_4KB);
	Write_reg(DCU_FEA,tmp_reg_value);
	
/*
=================================================================================
*   config ddr mode register
=================================================================================
*/
    tmp_reg_value = 0x20;
    if((pTiming_init->CL >=5 ) && (pTiming_init->CL <= 10))
        tmp_reg_value = 0x10*(pTiming_init->CL-4);
  
  switch(pTiming_init->tWR_ddr)
  {
  	case 5:
  		{
  			reg_value = 0x200;
  			break;
  		}
  	case 6:
  		{
  			reg_value = 0x400;
  			break;
  		}
  	case 7:
  		{
  			reg_value = 0x600;
  			break;
  		}
  	case 8:
  		{
  			reg_value = 0x800;
  			break;
  		}
  	case 10:
  		{
  			reg_value = 0xa00;
  			break;
  		}
  	case 12:
  		{
  			reg_value = 0xc00;
  			break;
  		}		
		default:
			{
				reg_value = 0xe00;
				break;
			}  
  }
  tmp_reg_value = tmp_reg_value|reg_value|0x1000;
  Write_reg(DCU_NMR,tmp_reg_value);
  

  Write_reg(DCU_EMR1,0x0004);
  //Write_reg(DCU_EMR1,0x0006);
  //Write_reg(DCU_EMR1,0x0040);
  //Write_reg(DCU_EMR1,0x0006);
  //Write_reg(DCU_EMR1,0x0042);
  //Write_reg(DCU_EMR1,0x0044);

    tmp_reg_value = 0x0;
    if((pTiming_init->CWL >=5 ) && (pTiming_init->CWL <= 10))
        tmp_reg_value = 0x8*(pTiming_init->CWL-5);
    
  Write_reg(DCU_EMR2,tmp_reg_value);
  Write_reg(DCU_EMR3,0x0);
  	
/*
=================================================================================
*   config ddr timing register
=================================================================================
*/	
	tmp_reg_value = (pTiming_init->tRCD)|(pTiming_init->tRRD<<4)|(pTiming_init->tWR<<8)|(pTiming_init->tMRD<<13)|(pTiming_init->tWTR<<16)|(pTiming_init->tRTW<<20)|(pTiming_init->tRP<<24)|(pTiming_init->tMRD_cmd<<28);
	Write_reg(DCU_TIME1,tmp_reg_value);
	
	tmp_reg_value = (pTiming_init->tRC)|(pTiming_init->tRAS<<8)|(pTiming_init->tCKE <<24)|(pTiming_init->tXP<<28);
	Write_reg(DCU_TIME2,tmp_reg_value);
	
	tmp_reg_value = (pTiming_init->tREFI)|(pTiming_init->AREFCNT<<16)|(pTiming_init->PRECNT <<20)|(pTiming_init->tRTP<<24)|(pTiming_init->tCKSR<<28);
	Write_reg(DCU_TIME3,tmp_reg_value);
	
	tmp_reg_value = (pTiming_init->tFAW)|(pTiming_init->t_iodt_sta<<8)|(pTiming_init->t_ibias_sta <<12)|(pTiming_init->tRFC<<16);
	Write_reg(DCU_TIME4,tmp_reg_value);
	
	tmp_reg_value = 0x600;
	tmp_reg_value = tmp_reg_value|(pTiming_init->tXS<<16);
	Write_reg(DCU_SR_CFG,tmp_reg_value);
	Write_reg(DCU_CAL_ADDR,0x1000);
	
	Write_reg(DCU_EN,0x11);
	
	//_delay_func(0x200);
	udelay(50);
	
	dcu_cmd_apply(CMD_INIT);
	
	dcu_cmd_apply(CMD_TIMING_ADJUST);
	
	//_delay_func(0x100);
	udelay(20);
	
	//dcu_cmd_apply(CMD_ZQC_IN);
	
//	Write_reg(DCU_PAD_DRV0,pTiming_init->DRV0);
//	Write_reg(DCU_PAD_DRV1,pTiming_init->DRV1);
//	
//	//dcu_cmd_apply(CMD_ZQC_IN);	
//	
//	_delay_func(0x1000);
//  udelay(200);
	
	dcu_cmd_apply(CMD_INIT);
	
	//_delay_func(0x100);
	udelay(20);
	
	dcu_cmd_apply(CMD_ZQCL_EX);	
	//_delay_func(0x400);
	udelay(60);
	
/*
=================================================================================
*   Preamble detect
=================================================================================
*/ 	
		
//	tmp_reg_value = preamble_detect_func(pTiming_init->IE_CDLY,pTiming_init->CYC_DLY);
//	
//	if(tmp_reg_value!=0)
//		{
//			reg_value = Read_reg(DCU_PREAMBLE_GPIPE_CTRL);
//	    reg_value = (reg_value&(~(0x3f<<10)))|(0x20<<10);
//	    Write_reg(DCU_PREAMBLE_GPIPE_CTRL,reg_value);
//			reg_value = preamble_detect_func(pTiming_init->IE_CDLY,pTiming_init->CYC_DLY);
//			if(reg_value!=0)
//				{
//					return -1;
//				}
//		}
		
	tmp_reg_value = Read_reg(DCU_FEA);
	tmp_reg_value = tmp_reg_value|(ODTIN_VALUE_120+ODT_INEN);
	Write_reg(DCU_FEA,tmp_reg_value);
	

//********************************for mode register set****************************************//
    tmp_reg_value = 0x20;
    if((pTiming->CL >=5 ) && (pTiming->CL <= 10))
        tmp_reg_value = 0x10*(pTiming->CL-4);
  
  switch(pTiming->tWR_ddr)
  {
  	case 5:
  		{
  			reg_value = 0x200;
  			break;
  		}
  	case 6:
  		{
  			reg_value = 0x400;
  			break;
  		}
  	case 7:
  		{
  			reg_value = 0x600;
  			break;
  		}
  	case 8:
  		{
  			reg_value = 0x800;
  			break;
  		}
  	case 10:
  		{
  			reg_value = 0xa00;
  			break;
  		}
  	case 12:
  		{
  			reg_value = 0xc00;
  			break;
  		}		
		default:
			{
				reg_value = 0xe00;
				break;
			}  
  }
  tmp_reg_value = tmp_reg_value|reg_value|0x1000;
  Write_reg(DCU_NMR,tmp_reg_value);
  
  dcu_cmd_apply(CMD_NMRS);
 
    tmp_reg_value = 0x0;
    if((pTiming->CWL >=5 ) && (pTiming->CWL <= 10))
        tmp_reg_value = 0x8*(pTiming->CWL-5);

  Write_reg(DCU_EMR2,tmp_reg_value);
  dcu_cmd_apply(CMD_EMRS2);
	
//********************************for mode register set****************************************//	
	
	
	adfu_printf("_GL5202_DDR3_init end");
	reg_value = _ddr_freq_change(freq,single_dram_cap,ddr_drv0,ddr_drv1,ddrmisc);
	if(reg_value!=0)
		{
			return -1;
		}
		
	return 0;	
}
