/***************************************************************************
 *                              GLBASE
 *                            Module: lcd  driver for mbrac
 *                 Copyright(c) 2011-2015 Actions Semiconductor,
 *                            All Rights Reserved.
 *
 * History:
 *      <author>    <time>           <version >             <desc>
 *       wanghui     2011-11-23 9:00     1.0             build this file
 ***************************************************************************/
#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "../boot_lcd_cfg.h"
#include "../display.h"
#include "../act_lcdc_config.h"

unsigned int PWM_NUM;
unsigned int PWM_FREQ;
unsigned int PWM_ACTIVE;
unsigned int PWM_DEFAULT_BRIGHTNESS;

struct gpio_pre_cfg* gpio_pwm;
struct gpio_pre_cfg* gpio_lcd_vcc;
struct gpio_pre_cfg* gpio_led_en;

unsigned int LCD_WIDTH;
unsigned int LCD_HEIGHT;
unsigned int DATA_WIDTH;
unsigned int HFP_SEL;
unsigned int HBP_SEL;
unsigned int HSP_SEL;                                      
unsigned int VFP_SEL;
unsigned int VBP_SEL;
unsigned int VSP_SEL;
unsigned int LCD_CLK_DIV;
unsigned int LCD0_CLK_DIV;

unsigned int vsync_inversion;
unsigned int hsync_inversion;
unsigned int dclk_inversion;
unsigned int lde_inversion;

unsigned int lvds_mapping;
unsigned int lvds_mirror;
unsigned int lvds_odd_polarity;
unsigned int lvds_even_polarity;
unsigned int lvds_ctl_reg;
unsigned int lvds_alg_reg;

unsigned int lcd_type;


unsigned int ssd2828_init(int xres , int yres ,int hbp ,int hfp, int hsw, int vbp ,int vfp, int vsw)
{
	
	
  spi_init();	
	
	spi_direct_cmd(0x28);//display off
	mdelay(50);
  spi_direct_cmd(0x10);//sleep out cmd
	mdelay(50);
		
	spi_direct_write(0xB7,0x0300);//video mode off	
	spi_direct_write(0xB7,0x0304); //ULP Mode

	
	spi_direct_read(0xB0); // id
	spi_direct_write(0xB1,0x3240);////VSA=50, HAS=64
	spi_direct_write(0xB2,0x5078);////VBP=30+50, HBP=56+64
	spi_direct_write(0xB3,0x243C);//VFP=36, HFP=60
	spi_direct_write(0xB4,0x0300);//HACT=768
	spi_direct_write(0xB5,0x0400);//VACT=1024	
	spi_direct_write(0xB6,0xB);  //burst mode, 18bpp loosely packed
	spi_direct_write(0xDE,0x3);  // //no of lane=4
	spi_direct_write(0xD6,0x4);//RGB order and packet number in blanking period	
	
  spi_direct_write(0xBA,0x801C); //may modify according to requirement, 500Mbps to 560Mbps
	spi_direct_write(0xBB,0x08); 

	spi_direct_write(0xB9,0x0);//disable PLL
		
	mdelay(50);

}
unsigned int ssd2828_open(){
	spi_direct_write(0xB9,0x1);//enable PLL
	
	spi_direct_write(0xB7,0x0300);//video mode off
	
	spi_direct_write(0xc4,0x1);////enable BTA
	
	spi_direct_write(0xB7,0x0342);//enter LP mode
	
	spi_direct_write(0xB8,0x0000);//VC
	
	spi_direct_write(0xBc,0x0);//set packet size	

	mdelay(50);
	spi_direct_cmd(0x11);//sleep out cmd
	mdelay(10);
	spi_direct_cmd(0x29);//display on
	mdelay(10);
	spi_direct_write(0xB7,0x030b);//enter LP mode	
		
	act_writel(act_readl(GPIO_ADAT ) & (~(0x01 << 23)),GPIO_ADAT); 	
}
void lcd_init(void){
	int tmp = 0;
	int dvfslevel=0;
	get_dvfslevel(dvfslevel);
	lcd_config_init();
	direct_set_lcd_gpio_active(gpio_pwm, (gpio_pwm->active_level ^ 0x01));
	act_writel((act_readl(GPIO_AOUTEN) | (0x01 << 23)),GPIO_AOUTEN);
	act_writel(act_readl(GPIO_ADAT )  | (0x01 << 23),GPIO_ADAT);	
		
	direct_set_lcd_gpio_active(gpio_lcd_vcc, 0) ;
	
	// get lcd pin
	act_writel(act_readl(PAD_CTL) | 0x02,PAD_CTL);
	dvfslevel = ASOC_GET_IC(dvfslevel);
	if((dvfslevel == 0x7021) || (dvfslevel == 0x7023)) {
	    // 7021A ic
	    act_writel((act_readl(MFP_CTL0) & 0x00003fff) | 0x24924000,MFP_CTL0);
		act_writel((act_readl(MFP_CTL1) & 0x1F80003f ) | 0x00600000 | 0x60000000,MFP_CTL1);
	} else if((dvfslevel == 0x7029)) {
	    // 7029B ic
	    act_writel((act_readl(MFP_CTL0) & 0x03ffffff) | 0x54000000,MFP_CTL0);
		act_writel((act_readl(MFP_CTL1) & 0x1F80003f ) | 0x00600000 | 0x60000000 | 0x100,MFP_CTL1);
	} else {
	    // others use 7021A setting
		act_writel((act_readl(MFP_CTL0) & 0x00003fff) | 0x24924000,MFP_CTL0);
		act_writel((act_readl(MFP_CTL1) & 0x1F80003f ) | 0x00600000 | 0x60000000,MFP_CTL1);
	}
  
  act_writel((act_readl(PAD_DRV1) & (0xc3ffffff)) | (0x28000000), PAD_DRV1);
  act_writel((act_readl(PAD_DRV2) & (0x0e7fffff)) | (0xa1000000), PAD_DRV2);

	/*reset lcd control  block */	
	act_writel(act_readl(CMU_DEVRST0) & (~(0x1 << 8)),CMU_DEVRST0);
	//udelay(50);
	act_writel(act_readl(CMU_DEVRST0) | (0x1 << 8),CMU_DEVRST0);
		
	act_writel(act_readl(CMU_DEVCLKEN0) | (0x1 << 18) ,CMU_DEVCLKEN0);
	
	/*set rgb lcd clk ,both lcd 0 and lcd1 */	
	act_writel(act_readl(CMU_DEVCLKEN0) | (0x1 << 9) ,CMU_DEVCLKEN0);
	act_writel(act_readl(CMU_DEVCLKEN0) | (0x1 << 10) ,CMU_DEVCLKEN0);
	act_writel(act_readl(CMU_DEVCLKEN0) | (0x1 << 11) ,CMU_DEVCLKEN0);	

	mdelay(400);
	direct_set_lcd_gpio_active(gpio_lcd_vcc,1);
	
	ssd2828_init(LCD_WIDTH,LCD_HEIGHT,HBP_SEL,HFP_SEL,HSP_SEL,VBP_SEL,VFP_SEL,VSP_SEL); 
	
    mdelay(50);
}

