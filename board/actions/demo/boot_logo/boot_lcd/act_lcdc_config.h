#ifndef __ACT_LCDC_CONFIG_H__
#define __ACT_LCDC_CONFIG_H__

#include <common.h>
#include <asm/arch/actions_arch_common.h>

#include "boot_lcd_cfg.h"


enum ENUM_CONFIG_TYPE {
	BACKLIGHT_PWM,
	BACKLIGHT_BRIGHTNESS,
	DCLK,
	LCD_DATA_WIDTH,
	RESOLUTION,
    PHYSICAL_AREA,
	HSYNC_SP,
	HSYNC_FP,
	HSYNC_BP,
	VSYNC_SP,
	VSYNC_FP,
	VSYNC_BP,
	HSYNC_ACTIVE_LEVEL,
	VSYNC_ACTIVE_LEVEL,
	DCLK_ACTIVE_LEVEL,
	DATA_EN_ACTIVE_LEVEL,
	LVDS_MAPPING,
	LVDS_MIRROR,
	LVDS_ECKPOLARL,
	LVDS_OCKPOLARL,	
	LVDS_SVOCML,	
	RB_SWAP,
	
	p_DSI_CTRL,
	p_DSI_SIZE,
	p_DSI_HT0,
	p_DSI_HT1,
	p_DSI_VT0,
	p_DSI_VT1,
	p_DSI_PCFG,
	p_DSI_PHEAD,
	p_DSI_VCFG,
	p_DSI_COLOR,
	p_DSI_PHYCTRL,
	p_DSI_PHYT0,
	p_DSI_PHYT1,
	p_DSI_PHYT2,
	p_DSI_INIT
};

enum ENUM_BACKLIGHT_PWM {
	BACKLIGHT_PWM_NUMBER_VALUE,
	BACKLIGHT_PWM_FREQ_VALUE,
	BACKLIGHT_PWM_ACTIVE_VALUE
};

enum ENUM_BACKLIGHT_BRIGHTNESS {
	BACKLIGHT_MIN_BRIGHTNESS_VALUE,
	BACKLIGHT_MAX_BRIGHTNESS_VALUE,
	BACKLIGHT_DEFAULT_BRIGHTNESS_VALUE
};

enum ENUM_DCLK {
	CLOCK_RATE_VALUE
};

enum ENUM_LCD_DATA_WIDTH {
	DATA_WIDTH_VALUE
};

enum ENUM_RESOLUTION {
	PIXEL_WIDTH_VALUE,
	PIXEL_HEIGHT_VALUE
};

enum ENUM_PHYSICAL_AREA {
	PHY_WIDTH_VALUE,
	PHY_HEIGHT_VALUE
};

enum ENUM_HSYNC_SP {
	HSYNC_PULSE_WIDTH_VALUE
};

enum ENUM_HSYNC_FP {
	HSYNC_FRONT_PORCH_VALUE
};

enum ENUM_HSYNC_BP {
	HSYNC_BACK_PORCH_VALUE
};

enum ENUM_VSYNC_SP {
	VSYNC_PULSE_WIDTH_VALUE
};

enum ENUM_VSYNC_FP {
	VSYNC_FRONT_PORCH_VALUE
};

enum ENUM_VSYNC_BP {
	VSYNC_BACK_PORCH_VALUE
};

enum ENUM_HSYNC_ACTIVE_LEVEL {
	HSYNC_ACTIVE_VALUE
};

enum ENUM_VSYNC_ACTIVE_LEVEL {
	VSYNC_ACTIVE_VALUE
};

enum ENUM_DCLK_ACTIVE_LEVEL {
	DCLK_ACTIVE_VALUE
};

enum ENUM_DATA_EN_ACTIVE_LEVEL {
	DATA_EN_ACTIVE_VALUE
};

enum ENUM_LVDS_MAPPING {
	LVDS_MAPPING_VALUE
};

enum ENUM_LVDS_MIRROR {
	LVDS_MIRROR_VALUE
};

enum ENUM_LVDS_ECKPOLARL {
	LVDS_ECKPOLARL_VALUE
};

enum ENUM_LVDS_OCKPOLARL {
	LVDS_OCKPOLARL_VALUE
};

enum ENUM_LVDS_SVOCML {
	LVDS_SVOCML_VALUE
};

enum ENUM_RB_SWAP{
	RB_SWAP_VAL
};	



