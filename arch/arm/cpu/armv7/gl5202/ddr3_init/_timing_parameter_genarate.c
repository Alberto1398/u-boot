/*
=================================================================================     
*                        
* FileName: _timing_parameter_genarate.c   parameter: start_addr,data_len>=0x80
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
#include "_ddr_timing_parameter.h"
#include "gl5202_ddr.h"

extern void adfu_printf(const char *fmt, ...);
#define adfu_printf(stuff...)		do{}while(0)

unsigned int cycle_calc(unsigned int picosecond,unsigned int freq)
{
	unsigned int ret;
	ret = picosecond*freq/1000000+1;	
	return ret;
}


void dcu_cmd_apply(unsigned int dcu_cmd)
{
	
	adfu_printf("DCU_CMD is %8x\n",dcu_cmd);
	//_delay_func(0x200);
		
	Write_reg(DCU_CMD,dcu_cmd);
	while(Read_reg(DCU_CMD)==dcu_cmd)
	{
	}
}



struct single_dram_cap_timing *_single_dram_timing_genarate_ddr3(unsigned int freq,struct single_dram_cap_timing *parameter,unsigned int single_dram_cap)
{
	unsigned int temp;
	switch(single_dram_cap)	
  {
  	case 0:   //single dram capacity is 512Mbit
  		{
  			parameter->tRFC = cycle_calc(90000,freq);
  			temp = cycle_calc(100000,freq);
  			break;  
  		}
  	case 1:   //single dram capacity is 1Gbit
  		{
  			parameter->tRFC = cycle_calc(110000,freq);
  			temp = cycle_calc(120000,freq);
  			break;
  		}
  	case 2:   //single dram capacity is 2Gbit
  		{
  			parameter->tRFC = cycle_calc(160000,freq);
  			temp = cycle_calc(170000,freq);
  			break;
  		}
  	case 4:   //single dram capacity is 4Gbit
  		{
        parameter->tRFC = cycle_calc(260000,freq);
        temp = cycle_calc(270000,freq);
        break;
  		}
  	case 8:   //single dram capacity is 8Gbit
  		{
  			parameter->tRFC = cycle_calc(350000,freq);
  			temp = cycle_calc(360000,freq);
  			break;
  		}
  	default:  //default single dram capacity is 2Gbit
  		{
  			parameter->tRFC = cycle_calc(160000,freq);
  			temp = cycle_calc(170000,freq);
  			break;
  		}	
  }

  if(freq<125)
  	{	
       parameter->tXS = (temp>5) ? temp:5;
  	}
  else
  	{
  		parameter->tXS = 0x200;
  	}
  		
  return parameter;	
}




struct timing_parameter *_timing_parameter_genarate_ddr3(unsigned int freq,struct timing_parameter *parameter,unsigned int single_dram_cap,unsigned int ddr_drv0,unsigned int ddr_drv1)
{
	unsigned int temp;
	
	struct single_dram_cap_timing dram_timing;
	struct single_dram_cap_timing *pdram_timing;
	pdram_timing = &dram_timing;
	
	_single_dram_timing_genarate_ddr3(freq,pdram_timing,single_dram_cap);
		
/***********************************bdma0 transfer data init*****************************/
  parameter->tRCD = cycle_calc(15000,freq);
  parameter->CL = parameter->tRCD;
  if(freq>=533)
  	{
  		parameter->CWL = 7;
  	}
  else if(freq>=400)
    {
    	parameter->CWL = 6;
    }
  else if(freq<125)
  	{
  		parameter->CL= 6;
  		parameter->CWL = 6;
  		parameter->tRCD = 6;
  	}	
  else
  	{
  		parameter->CL = 6;
  		parameter->tRCD = 6;
    	parameter->CWL = 5;
  	} 
  	 
  if(freq>=400)
  	{
  		parameter->IE_CDLY = 3;
  		parameter->CYC_DLY = 4;
		}
	else if(freq>=180)
		{
			parameter->IE_CDLY = 2;
  		parameter->CYC_DLY = 3;
		}
	else if(freq>=132)
		{
			parameter->IE_CDLY = 1;
  		parameter->CYC_DLY = 2;
		}
	else
		{
			parameter->IE_CDLY = 0;
  		parameter->CYC_DLY = 1;
		}
	
  parameter->IE_CEXT = 3;
  parameter->OE_CDLY = 0;
  parameter->OE_CEXT = 0;
  parameter->WR_ODT_EXT = 0;
  parameter->RD_ODT_EXT = 3;
  
  //temp = 1000000/(freq*30*4);
  temp = 10000000/(freq*347*4);
  temp = (temp>0x2b)?(0x2b):temp;
  parameter->CLK_DLY = 20+temp;
  parameter->WR_DQS_DLY = ((parameter->CLK_DLY)<<24)| ((parameter->CLK_DLY)<<16)|((parameter->CLK_DLY)<<8)|((parameter->CLK_DLY));
  
  temp = 9+temp;
  parameter->RD_DQS_DLY = (temp<<24)| (temp<<16)|(temp<<8)|(temp);;