int get_lcd_clk_divisor(int source_rate, int target_rate, int *lcd_div, int *lcd0_div) {
	int tmp = 0;
	if(source_rate == 0 || target_rate == 0) {
		return -1;
	}
	
	if(target_rate > source_rate) {
	    *lcd_div = 1;
	    *lcd0_div = 1;
	} else if(target_rate*7*12 < source_rate) {
        *lcd_div = 7;
	    *lcd0_div = 12;
	} else {
		int i, last_pos = 0, cur_pos;
		tmp = source_rate/target_rate;
		
		for(i = 1; i < 24; i++,last_pos=cur_pos) {
			cur_pos = i;
		    if(i > 12) {
			    cur_pos = (i - 11)*7;
		    }
		    if(tmp < cur_pos) {
		    	break;
		    }
		}

		if((source_rate*cur_pos + source_rate*last_pos) > 2*target_rate*cur_pos*last_pos) {
		    tmp = cur_pos;
		} else {
			tmp = last_pos;
		}
		*lcd_div = tmp > 12 ? 7 : 1;
	    *lcd0_div = tmp > 12 ? tmp/7 : tmp;	
	}
	return 0;
}

int lcd_config_init(void)
{
	int i, sum = 0, lcd_clk, display_clk;
	
	PWM_NUM = get_lcd_config(BACKLIGHT_PWM, BACKLIGHT_PWM_NUMBER_VALUE, 0);
    PWM_FREQ = get_lcd_config(BACKLIGHT_PWM, BACKLIGHT_PWM_FREQ_VALUE, 200);
    PWM_ACTIVE = get_lcd_config(BACKLIGHT_PWM, BACKLIGHT_PWM_ACTIVE_VALUE, 1);
	
    PWM_DEFAULT_BRIGHTNESS = get_lcd_config(BACKLIGHT_BRIGHTNESS, BACKLIGHT_DEFAULT_BRIGHTNESS_VALUE, 512);
	
    gpio_pwm = get_lcd_gpio_cfg(GPIO_LCD_PWM);
    gpio_lcd_vcc = get_lcd_gpio_cfg(GPIO_LCD_VCC);
    gpio_led_en = get_lcd_gpio_cfg(GPIO_LCD_LED_EN);
	
	LCD_WIDTH = get_lcd_config(RESOLUTION, PIXEL_WIDTH_VALUE, 800);
	LCD_HEIGHT = get_lcd_config(RESOLUTION, PIXEL_HEIGHT_VALUE, 1280);
	
	HSP_SEL = get_lcd_config(HSYNC_SP, HSYNC_PULSE_WIDTH_VALUE, 12);
	HFP_SEL = get_lcd_config(HSYNC_FP, HSYNC_FRONT_PORCH_VALUE, 20);
	HBP_SEL = get_lcd_config(HSYNC_BP, HSYNC_BACK_PORCH_VALUE, 40);
	
	VSP_SEL = get_lcd_config(VSYNC_SP, VSYNC_PULSE_WIDTH_VALUE, 2);
	VFP_SEL = get_lcd_config(VSYNC_FP, VSYNC_FRONT_PORCH_VALUE, 10);
	VBP_SEL = get_lcd_config(VSYNC_BP, VSYNC_BACK_PORCH_VALUE, 33);
	
	DATA_WIDTH = get_lcd_config(LCD_DATA_WIDTH, DATA_WIDTH_VALUE, 0);
	
	lcd_clk = get_lcd_config(DCLK, CLOCK_RATE_VALUE, 60000);
	display_clk = (act_readl(CMU_DISPLAYPLL) & 0x7F) * 6000;
	get_lcd_clk_divisor(display_clk, lcd_clk, &LCD_CLK_DIV, &LCD0_CLK_DIV);
	
	hsync_inversion= get_lcd_config(HSYNC_ACTIVE_LEVEL, HSYNC_ACTIVE_VALUE, 0);
	vsync_inversion= get_lcd_config(VSYNC_ACTIVE_LEVEL, VSYNC_ACTIVE_VALUE, 0);
	dclk_inversion= get_lcd_config(DCLK_ACTIVE_LEVEL, DCLK_ACTIVE_VALUE, 1);
	lde_inversion = get_lcd_config(DATA_EN_ACTIVE_LEVEL, DATA_EN_ACTIVE_VALUE, 0);
    
	return 0;
}
 