#define LCD_MAX_INFO_NUM 37
#define EACH_INFO_MAX_LEN  5
static int s_config_matrix[LCD_MAX_INFO_NUM][EACH_INFO_MAX_LEN];
static const char * s_config_key_array[] = {
    "lcd.backlight_pwm",
    "lcd.backlight_brightness",
    "lcd.dclk",
    "lcd.data_width",
    "lcd.resolution",
	"lcd.physical_area",
	"lcd.hsync_sp",
    "lcd.hsync_fp",
    "lcd.hsync_bp",
    "lcd.vsync_sp",
    "lcd.vsync_fp",
	"lcd.vsync_bp",
	"lcd.hsync_active_level",
    "lcd.vsync_active_level",
	"lcd.dclk_active_level",
    "lcd.data_en_active_level",
    "lcd.lvds_mapping",
    "lcd.lvds_mirror",
    "lcd.lvds_eckpolarl",
	"lcd.lvds_ockpolarl",
	"lcd.lvds_svocml",

	"lcd.rb_swap",
	
	"lcd.dsi_ctrl",
	"lcd.dsi_size",
	"lcd.dsi_rgbht0",
	"lcd.dsi_rgbht1",
	"lcd.dsi_rgbvt0",
	"lcd.dsi_rgbvt1",
	"lcd.dsi_pack_cfg",
	"lcd.dsi_pack_header",
	"lcd.dsi_vedio_cfg",
	"lcd.dsi_color",
	"lcd.dsi_phyctrl",
	"lcd.dsi_phy_t0",
	"lcd.dsi_phy_t1",
	"lcd.dsi_phy_t2",
	"lcd.dsi_init"

};

static const char * s_config_key_array1[] = {
    "lcd1.backlight_pwm",
    "lcd1.backlight_brightness",
    "lcd1.dclk",
    "lcd1.data_width",
    "lcd1.resolution",
	"lcd1.physical_area",
	"lcd1.hsync_sp",
    "lcd1.hsync_fp",
    "lcd1.hsync_bp",
    "lcd1.vsync_sp",
    "lcd1.vsync_fp",
	"lcd1.vsync_bp",
	"lcd1.hsync_active_level",
    "lcd1.vsync_active_level",
	"lcd1.dclk_active_level",
    "lcd1.data_en_active_level",
    "lcd1.lvds_mapping",
    "lcd1.lvds_mirror",
    "lcd1.lvds_eckpolarl",
	"lcd1.lvds_ockpolarl",
	"lcd1.lvds_svocml",

	"lcd1.rb_swap",
	
	"lcd1.dsi_ctrl",
	"lcd1.dsi_size",
	"lcd1.dsi_rgbht0",
	"lcd1.dsi_rgbht1",
	"lcd1.dsi_rgbvt0",
	"lcd1.dsi_rgbvt1",
	"lcd1.dsi_pack_cfg",
	"lcd1.dsi_pack_header",
	"lcd1.dsi_vedio_cfg",
	"lcd.dsi_color",
	"lcd.dsi_phyctrl",
	"lcd1.dsi_phy_t0",
	"lcd1.dsi_phy_t1",
	"lcd1.dsi_phy_t2",
	"lcd1.dsi_init"

};

static int get_lcd_module_by_gpio(void) {
#ifdef CONFIG_ACTS_ATM7029_10_B_SUMSUNG_CHIMEI
	int ret ;
	int pull_val = act_readl(PAD_PULLCTL1) & (1<<28);
	int gpioc9_stat = act_readl(GPIO_COUTEN) & (1 << 9);
	int gpioc9_value = act_readl(GPIO_CDAT) & (1 << 9);
	act_writel((act_readl(PAD_PULLCTL1) |(1 << 28)), PAD_PULLCTL1);
	act_writel((act_readl(GPIO_CDAT) & (~(1 << 9))), GPIO_CDAT);
	act_writel((act_readl(GPIO_COUTEN) & (~(1 << 9))), GPIO_COUTEN);
	mdelay(2);
	act_writel((act_readl(GPIO_CINEN) |(1 << 9)), GPIO_CINEN);
	mdelay(2);
	ret = act_readl(GPIO_CDAT) & (1 << 9);
	if(pull_val) {
       		act_writel(act_readl(PAD_PULLCTL1) | (1<<28), PAD_PULLCTL1);
	} else {
		act_writel(act_readl(PAD_PULLCTL1) & (~(1<<28)), PAD_PULLCTL1);
	}
	if(gpioc9_stat) {
		act_writel(act_readl(GPIO_COUTEN) | (1<<9), GPIO_COUTEN);
	} else {
		act_writel(act_readl(GPIO_COUTEN) & (~(1<<9)), GPIO_COUTEN);
	}
	if(gpioc9_value) {
		act_writel(act_readl(GPIO_CDAT) | (1<<9), GPIO_CDAT);
	} else {
		act_writel(act_readl(GPIO_CDAT) & (~(1<<9)), GPIO_CDAT);
	}
	act_writel((act_readl(GPIO_CINEN) & (~(1 << 9))), GPIO_CINEN);
	//#ifndef CONFIG_ACTS_BUILD_IN_BOOT_LCD
	//	printk("================================= in get ret= %d\n",ret);
	//#endif
	return ret;
#else
	return 0;
#endif
}

