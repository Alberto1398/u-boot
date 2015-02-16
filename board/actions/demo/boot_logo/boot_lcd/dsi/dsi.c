/***************************************************************************
 *                              GLBASE
 *                            Module: lcd  driver for mbrac
 *                 Copyright(c) 2011-2015 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       wanghui     2013-11-23 9:00     1.0             build this file
 ***************************************************************************/
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "../boot_lcd_cfg.h"
#include "../display.h"
#include "../act_lcdc_config.h" 
#include "mipi_dsi.h"


#define OutLaneNum      4   //1--1lane;......4--4lane
#define TR_Mode     0   //0--Command mode; 1--Video Mode
#define VCNum       0          
//#define HS_CLOCK   276        //276MHz   too high, wrong  
//#define HS_CLOCK   256        //256MHz     
//#define HS_CLOCK   216        //216MHz     2lane no response
#define HS_CLOCK        180
//#define HS_CLOCK      168
//#define HS_CLOCK      156  
//#define HS_CLOCK      144  
//#define HS_CLOCK      120        //120MHz   
//#define HS_CLOCK      88        //88MHz   
#define PHY_CLOCK   (HS_CLOCK/4)  
#define HS_Clk_SP       (HS_CLOCK/6)    //gl6050 hs clock setting

#if OutLaneNum==1
#define PROCLKDIV   0x30  
#define DLNUM       0x0
#define Lane_Enable 0x1800
#endif

#if OutLaneNum==2
#define PROCLKDIV   0x10  
#define DLNUM       0x100
#define Lane_Enable 0x3800
#endif

#if OutLaneNum==3
#define PROCLKDIV   0x20  
#define DLNUM       0x200
#define Lane_Enable 0x7800
#endif

#if OutLaneNum==4
#define PROCLKDIV   0x00  
#define DLNUM       0x300
#define Lane_Enable 0xf800
#endif


unsigned int PWM_NUM;
unsigned int PWM_FREQ;
unsigned int PWM_ACTIVE;
unsigned int PWM_DEFAULT_BRIGHTNESS;

struct gpio_pre_cfg* gpio_pwm;
struct gpio_pre_cfg* gpio_lcd_vcc;
struct gpio_pre_cfg* gpio_led_en;
struct gpio_pre_cfg* gpio_lcd_reset;




unsigned int dsi_div;

unsigned int dsi_ctrl;
unsigned int dsi_size;
unsigned int dsi_ht0;
unsigned int dsi_ht1;
unsigned int dsi_vt0;
unsigned int dsi_vt1;
unsigned int dsi_pack_cfg;
unsigned int dsi_pack_header;
unsigned int dsi_vd_cfg;
unsigned int dsi_color;
unsigned int dsi_phyctrl;

unsigned int phy_t0;
unsigned int phy_t1;
unsigned int phy_t2;

unsigned int dsi_init;


static int lcd_config_init(void);

static int get_dsi_clk_divisor(unsigned int source_rate, unsigned int target_rate, unsigned int *dsi_div)
{
	return 0;
}

static int dsi_phy_setting(void)
{    
	unsigned int temp;
	
	act_writel(phy_t0,DSI_PHY_T0);
	act_writel(phy_t1,DSI_PHY_T1);
	act_writel(phy_t2,DSI_PHY_T2);


	temp = (TR_Mode<<12)|(VCNum<<10)|(DLNUM);
	act_writel(temp ,DSI_CTRL);//enable cal
	//act_writel(act_readl(DSI_PHY_CTRL) | (0x3<<24) | (0x7f) | (0<<16)| Lane_Enable,DSI_PHY_CTRL);//enable cal
	act_writel(act_readl(DSI_PHY_CTRL)|dsi_phyctrl,DSI_PHY_CTRL);//enable cal
	mdelay(10);
	
	act_writel(act_readl(DSI_PHY_CTRL) & 0xfdffffff,DSI_PHY_CTRL);//disable phy cal
	
	mdelay(10);

	act_writel(0x03,DSI_TR_STA);//Clear LP1 & LP0 Error
	
	return 0;

}