#define CMU_HOSC_FREQ                   25165824
#define CMU_LOSC_FREQ                   32768 
#define MAX_PWM_CLK_DIV                 1024
void back_light_open(void){
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
	int tmp;  
	 
   /*set clk*/
  	act_writel(((LCD_CLK_DIV/7) << 8) | (LCD0_CLK_DIV-1), CMU_LCDCLK);	/*pull down dclk*/
  
	/*set size*/
	tmp = act_readl(LCD_SIZE);
	tmp = tmp&(~(0x7ff<<16))&(~(0x7ff));
	tmp = tmp |((LCD_HEIGHT - 1)<<16)|(LCD_WIDTH - 1);
	act_writel(tmp, LCD_SIZE);

	/*set timeing0*/
	tmp = act_readl(LCD_TIM0);
	tmp =tmp &(~(0xf<<4));	
	tmp = tmp|(vsync_inversion << 7)
		|(hsync_inversion << 6)
		|(dclk_inversion << 5)
		|(lde_inversion << 4)
		|(0x1 << 13)
		|(0x8 << 8);
		
        act_writel(tmp, LCD_TIM0);

	/*set timeing1*/
	tmp = (HBP_SEL - 1)  | ((HFP_SEL - 1)<< 10) | ((HSP_SEL - 1) << 20);
	act_writel(tmp, LCD_TIM1);

	/*set timeing2*/
	tmp = (VBP_SEL - 1) | ((VFP_SEL - 1) << 10) | ((VSP_SEL - 1) << 20);
	act_writel(tmp, LCD_TIM2);

	act_writel(0x0066ff33, LCD_COLOR);

	/*set lcd ctl*/
	tmp = act_readl(LCD_CTL);

  tmp = tmp & (~(0x1<<31)) & (~(0x7<<16));
	
	tmp = tmp|(DATA_WIDTH << 16) | 0x3; //rgb if, data width, video from de,enable lcdc
	
	tmp = (tmp & (~(0x03 << 6))) | (0x2 << 6);
	
	act_writel(tmp, LCD_CTL);
		
	mdelay(10);	
	
  ssd2828_open();    
}

void lcd_close(void){
	int tmp;
	
	back_light_close();
		
	act_writel(0xc0, LCD_TIM0);		/*pull down vsync/hsync*/
	/******************************************************************/  
  
	tmp = act_readl(LCD_CTL);
	
	tmp &= (~0x01);
	
	act_writel(tmp, LCD_CTL);	
	
	act_writel(0x0, CMU_LCDCLK);	/*pull down dclk*/		
	
	act_writel( act_readl(CMU_DEVCLKEN0) & ~(0x1<<9), CMU_DEVCLKEN0 );
	
	direct_set_lcd_gpio_active(gpio_lcd_vcc, -1) ;
	direct_set_lcd_gpio_active(gpio_led_en, -1) ;
}

int get_lcd_type(void)
{
	return 0;
}


int get_lcd_version(void)
{
	return 0x01cd;
}
