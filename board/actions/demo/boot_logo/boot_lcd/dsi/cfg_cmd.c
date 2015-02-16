#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "../boot_lcd_cfg.h"
#include "mipi_dsi.h"

//#define	CLK_436
//#define	CLK_654
#define	CLK_564
//#define	CLK_376

#define LONG_CMD_MODE 0x39
#define SHORT_CMD_MODE 0x05

#define POWER_MODE  0x01

void send_long_cmd(char *pcmd, int cnt)
{
	send_normal_long_packet(LONG_CMD_MODE,cnt,(unsigned int *)pcmd,POWER_MODE);
}

void send_short_cmd(int cmd)
{
	send_short_packet(SHORT_CMD_MODE,cmd,POWER_MODE);  
}

int set_dsiclk(void)
{
#ifdef CLK_436
	return CLK_436M;
#endif

#ifdef CLK_654
	return CLK_654M;
#endif

#ifdef CLK_564
	return CLK_564M;
#endif

#ifdef CLK_376
	return CLK_376M;
#endif

	return CLK_436M;
}

/*
长包格式
	pakg[0] cmd
	pakg[1] data1
	pakg[2] data2
	.       .
	.       .
	send_long_cmd(pakg, 4); 第一个参数是指针，第二个参数是传输个数
	
短包格式
	send_short_cmd(cmd);
	exp： send_short_cmd(0x11);
	
需要用到毫秒延时
	mdelay(100); //100毫秒的延时
*/
void send_cmd(void)
{
	char __maybe_unused pakg[100];
	
	pakg[0] = 0xbf; 
	pakg[1] = 0x93; 
	pakg[2] = 0x61; 
	pakg[3] = 0xf4; 
	//send_long_cmd(pakg, 4);
	send_short_cmd(0x11);
	mdelay(10);
	send_short_cmd(0x29);
	mdelay(10);
}