void send_short_packet(unsigned int data_type, unsigned int sp_data, unsigned int trans_mode)
{
  
	
	act_writel(act_readl(DSI_CTRL) & 0xffffefff,DSI_CTRL);   //set to command mode and no continue clock
	
	act_writel(sp_data,DSI_PACK_HEADER);
	
	act_writel((data_type << 8) | (trans_mode << 14),DSI_PACK_CFG);
	
	mdelay(1);         //Require for sync.
			
	act_writel(act_readl(DSI_PACK_CFG) + 1 ,DSI_PACK_CFG );  //start DMA0           
	
	//Transmit Complete
	while(!(act_readl(DSI_TR_STA) &(1<<19)));		
	
	act_writel((1<<19),DSI_TR_STA);    //Clear TCIP
}

void send_normal_long_packet(unsigned int data_type, unsigned int word_cnt, unsigned int * send_data, unsigned int trans_mode)
{
	unsigned long *src_addr;
	
	act_writel((act_readl(DSI_CTRL) & 0xffffefbf) , DSI_CTRL); //set to command mode and no continue clock
	
	//src_addr = (unsigned long *)(0x000a0000 + 0x00000000);
	src_addr = memalign(ARCH_DMA_MINALIGN, word_cnt*sizeof(unsigned long));
	if(src_addr)
	{
	    printf("%s: no mem\n", __FUNCTION__);
	    hang();
	}

	memcpy(src_addr, send_data, word_cnt);
	flush_dcache_range((ulong)src_addr, (ulong)src_addr+word_cnt);
	
	act_writel(word_cnt,DSI_PACK_HEADER);
	
	act_writel((data_type << 8) | 0x40000 | (trans_mode << 14),DSI_PACK_CFG);
	
	act_writel(0x00510012,BDMA0_MODE);
	
	act_writel(0x1,DMA_IRQPD); //Clear DMA0 IRQ Pending
	
	act_writel(0x0,DMA_IRQEN); //DMA0 in IRQ enable.
	
	act_writel(0, BDMA0_CACHE);

	act_writel((unsigned int)src_addr, BDMA0_SRC); //DMA0 in IRQ enable.
	
	act_writel(DSI_FIFO_ODAT,BDMA0_DST); //DMA0_DST info  
	
	act_writel(word_cnt,BDMA0_CNT); //DMA0_CNT info  
	
	act_writel(0x01,BDMA0_CMD);  //start DMA0   
	
	mdelay(10);
	
	act_writel(act_readl(DSI_PACK_CFG) + 1 ,DSI_PACK_CFG );  //start DMA0   
	
	while(act_readl(BDMA0_CMD) &0x1);
	 
	//Transmit Complete
	//Transmit Complete
	while(!(act_readl(DSI_TR_STA)&(1<<19)));
	
	act_writel((1<<19),DSI_TR_STA);    //Clear TCIP
	
	act_writel(0xfff,DSI_TR_STA);    //Clear TCIP
	
	free(src_addr);
	return;
}

static void set_dsi_clk(void)
{ 
	dsi_div = set_dsiclk();		
	act_xmlp_get_config("lcd.cmu_dsiclk", (char*)(&dsi_div), sizeof(int));
	act_writel(dsi_div, CMU_DSICLK);  /*set the dsi clk*/
}