static int s_gpio_status = -1;
static void config_array_init(void) {
	int i = 0, j = 0;
	//#ifndef CONFIG_ACTS_BUILD_IN_BOOT_LCD
	//	printk("================================= in config_array_init  gpiostat=%d\n",s_gpio_status);
	//#endif
	if(s_gpio_status < 0) {
		s_gpio_status = get_lcd_module_by_gpio();
	}
	for(i = 0; i < LCD_MAX_INFO_NUM; i++) {
		for(j = 0; j < EACH_INFO_MAX_LEN; j++) {
			s_config_matrix[i][j] = -1;
		}
		if(s_gpio_status > 0) {
			//samsung lcd
		    act_xmlp_get_config(s_config_key_array1[i], (char*)s_config_matrix[i], EACH_INFO_MAX_LEN*sizeof(int)) ;
		} else {
			//chimei lcd
		    act_xmlp_get_config(s_config_key_array[i], (char*)s_config_matrix[i], EACH_INFO_MAX_LEN*sizeof(int)) ;
		}
	}
}

inline static int get_lcd_config(int info_index, int offset, int def) {
	static int s_config_have_inited = 0;
	if(!s_config_have_inited) {
		config_array_init();
		s_config_have_inited = 1;
	}
	
	if(info_index < LCD_MAX_INFO_NUM && offset < EACH_INFO_MAX_LEN) {
		int value = s_config_matrix[info_index][offset];
		if(value!= -1) {
			return value;
		} 
	}
    return def;
}

/********************************************************************gpio  config*********************************************************************/
enum ENUM_GPIO_NAME {
	GPIO_LCD_PWM,
	GPIO_LCD_VCC,
	GPIO_LCD_LED_EN,
	GPIO_LCD_STANDBY,
	GPIO_LCD_RESET,
	GPIO_LCD_MAX_NUM
};

static struct gpio_pre_cfg s_lcd_gpio_array[5];
static char * s_gpio_key_array[] = {
     "lcd_pwm_ctl",
     "lcd_vcc_en",
     "lcd_led_en",
     "lcd_standby_en",
     "lcd_reset"
 };

static char * s_gpio_key_array1[] = {
     "lcd_pwm_ctl",
     "lcd_vcc_en",
     "lcd_led_en",
     "lcd_standby_en",
     "lcd_reset"
};

//用reserved[0]表示gpio是否已经调用gpio_request获取过，避免反复获取
static void gpio_array_init(void)
{
	int i = 0;
	//	#ifndef CONFIG_ACTS_BUILD_IN_BOOT_LCD
	//		printk("================================= in gpio_array_init  gpiostat=%d\n",s_gpio_status);
	//	#endif
	if(s_gpio_status < 0) {
	    s_gpio_status = get_lcd_module_by_gpio();
    }
    for(i = 0; i < GPIO_LCD_MAX_NUM; i++) {
        // s_lcd_gpio_array[i].valid = 0;
        s_lcd_gpio_array[i].reserved[0] = 0;
        if(s_gpio_status > 0) {
            //samsung lcd
            act_gpio_get_pre_cfg(s_gpio_key_array1[i], &s_lcd_gpio_array[i]);
    	} else {
    		//chimei lcd
    	    act_gpio_get_pre_cfg(s_gpio_key_array[i], &s_lcd_gpio_array[i]);
    	}
    }
}

inline static struct gpio_pre_cfg* get_lcd_gpio_cfg(enum ENUM_GPIO_NAME gpio) {
	static int s_gpio_have_inited = 0;
	if(!s_gpio_have_inited) {
		//memset((char *)&s_lcd_gpio_array[0], 0x55, 5*sizeof(struct gpio_pre_cfg));	
		s_lcd_gpio_array[0].iogroup=0x55;
		s_lcd_gpio_array[1].iogroup=0x55;
		s_lcd_gpio_array[2].iogroup=0x55;
		s_lcd_gpio_array[3].iogroup=0x55;
		s_lcd_gpio_array[4].iogroup=0x55;
		
		gpio_array_init();
		s_gpio_have_inited = 1;
	}
	
	if(gpio < GPIO_LCD_MAX_NUM) {
		return &s_lcd_gpio_array[gpio];
	}
	return 0;	
}

