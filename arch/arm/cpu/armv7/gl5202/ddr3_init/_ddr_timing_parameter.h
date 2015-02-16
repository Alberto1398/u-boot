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

struct timing_parameter
{
	unsigned int CL;
	unsigned int CWL;
	unsigned int IE_CDLY;
	unsigned int CYC_DLY;
	unsigned int IE_CEXT;
	unsigned int OE_CDLY;
	unsigned int OE_CEXT;
	unsigned int RD_ODT_EXT;
	unsigned int WR_ODT_EXT;
	/************delay chain set***********/
	unsigned int CLK_DLY;
	unsigned int WR_DQS_DLY;
	unsigned int RD_DQS_DLY;
	/***************TIME1****************/
	unsigned int tMRD_cmd;
	unsigned int tRP;
	unsigned int tRTW;
	unsigned int tWTR;
	unsigned int tMRD;
	unsigned int tWR;
	unsigned int tWR_ddr;
	unsigned int tRRD;
	unsigned int tRCD;
	/***************TIME2****************/
	unsigned int tXP;
	unsigned int tCKE;
	unsigned int tRAS;
	unsigned int tRC;
	/***************TIME3****************/
	unsigned int tCKSR;
	unsigned int tRTP;
	unsigned int PRECNT;
	unsigned int AREFCNT;
	unsigned int tREFI;
	/***************TIME4****************/
	unsigned int tRFC;
	unsigned int t_ibias_sta;
	unsigned int t_iodt_sta;
	unsigned int tFAW;
	/***************drive****************/
	unsigned int DRV0;
	unsigned int DRV1;
	/*************** SR ****************/
	unsigned int tXS;
};



struct single_dram_cap_timing
{	
	/***************TIME4****************/
	unsigned int tRFC;
	/*************** SR ****************/
	unsigned int tXS;
};

extern void udelay(unsigned long usec);

extern struct timing_parameter *_timing_parameter_genarate_ddr3(unsigned int freq,struct timing_parameter *parameter,unsigned int single_dram_cap,unsigned int ddr_drv0,unsigned int ddr_drv1);
extern int preamble_detect_func(unsigned int t_ie_cdly,unsigned int t_cyc_dly);
extern int cap_timing_set(unsigned int freq,unsigned int single_dram_cap);

#include <asm/arch/afinfo.h>

extern int _GL5202_DelayChain_Set(ddr_param_t const *cfg);