/***************TIME1****************/  
  parameter->tMRD_cmd =0xc;
  parameter->tRP = parameter->tRCD;
  
  temp = cycle_calc(7500,freq);
  parameter->tWTR = (temp>4) ? temp : 4;
  parameter->tRTW = (temp>4) ? temp : 4;
  //parameter->tRTW = 4;

  parameter->tMRD = 4;
  //if((parameter->tRCD)>4)
  if((parameter->tRCD)>4)
  	{
  		parameter->tWR = parameter->tRCD;
  	}
  else
  	{
  		parameter->tWR = 4;
  		//parameter->tWR = parameter->tRCD+2;
  	}
  parameter->tWR_ddr = parameter->tRCD;
    
  temp = cycle_calc(10000,freq);
  parameter->tRRD = (temp>4) ? temp : 4;
  	
/***************TIME2****************/
  if(freq<=533)
  	{
  		temp = cycle_calc(7500,freq);
  		parameter->tXP = (temp>3) ? temp:3;  			  		
  		parameter->tRAS = cycle_calc(37500,freq);
  		parameter->tRC = cycle_calc(52500,freq);
  	}
  else  
  	{  			
  		temp = cycle_calc(6000,freq);
  		parameter->tXP = (temp>3) ? temp:3; 
  		parameter->tRAS = cycle_calc(36000,freq);
  		parameter->tRC = cycle_calc(51000,freq);
  	}
  if(freq<=400)	
  	{
  		temp = cycle_calc(7500,freq);
  		parameter->tCKE = (temp>3) ? temp:3;
  	}
  else
  	{
  		temp = cycle_calc(5625,freq);
  		parameter->tCKE = (temp>3) ? temp:3;
  	}
/***************TIME3****************/  
  temp = cycle_calc(10000,freq);
  parameter->tCKSR = (temp>5) ? temp : 5;
  temp = cycle_calc(7500,freq);
  parameter->tRTP = (temp>4) ? temp : 4;
  parameter->PRECNT  = 0xf;
  parameter->AREFCNT  = 0x8;
  parameter->tREFI = 78*freq/10-0x10;//
  
/***************TIME3****************/ 
	
	parameter->tRFC = pdram_timing->tRFC;
	parameter->tXS = pdram_timing->tXS;
  	
  	  	
  if(freq<=400)
  	{
  		parameter->t_ibias_sta = 7;
  		parameter->t_iodt_sta = 6;
  	}
  else
  	{
  		parameter->t_ibias_sta = 7;
  		parameter->t_iodt_sta = 6;
  	}
  if(freq<=533)
  	{
  		parameter->tFAW = cycle_calc(50000,freq);
  	}
  else
  	{
  		parameter->tFAW = cycle_calc(45000,freq);
  	}
  
  	
  switch(ddr_drv0)	
  	{
  		case 0:
  			{
  				parameter->DRV0 = 0x4466550f;
  				break;
  			}
  		case 1:
  			{
  				parameter->DRV0 = 0x4366550f;
  				break;
  			}
  		case 2:
  			{
  				parameter->DRV0 = 0x4477550f;
  				break;
  			}
  		case 3:
  			{
  				parameter->DRV0 = 0x5577550f;
  				break;
  			}
  		case 4:
  			{
  				parameter->DRV0 = 0x5566550f;
  				break;
  			}
  		case 5:
  			{
  				parameter->DRV0 = 0x3366550f;
  				break;
  			}
  		case 6:
  			{
  				parameter->DRV0 = 0x3355550f;
  				break;
  			}
  		case 7:
  			{
  				parameter->DRV0 = 0x3377550f;
  				break;
  			}
  		case 8:
  			{
  				parameter->DRV0 = 0x4377550f;
  				break;
  			}		
  	    case 9:
  			{
  				parameter->DRV0 = 0x3344550f;
  				break;
  			}	
  		case 10:
  			{
  				parameter->DRV0 = 0x3333550f;
  				break;
  			}	
  		case 11:
  			{
  				parameter->DRV0 = 0x3322550f;
  				break;
  			}
  		case 12:
  			{
  				parameter->DRV0 = 0x2233550f;
  				break;
  			}
  		default:
  			{
  				parameter->DRV0 = 0x4466550f;
  				break;
  			}
  	}
  	//parameter->DRV1 = 0x2a3;
