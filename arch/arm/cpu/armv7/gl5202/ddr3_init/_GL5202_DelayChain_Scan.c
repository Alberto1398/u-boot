/*
=================================================================================     
*                        GL5202_DelayChain_Scan
* FileName: GL5202_DelayChain_Scan.c
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2012-09-05
* Description:  GL5202_DelayChain_Scan on arm platform
* History :
*   v1.0    create this file    2011-12-29
=================================================================================  
*/

#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"
#include "_ddr_timing_parameter.h"

extern void adfu_printf(const char *fmt, ...);
#define adfu_printf(stuff...)		do{}while(0)

int _GL5202_DelayChain_Set(ddr_param_t const *cfg)
{
	int ret=0;
	int val_o=0,val=0;
	
	//set clk_dly
	val_o = cfg->wr_dqs;
	Write_reg(DCU_CLK_DLY, (Read_reg(DCU_CLK_DLY) & ~0xff) | val_o);
		
	//set wr_dqs_dly
	val = val_o + (val_o<<8) + (val_o<<16) + (val_o<<24);	
	Write_reg(DCU_WR_DQS_DLY, val);
	
	//set rd_dqs_dly	
	val_o = cfg->rd_dqs;
	val = val_o + (val_o<<8) + (val_o<<16) + (val_o<<24);	
	Write_reg(DCU_RD_DQS_DLY, val);
	
	//set DCU bit15 1, �ж�1-->0
	Write_reg(DCU_CMD, Read_reg(DCU_CMD) | (1<<15));
	
	while(Read_reg(DCU_CMD) & (1<<15));
	
	return ret;
}

int _GL5202_DelayChain_Scan(unsigned int srcAddr,unsigned int dstAddr,unsigned int dataLen,unsigned int freq)
{
  int dqs_scan_ok;
  
/*============================================================================================
*********************************default dqs delay chain check*******************************
============================================================================================*/	    
	_test_data_init(srcAddr,dataLen);
	_GL5202_BDMA0_TRANSFER(srcAddr,dstAddr,dataLen,(DMA_DTrg_DDR+DMA_STrg_DDR));
	dqs_scan_ok = _GL5202_data_compare_byte(srcAddr,dstAddr,dataLen);
	if(dqs_scan_ok != 0)
  	{
  		return -1;
  	}
/*============================================================================================
*************************************dqs output delay scan**********************************
============================================================================================*/	  
  // dqs out delay chain scan start 
	dqs_scan_ok = _ddr_delaychain_check(srcAddr,dstAddr,dataLen,0x14,1,freq);//1:od scan,0:id scan;od scan start:0x14
  if(dqs_scan_ok != 0)
  	{
  		adfu_printf("DDR output delaychain scan error!\n");
  		return -1;
  	}
  	
  adfu_printf("DDR output delaychain scan ok!\n");
  
  // dqs out delay chain scan end	
/*============================================================================================
*************************************dqs output delay scan**********************************
============================================================================================*/	    
  // dqs in delay chain scan start
	//dqs_scan_ok = _ddr_delaychain_check(dstAddr,srcAddr,dataLen,0x9,0,freq);//1:od scan,0:id scan,od scan start:0x09
	dqs_scan_ok = _ddr_delaychain_check(dstAddr,srcAddr,dataLen,0xc,0,freq);//1:od scan,0:id scan,od scan start:0x0c
	if(dqs_scan_ok != 0)
  {
  	adfu_printf("DDR input delaychain scan error!\n");
  	return -1;
  }
	// dqs in delay chain scan end
	
	adfu_printf("DDR input delaychain scan ok!\n");
	
	return 0; 
}
        
    
    


    

