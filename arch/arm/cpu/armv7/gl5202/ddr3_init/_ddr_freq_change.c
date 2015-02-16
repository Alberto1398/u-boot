/*
=================================================================================     
*                        
* FileName: _ddr_freq_change.c   parameter: freq
* Return value : r0(init complete or not)
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2012-09-24
* Description:  
* History :
*   v1.0    create this file    2012-09-07
=================================================================================  
*/

#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"
#include "_ddr_timing_parameter.h"

extern void adfu_printf(const char *fmt, ...);
#define adfu_printf(stuff...)		do{}while(0)

int _ddr_freq_change (unsigned int freq,unsigned int single_dram_cap,unsigned int driver0,unsigned int driver1,unsigned char ddrmisc)
{
	unsigned int tmp_reg_value,reg_value;
	unsigned int current_freq_state;
	unsigned int go_to_freq_state;
		
	struct timing_parameter timing;
	struct timing_parameter *pTiming;
	
	adfu_printf("_ddr_freq_change start");
	pTiming = &timing;
	_timing_parameter_genarate_ddr3(freq,pTiming,single_dram_cap,driver0,driver1);
	
/*
=================================================================================
*   comform the freq state
=================================================================================
*/
	
	tmp_reg_value = Read_reg(CMU_DDRPLL);
	tmp_reg_value = (tmp_reg_value&0xff)*6;
	current_freq_state = (tmp_reg_value>125);//1:dll on;0:dll off
	//go_to_freq_state = (freq>125);//1:dll on;0:dll off
	go_to_freq_state = ddrmisc?0:1;//1:dll on;0:dll off

/*
=================================================================================
*   when dll on to dll off ,turn off odt
=================================================================================
*/  	
  if(current_freq_state>go_to_freq_state)
  	{
  		tmp_reg_value = Read_reg(DCU_FEA);
			tmp_reg_value = (tmp_reg_value&0xffefefff);
			Write_reg(DCU_FEA,tmp_reg_value);
	
			tmp_reg_value = Read_reg(DCU_EMR1);
			tmp_reg_value = (tmp_reg_value&0xfffffdbb);
			Write_reg(DCU_EMR1,tmp_reg_value);
	
			dcu_cmd_apply(CMD_EMRS1);

/*
=================================================================================
*   turn off dll
=================================================================================
*/			
			tmp_reg_value = Read_reg(DCU_NMR);
			tmp_reg_value = tmp_reg_value&0xffffefff;
			Write_reg(DCU_NMR,tmp_reg_value);
			
			dcu_cmd_apply(CMD_NMRS);
			
			tmp_reg_value = Read_reg(DCU_EMR1);
			tmp_reg_value = (tmp_reg_value|0x1);
			Write_reg(DCU_EMR1,tmp_reg_value);
	
			dcu_cmd_apply(CMD_EMRS1);
			
  	}
  	
/*
=================================================================================
*   self refresh enter
=================================================================================
*/	
	dcu_cmd_apply(CMD_SREN);

/*
=================================================================================
*   change ddr pll
=================================================================================
*/
	
	tmp_reg_value = Read_reg(DCU_SR_CFG);
	tmp_reg_value = tmp_reg_value &0xffff;
	tmp_reg_value = tmp_reg_value|(pTiming->tXS<<16);
	Write_reg(DCU_SR_CFG,tmp_reg_value);
	
	
	tmp_reg_value = Read_reg(CMU_DDRPLL);
	tmp_reg_value = (tmp_reg_value&0xffffff00)|(freq/6);
	Write_reg(CMU_DDRPLL,tmp_reg_value);
	
    //_delay_func(1000);
	udelay(500);

/*
=================================================================================
*   self refresh exit
=================================================================================
*/	
	dcu_cmd_apply(CMD_SREX);
	
	if(current_freq_state<go_to_freq_state)
	{
/*
=================================================================================
*   turn on dll
=================================================================================
*/
		tmp_reg_value = Read_reg(DCU_EMR1);
		tmp_reg_value = (tmp_reg_value&0xfffffffe);
		Write_reg(DCU_EMR1,tmp_reg_value);
	
		dcu_cmd_apply(CMD_EMRS1);
/*
=================================================================================
*   reset dll
=================================================================================
*/
		tmp_reg_value = Read_reg(DCU_NMR);
		tmp_reg_value = tmp_reg_value&0xffffeeff;
		tmp_reg_value = tmp_reg_value|0x1100;        //reset dll
		Write_reg(DCU_NMR,tmp_reg_value);

		dcu_cmd_apply(CMD_NMRS);
		
	}
		
/*
=================================================================================
*   modify ddr mode register 0
=================================================================================
*/	
	
    tmp_reg_value = 0x20;
    if((pTiming->CL >=5 ) && (pTiming->CL <= 10))
        tmp_reg_value = 0x10*(pTiming->CL-4);
  reg_value = Read_reg(DCU_NMR);
	reg_value = reg_value&0xffffff8b;
	tmp_reg_value = reg_value|tmp_reg_value;
	
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
	
/*
=================================================================================
*   modify ddr mode register 2
=================================================================================
*/
	
    tmp_reg_value = 0x0;
    if((pTiming->CWL >=5 ) && (pTiming->CWL <= 10))
        tmp_reg_value = 0x8*(pTiming->CWL-5);
  reg_value = Read_reg(DCU_EMR2);
	reg_value = reg_value&0xffffffc7;
	tmp_reg_value = reg_value|tmp_reg_value;
  Write_reg(DCU_EMR2,tmp_reg_value);
  
	dcu_cmd_apply(CMD_EMRS2);

/*
=================================================================================
*   CMD_ZQCL_EX
=================================================================================
*/	 
	dcu_cmd_apply(CMD_ZQCL_EX);
	  	
/*
=================================================================================
*   config ddr timing register
=================================================================================
*/	
	tmp_reg_value = (pTiming->tRCD)|(pTiming->tRRD<<4)|(pTiming->tWR<<8)|(pTiming->tMRD<<13)|(pTiming->tWTR<<16)|(pTiming->tRTW<<20)|(pTiming->tRP<<24)|(pTiming->tMRD_cmd<<28);
	//tmp_reg_value = 0xffffffff;
	Write_reg(DCU_TIME1,tmp_reg_value);
	
	tmp_reg_value = (pTiming->tRC)|(pTiming->tRAS<<8)|(pTiming->tCKE <<24)|(pTiming->tXP<<28);
	//tmp_reg_value = 0xffffffff;
	Write_reg(DCU_TIME2,tmp_reg_value);
	
	tmp_reg_value = (pTiming->tREFI)|(pTiming->AREFCNT<<16)|(pTiming->PRECNT <<20)|(pTiming->tRTP<<24)|(pTiming->tCKSR<<28);
	//tmp_reg_value = (pTiming->tREFI)|(pTiming->AREFCNT<<16)|(pTiming->PRECNT <<20)|0xff000000;
	Write_reg(DCU_TIME3,tmp_reg_value);
	
	tmp_reg_value = (pTiming->tFAW)|(pTiming->t_iodt_sta<<8)|(pTiming->t_ibias_sta <<12)|(pTiming->tRFC<<16);
	//tmp_reg_value = (pTiming->t_iodt_sta<<8)|(pTiming->t_ibias_sta <<12)|0xffff00ff;
	Write_reg(DCU_TIME4,tmp_reg_value);
	
	
	Write_reg(DCU_PAD_DRV0,pTiming->DRV0);
	Write_reg(DCU_PAD_DRV1,pTiming->DRV1);
	
//	dcu_cmd_apply(CMD_ZQC_IN);
	
/*
=================================================================================
*   modify ddr delay chain
=================================================================================
*/	
  tmp_reg_value = (pTiming->CLK_DLY)|(pTiming->IE_CEXT<<8)|(pTiming->OE_CDLY<<12)|(pTiming->OE_CEXT<<16)|(pTiming->WR_ODT_EXT<<20)|(pTiming->RD_ODT_EXT<<24);
	Write_reg(DCU_CLK_DLY,tmp_reg_value);
	Write_reg(DCU_WR_DQS_DLY,(pTiming->WR_DQS_DLY));
	Write_reg(DCU_RD_DQS_DLY,(pTiming->RD_DQS_DLY));
	
	dcu_cmd_apply(CMD_TIMING_ADJUST);
	
/*
=================================================================================
*   Preamble detect
=================================================================================
*/  
/*************************** try width = 32bit start********************************/
  reg_value = 0;
  tmp_reg_value = Read_reg(DCU_FEA);
  tmp_reg_value = tmp_reg_value & (~(0x01000006));
  tmp_reg_value = tmp_reg_value|(Width_32+PAGE_4KB);
  Write_reg(DCU_FEA,tmp_reg_value);
  
	tmp_reg_value = preamble_detect_func(pTiming->IE_CDLY,pTiming->CYC_DLY);
	
	if(tmp_reg_value!=0)
		{
			//reg_value = Read_reg(DCU_PREAMBLE_GPIPE_CTRL);
	    //reg_value = (reg_value&(~(0x3f<<10)))|(0x20<<10);
	    //Write_reg(DCU_PREAMBLE_GPIPE_CTRL,reg_value);
			reg_value = preamble_detect_func((pTiming->IE_CDLY+1),(pTiming->CYC_DLY+1));
//			if(reg_value!=0)
//				{
//					return -1;
//				}
		}
/*************************** try width = 16bit start********************************/	
	if(reg_value!=0)
		{
	    reg_value = 0;
      tmp_reg_value = Read_reg(DCU_FEA);
      tmp_reg_value = tmp_reg_value & (~(0x01000006));
      tmp_reg_value = tmp_reg_value|(Width_16+PAGE_2KB);
      Write_reg(DCU_FEA,tmp_reg_value);
  
	    tmp_reg_value = preamble_detect_func(pTiming->IE_CDLY,pTiming->CYC_DLY);
	
	    if(tmp_reg_value!=0)
		   {
			//reg_value = Read_reg(DCU_PREAMBLE_GPIPE_CTRL);
	    //reg_value = (reg_value&(~(0x3f<<10)))|(0x20<<10);
	    //Write_reg(DCU_PREAMBLE_GPIPE_CTRL,reg_value);
			   reg_value = preamble_detect_func((pTiming->IE_CDLY+1),(pTiming->CYC_DLY+1));
			   if(reg_value!=0)
				  {
						return -1;
				  }
		   }
			
		}
/*************************** try width = 16bit end********************************/
	
	if(go_to_freq_state)
		{
			tmp_reg_value = Read_reg(DCU_FEA);
			tmp_reg_value = tmp_reg_value|(ODT_INEN+ODTIN_VALUE_120);   // turn on the ODT_IN
			Write_reg(DCU_FEA,tmp_reg_value);	
	  }
	
	if(current_freq_state<go_to_freq_state)
	{
		tmp_reg_value = Read_reg(DCU_FEA);
		tmp_reg_value = tmp_reg_value|(ODT_EXEN);   // turn on the ODT
		Write_reg(DCU_FEA,tmp_reg_value);	

		tmp_reg_value = Read_reg(DCU_EMR1);
		tmp_reg_value = (tmp_reg_value|0x4);
		Write_reg(DCU_EMR1,tmp_reg_value);

		dcu_cmd_apply(CMD_EMRS1);
	}

  	adfu_printf("_ddr_freq_change end");
	return 0;	
}