//  	parameter->DRV1 = 0x0;
  	switch(ddr_drv1)	
  	{
  		case 0:
  			{
  				parameter->DRV1 = 0x2a3;
  				break;
  			}
  		case 1:
  			{
  				parameter->DRV1 = 0x0;
  				break;
  			}
  		default:
  			{
  				parameter->DRV1 = 0x2a3;
  				break;
  			}
  	}
  	
  	  	
	return parameter;	
}




int cap_timing_set(unsigned int freq,unsigned int single_dram_cap)
{
	unsigned int tmp_reg_value;//capacity,temp;
	
	struct single_dram_cap_timing dram_timing;
	struct single_dram_cap_timing *pdram_timing;
	pdram_timing = &dram_timing;
	
	
//	tmp_reg_value = Read_reg(DCU_FEA);
//	tmp_reg_value = tmp_reg_value & 0xf0;
//	capacity = tmp_reg_value>>4;
//	
//	
//	switch(capacity)
//	{
//		case 4:
//			{
//				//tmp_reg_value = (width?1:2);//single_dram_cap = 1Gbit;
//				tmp_reg_value = 4;//capacity = 4Gbit;
//				break;
//			}
//		case 5:
//			{
//				//tmp_reg_value = (width?2:4);//single_dram_cap = 2Gbit;
//				tmp_reg_value = 8;//capacity = 8Gbit;
//				break;	
//			}
//		case 6:
//			{
//				//tmp_reg_value = 4;//single_dram_cap = 4Gbit;
//				tmp_reg_value = 16;//capacity = 16Gbit;
//				break;
//			}
//		default:
//			{
//				//tmp_reg_value = (width?2:4);//single_dram_cap = 2Gbit;
//				tmp_reg_value = 8;//capacity = 8Gbit;
//				break;
//			}
//	}	
//	tmp_reg_value = tmp_reg_value/dram_num;//calculate the single_dram_cap
	  		
    _single_dram_timing_genarate_ddr3(freq,pdram_timing,single_dram_cap);
	
	tmp_reg_value = Read_reg(DCU_TIME4);
	tmp_reg_value = tmp_reg_value & 0xffff;
	tmp_reg_value = tmp_reg_value |((pdram_timing->tRFC)<<16);
	Write_reg(DCU_TIME4,tmp_reg_value);	
  		
  tmp_reg_value = Read_reg(DCU_SR_CFG);
	tmp_reg_value = tmp_reg_value & 0xffff;
	tmp_reg_value = tmp_reg_value |((pdram_timing->tXS)<<16);
	Write_reg(DCU_SR_CFG,tmp_reg_value);	
	
	return 0;
 
}



int preamble_detect_func(unsigned int t_ie_cdly,unsigned int t_cyc_dly)
//int preamble_detect_func()
{
	unsigned int tmp_reg_value;
	unsigned char tmp_cyc_ie;
	
	
	tmp_reg_value = Read_reg(DCU_FEA);
	tmp_reg_value = tmp_reg_value&0xffefffff; // turn off the ODT_IN
	Write_reg(DCU_FEA,tmp_reg_value);	
	
	tmp_reg_value = Read_reg(DCU_DQS_EN_DLY);
	tmp_reg_value = tmp_reg_value&0x7FFFFFFF;
	Write_reg(DCU_DQS_EN_DLY,tmp_reg_value);

	
	
	tmp_cyc_ie = t_ie_cdly|(t_cyc_dly<<4);
	

	tmp_reg_value = Read_reg(DCU_PREAMBLE_GPIPE_CTRL);
	tmp_reg_value = tmp_reg_value&0xffffff00;
	tmp_reg_value = tmp_reg_value|(tmp_cyc_ie&0xff);
	Write_reg(DCU_PREAMBLE_GPIPE_CTRL,tmp_reg_value);
	
	adfu_printf("DCU_PREAMBLE_GPIPE_CTRL = %8x \n",Read_reg(DCU_PREAMBLE_GPIPE_CTRL));
	
	dcu_cmd_apply(CMD_PREAMBLE_DETECT);     
	
	tmp_reg_value = Read_reg(DCU_PREAMBLE_GPIPE_CTRL);
	tmp_reg_value = tmp_reg_value&0x00000100;
	if(tmp_reg_value == 0x00000100)
		{
				return 0;
		}
	else
		{
			return -1;
		}
	return 0;
 
}
