/*
=================================================================================     
*                        
* FileName: _ddr_delaychain_check.c   parameter: r0(BDMA_SRC),r1(BDMA_DST),r2(BDMA_LEN),r3(StartScanDly),r4(od_id)
* Return value : r0(check ok or error), r1(best od dm value)
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2012-09-07
* Description:  check the ddr dm output delay chain set which should be used
* History :
*   v1.0    create this file    2012-09-07
=================================================================================  
*/

#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"
#include "_ddr_timing_parameter.h"

int od_first_ok_dly, od_last_ok_dly;
int id_first_ok_dly, id_last_ok_dly;

int _ddr_delaychain_check(unsigned int srcAddr,unsigned int dstAddr,unsigned int dataLen,unsigned int StartScanDly,unsigned int od_id,unsigned int freq)
{
//srcAddr:source address
//dstAddr: dst address
//dataLen: data length
//StartScanDly: start scan delay chain
//od_id:	1:od;0:id
	unsigned int dqs_dly_bak,clk_dly_bak;
	unsigned int clk_dly_tmp,dqs_dly_tmp;
	unsigned int dqs_dly_current,clk_dly_current;
	unsigned int dqs_window_width;
	int current_dly = 0;
	int first_ok_dly= 0;
	int last_ok_dly = 0;
	int mid_ok_dly = 0;
	int ok_dly_num = 0;
	int data_check_result;
	int GotFirstOk = 0;
	
	if(od_id)
		{
	    dqs_dly_bak  = Read_reg(DCU_WR_DQS_DLY);       //save the DCU_WR_DQS_DLY register value
    }
  else
  	{
	    dqs_dly_bak  = Read_reg(DCU_RD_DQS_DLY);       //save the DCU_RD_DQS_DLY register value
    }
  
	clk_dly_bak  = Read_reg(DCU_CLK_DLY);       //save the DCU_CLK_DLY register value
  clk_dly_tmp = clk_dly_bak & 0xffffff00; 

	//while(current_dly<=27)
	//while(current_dly<=25)
	//dqs_window_width = 3750000/(freq*347);//(10000000*3)/(freq*2*347*4)
	//dqs_window_width = 3500000/(freq*347);//(10000000*7)/(freq*2*347*10)
	dqs_window_width = 3250000/(freq*347);//(10000000*13)/(freq*2*347*20)// 13/20
	while(current_dly<=dqs_window_width)
	{		
		dqs_dly_tmp = StartScanDly+current_dly;
		dqs_dly_current = dqs_dly_tmp |(dqs_dly_tmp<<8) | (dqs_dly_tmp<<16) |(dqs_dly_tmp<<24);
	
	  if(od_id)
	  	{
	  		clk_dly_current  = clk_dly_tmp + dqs_dly_tmp;
	  		Write_reg(DCU_CLK_DLY,clk_dly_current);
		    Write_reg(DCU_WR_DQS_DLY,dqs_dly_current);
	  	}
	  else
	  	{
	  		Write_reg(DCU_RD_DQS_DLY,dqs_dly_current);
	  	}		
		current_dly++;
	
	  dcu_cmd_apply(CMD_TIMING_ADJUST);
	
		_GL5202_BDMA0_TRANSFER(srcAddr,dstAddr,dataLen,(DMA_DTrg_DDR+DMA_STrg_DDR));
		data_check_result = _GL5202_data_compare_byte(srcAddr,dstAddr,dataLen);
		if(data_check_result==0)
			{
				ok_dly_num++;
				if(GotFirstOk == 0)
					{
						if(ok_dly_num>7)
							{
								GotFirstOk = 1;
								first_ok_dly = current_dly-7;
							}
						last_ok_dly = current_dly;
					}
				else
					{
						last_ok_dly = current_dly;
					}					
			}
		else
			{
			   if(GotFirstOk)
			   	{
			   		break;
			   	}
			   	ok_dly_num = 0;		
			}
	}
  ok_dly_num =  last_ok_dly- first_ok_dly;
  mid_ok_dly =  (last_ok_dly+ first_ok_dly)/2;
  if(ok_dly_num <= 7)
  	{
  		
  		if(od_id)
  			{
  	    	Write_reg(DCU_CLK_DLY,clk_dly_bak);
			    Write_reg(DCU_WR_DQS_DLY,dqs_dly_bak);
		    }
		  else
		  	{
		  		Write_reg(DCU_RD_DQS_DLY,dqs_dly_bak);
		  	}

	    dcu_cmd_apply(CMD_TIMING_ADJUST); 
	    
  		return -1;
  	}
  else
  	{
        if(od_id)
        {
            od_first_ok_dly = first_ok_dly;
            od_last_ok_dly = last_ok_dly;
        }
        else
        {
            id_first_ok_dly = first_ok_dly;
            id_last_ok_dly = last_ok_dly;           
        }       
		  dqs_dly_tmp = StartScanDly+mid_ok_dly;
		  dqs_dly_current = dqs_dly_tmp |(dqs_dly_tmp<<8) | (dqs_dly_tmp<<16) |(dqs_dly_tmp<<24);
  		
  		if(od_id)
  			{
  				clk_dly_current  = clk_dly_tmp + dqs_dly_tmp;
  				Write_reg(DCU_CLK_DLY,clk_dly_current);
			    Write_reg(DCU_WR_DQS_DLY,dqs_dly_current);
  			}
  		else
  			{
  				Write_reg(DCU_RD_DQS_DLY,dqs_dly_current);
  			}
	    dcu_cmd_apply(CMD_TIMING_ADJUST);  
	    
	    return 0;
  	}
}