void lcd_init(void)
{
	lcd_config_init();
	
	direct_set_lcd_gpio_active(gpio_pwm, (gpio_pwm->active_level ^ 0x01));          

	/*open lcd vcc */       
	direct_set_lcd_gpio_active(gpio_lcd_vcc, 1) ; 

		
	if(0x55!=gpio_lcd_reset->iogroup)
	{
		direct_set_lcd_gpio_active(gpio_lcd_reset, 1);	
		mdelay(10);		
		direct_set_lcd_gpio_active(gpio_lcd_reset, 0);		
		mdelay(10);	
		direct_set_lcd_gpio_active(gpio_lcd_reset, 1);	
	}
	
	//act_writel(act_readl(GPIO_ADAT) | (0x01 << 15) , GPIO_ADAT); 
	
	// set mfp contol
	act_writel(act_readl(MFP_CTL1) & 0xfffffeff, MFP_CTL1);
	/*reset dsi control  block */  
	act_writel(act_readl(CMU_DEVRST0) & (~(0x1 << 10)), CMU_DEVRST0);
	mdelay(10);
	act_writel(act_readl(CMU_DEVRST0) | (0x1 << 10), CMU_DEVRST0);  
	
	/*enable dsi clock */
	act_writel(act_readl(CMU_DEVCLKEN0) | (0x1<<12), CMU_DEVCLKEN0);   	
	set_dsi_clk();
	
	dsi_phy_setting();
	//wait D-PHY initialize complete
	
	act_writel(act_readl(DSI_CTRL) | 0x40 ,DSI_CTRL);
	mdelay(4);
	while(!(act_readl(DSI_LANE_STA) & 0x1020)){ 
	}

	if(dsi_init)
	{
		send_cmd();
	}


}

static int lcd_config_init(void)
{
	int lcd_clk, display_clk;
	
	PWM_NUM = get_lcd_config(BACKLIGHT_PWM, BACKLIGHT_PWM_NUMBER_VALUE, 0);
	PWM_FREQ = get_lcd_config(BACKLIGHT_PWM, BACKLIGHT_PWM_FREQ_VALUE, 200);
	PWM_ACTIVE = get_lcd_config(BACKLIGHT_PWM, BACKLIGHT_PWM_ACTIVE_VALUE, 1);
	PWM_DEFAULT_BRIGHTNESS = get_lcd_config(BACKLIGHT_BRIGHTNESS, BACKLIGHT_DEFAULT_BRIGHTNESS_VALUE, 512);
	
	gpio_pwm = get_lcd_gpio_cfg(GPIO_LCD_PWM);
	gpio_lcd_vcc = get_lcd_gpio_cfg(GPIO_LCD_VCC);
	gpio_led_en = get_lcd_gpio_cfg(GPIO_LCD_LED_EN);
	gpio_lcd_reset = get_lcd_gpio_cfg(GPIO_LCD_RESET);


	lcd_clk = get_lcd_config(DCLK, CLOCK_RATE_VALUE, 300000);
	display_clk = (act_readl(CMU_DISPLAYPLL) & 0x7F) * 6000;
	get_dsi_clk_divisor(display_clk, lcd_clk, &dsi_div );

	dsi_ctrl  =  get_lcd_config(p_DSI_CTRL, 0, 4944);
	dsi_size  =  get_lcd_config(p_DSI_SIZE, 0, 67043328);
	dsi_ht0  =  get_lcd_config(p_DSI_HT0, 0, 245185);
	dsi_ht1  =  get_lcd_config(p_DSI_HT1, 0, 2499);
	dsi_vt0  =  get_lcd_config(p_DSI_VT0, 0, 32580658);
	dsi_vt1  =  get_lcd_config(p_DSI_VT1, 0, 33);
	dsi_pack_cfg  =  get_lcd_config(p_DSI_PCFG, 0, 0);
	dsi_pack_header  =  get_lcd_config(p_DSI_PHEAD, 0, 2304);
	dsi_vd_cfg  =  get_lcd_config(p_DSI_VCFG, 0, 779);
	dsi_color = get_lcd_config(p_DSI_COLOR, 0, 0xff);
	dsi_phyctrl = get_lcd_config(p_DSI_PHYCTRL, 0, 0x0300f87f);
	
	phy_t0  =  get_lcd_config(p_DSI_PHYT0, 0, 6755);
	phy_t1  =  get_lcd_config(p_DSI_PHYT1, 0, 6931);
	phy_t2  =  get_lcd_config(p_DSI_PHYT2, 0, 12040);
	
	dsi_init = get_lcd_config(p_DSI_INIT, 0, 1);

	return 0;
}
 
#define CMU_HOSC_FREQ                   25165824
#define CMU_LOSC_FREQ                   32768 
#define MAX_PWM_CLK_DIV                 1024

