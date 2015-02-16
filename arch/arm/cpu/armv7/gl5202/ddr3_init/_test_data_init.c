/*
=================================================================================     
*                        
* FileName: test_data_init.c   parameter: start_addr,data_len>=0x80
* Return value : r0(init complete or not)
* Author  : zhongxu  
* Version : v1.0
* Chip    : GL5202
* Date    : 2012-09-20
* Description:  
* History :
*   v1.0    create this file    2012-09-07
=================================================================================  
*/

#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include "gl5202_ddr.h"


int _test_data_init(unsigned int start_addr,unsigned int data_len)
{
    static const uint32_t s_init_tbl[128/4] = {
            0x5a5aa5a5,
            0xa5a55a5a,
            0xffff0000,
            0x0000ffff,
            0x12345678,
            0x87654321,
            0xdeadabcd,
            0xabcddead,
            0x00000000,
            0xffffffff,
            0x5555aaaa,
            0xaaaa5555,
            0x3333cccc,
            0xcccc3333,
            0x66669999,
            0x99996666,
            0x77778888,
            0x88887777,
            0x1111eeee,
            0xeeee1111,
            0x55555555,
            0xaaaaaaaa,
            0x66666666,
            0x99999999,
            0x33333333,
            0xcccccccc,
            0x77777777,
            0x88888888,
            0x11111111,
            0xeeeeeeee,
            0xa5a5a5a5,
            0x5a5a5a5a,
    };

/***********************************bdma0 transfer data init*****************************/	
    unsigned int i, tmp_addr, datainit_num = 0;
    unsigned int dma_mode = DMA_DTrg_SRAM+DMA_STrg_SRAM;
  
    tmp_addr = start_addr;
    for(i=0; i<(128/4); i++)
    {
        Write_reg(tmp_addr, s_init_tbl[i]);
        tmp_addr += 4;
    }
	
	datainit_num = datainit_num +0x80;
	
	if(start_addr>0x80000000)
		dma_mode = DMA_DTrg_SRAM+DMA_STrg_SRAM;
	else
		dma_mode = DMA_DTrg_DDR+DMA_STrg_DDR;
	
	while(datainit_num<data_len)
	{
		_GL5202_BDMA0_TRANSFER(start_addr,(start_addr+datainit_num),0x80,dma_mode);
		datainit_num = datainit_num +0x80;
	}
	
	return 0;	
}