//value: 0, output low voltage; 1, output hight voltage; -1, stop output
static void set_gpio_value(int io_group, int pin_num, int put_way, int value) {
	int group_offset = io_group * 0x0c;
	int gpio_pos_reg,  gpio_neg_reg;
	
	if(put_way == 0) {
		gpio_pos_reg = GPIO_AOUTEN;
		gpio_neg_reg = GPIO_AINEN;
	} else {
		gpio_pos_reg = GPIO_AINEN;
		gpio_neg_reg = GPIO_AOUTEN;
	}
	
	if(value >= 0) {
	    act_writel(act_readl(gpio_neg_reg + group_offset) & ~(1 << pin_num) , gpio_neg_reg + group_offset);
        act_writel(act_readl(gpio_pos_reg + group_offset) | (1 << pin_num),  gpio_pos_reg + group_offset);
        if(value > 0) {
	        act_writel(act_readl(GPIO_ADAT + group_offset) | (1 << pin_num),  GPIO_ADAT + group_offset);
	    } else {
	    	act_writel(act_readl(GPIO_ADAT + group_offset) & (~(1 << pin_num)),  GPIO_ADAT + group_offset);
	    }
	  } else {
	  	act_writel(act_readl(GPIO_ADAT + group_offset) & (~(1 << pin_num)),  GPIO_ADAT + group_offset);
	  	act_writel(act_readl(gpio_pos_reg + group_offset) & (~(1 << pin_num)),  gpio_pos_reg + group_offset);
	  }
}

//通过直接设置相关寄存器的方式控制gpio, active: 0,设置gpio状态为unactive;  1,设置gpio状态为active
inline static void direct_set_lcd_gpio_active(struct gpio_pre_cfg* pcfg, int active) {
	int set_value;
	if(pcfg == 0) {
		return;
	}
	if(active >= 0) {
	    set_value = active ? pcfg->active_level : (pcfg->active_level ^ 0x1);
	} else {
		set_value = -1;
	}
	
/* 	if(!pcfg->valid) {
		return;
	} */
	set_gpio_value(pcfg->iogroup, pcfg->pin_num, pcfg->gpio_dir, set_value);
}

#ifndef CONFIG_ACTS_BUILD_IN_BOOT_LCD
//通过调用内核相关接口的方式控制gpio, active: 0,设置gpio状态为unactive;  1,设置gpio状态为active
inline static void set_lcd_gpio_active(struct gpio_pre_cfg* pcfg, int active) {
	int pin = ASOC_GPIO_PORT(pcfg->iogroup, pcfg->pin_num);
	int set_value = active ? pcfg->active_level : (pcfg->active_level ^ 0x1);
	if(pcfg == NULL) {
		return;
	}
	
/* 	if(!pcfg->valid) {
		return;
	} */
	
	if(pcfg->reserved[0] == 0) {
			gpio_request(pin, NULL);
			pcfg->reserved[0] = 1;
	}
	if(pcfg->gpio_dir == 0) {
		 gpio_direction_output(pin, set_value);
	} else if(pcfg->gpio_dir == 2) {
		 gpio_direction_input(pin);
	}	
}

inline static void free_lcd_gpio(struct gpio_pre_cfg* pcfg) {
	if(pcfg == NULL) {
		return;
	}
	if(pcfg->reserved[0]) {
		int pin = ASOC_GPIO_PORT(pcfg->iogroup, pcfg->pin_num);
        gpio_free(pin);
        pcfg->reserved[0] = 0;
      }
}

extern int hibernate_reg_setmap(unsigned int reg, unsigned int bitmap);
inline static void quickboot_not_control_gpio(struct gpio_pre_cfg* pcfg) {
	hibernate_reg_setmap(pcfg->iogroup*12 + GPIO_ADAT, 1 << pcfg->pin_num);
	hibernate_reg_setmap(pcfg->iogroup*12 + GPIO_AOUTEN, 1 << pcfg->pin_num);
	hibernate_reg_setmap(pcfg->iogroup*12 + GPIO_AINEN, 1 << pcfg->pin_num);
}

#endif

#endif