void back_light_open(void)
{
	int pwm_duty = PWM_DEFAULT_BRIGHTNESS;
	int pwm_clk_div = MAX_PWM_CLK_DIV;
	act_writel(act_readl(CMU_DEVCLKEN1) | (0x1 << (23 + PWM_NUM)) ,CMU_DEVCLKEN1);
	if(PWM_FREQ < CMU_LOSC_FREQ / MAX_PWM_CLK_DIV) {
		act_writel((CMU_LOSC_FREQ /(PWM_FREQ * MAX_PWM_CLK_DIV) - 1), CMU_PWM0CLK + 4*PWM_NUM);
	} else if(PWM_FREQ <= CMU_HOSC_FREQ / MAX_PWM_CLK_DIV) {
		act_writel(0x1000 | (CMU_HOSC_FREQ /(PWM_FREQ * MAX_PWM_CLK_DIV) - 1), CMU_PWM0CLK + 4*PWM_NUM);
	} else if(PWM_FREQ > CMU_HOSC_FREQ / MAX_PWM_CLK_DIV){
	   pwm_clk_div = CMU_HOSC_FREQ / PWM_FREQ ;
	   act_writel(0x1000 | (CMU_HOSC_FREQ /(PWM_FREQ * pwm_clk_div) - 1), CMU_PWM0CLK + 4*PWM_NUM);
	}       
	
	pwm_duty = pwm_duty * pwm_clk_div / 1024;
	
	if(!PWM_ACTIVE){
       pwm_duty = pwm_clk_div - pwm_duty;
	}
	act_writel(0x100 | (pwm_duty << 19) | ((pwm_clk_div - 1) << 9) , PWM_CTL0 + 4*PWM_NUM);
	mdelay(200);
	direct_set_lcd_gpio_active(gpio_pwm, -1);
	act_writel(act_readl(GPIO_DDAT) | (0x01 << 13) , GPIO_DDAT);
	direct_set_lcd_gpio_active(gpio_led_en, 1) ;
	
}

void back_light_close(void){
	if(PWM_ACTIVE){
	    act_writel(0x100, PWM_CTL0 + 4*PWM_NUM);
	} else {
		act_writel(0x13f , PWM_CTL0 + 4*PWM_NUM);
	}
	//mdelay(10);
	act_writel(act_readl(CMU_DEVCLKEN1) & (~(0x1 << (23 + PWM_NUM))), CMU_DEVCLKEN1);
}

void lcd_open(void){  


	act_writel(dsi_ctrl,DSI_CTRL);  
	act_writel(dsi_size,DSI_SIZE);
	act_writel(dsi_pack_cfg,DSI_PACK_CFG);  
	act_writel(dsi_pack_header,DSI_PACK_HEADER);      
	act_writel(dsi_ht0,DSI_RGBHT0);
	act_writel(dsi_ht1,DSI_RGBHT1);
	act_writel(dsi_vt0,DSI_RGBVT0);
	act_writel(dsi_vt1,DSI_RGBVT1);
	act_writel(dsi_vd_cfg,DSI_VIDEO_CFG); 

	/*act_writel(act_readl(DSI_INT_EN)|(0x1<<31), DSI_INT_EN);*/
	//while(!(act_readl(DSI_TR_STA) & 0x80000000));    
		act_writel(0x80000000,DSI_TR_STA);        //Clear TCIP*/
}

void lcd_close(void)
{
	back_light_close();             
	
	act_writel(0x0, CMU_DSICLK);    /*pull down dclk*/              
	
	act_writel(act_readl(CMU_DEVCLKEN0) & ~(0x1<<12), CMU_DEVCLKEN0 );
	
	direct_set_lcd_gpio_active(gpio_lcd_vcc, -1) ;
	direct_set_lcd_gpio_active(gpio_led_en, -1) ;
}


int get_lcd_type(void)
{
	return LCD_TYPE_DSI;
}


int get_lcd_version(void)
{
	//adfu_printf(" get_lcd_version ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!\n");
	return 0x01cd;
}
