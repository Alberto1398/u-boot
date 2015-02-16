#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/dvfs_level.h>

#include "act_lcdc_config.h"
#include "display.h"

/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;


#define GRAPHIC_LAYER               0
#define VIDEO1_LAYER                1
#define VIDEO2_LAYER                3
#define VIDEO3_LAYER                5

#define PATHx_CTL_SELB(x)          (((x) & 0x7) << 20)
#define PATHx_CTL_SELA(x)          (((x) & 0x7) << 16)

#define DISP_MODE_LCD        1
#define DISP_MODE_HDMI       2
#define DISP_MODE_HDMI_LCD   3

#define NEED_XFLIP    1
#define NEED_YFLIP    2

#define VIDEO_CFG_XFLIP   (1 << 21)
#define VIDEO_CFG_YFLIP   (1 << 20)
struct display_dev_info
{
  int de_clk;
  int display_mode; 

  int lcd_xres;
  int lcd_yres;
  int lcd_dither;
  int lcd_rotate;
  
  int hdmi_mode;
  int hdmi_xres;
  int hdmi_yres;
      
};
static struct display_dev_info disp_info;

static int de_getconfig(struct display_dev_info * disp_info){
 	int temp[15];
	
  
  disp_info->display_mode = DISP_MODE_HDMI_LCD;
  
  temp[0] = temp[1] = temp[2] = temp[3] = -1;     
  act_xmlp_get_config("boot_disp_cfg.disp_cfg",(char*)&temp[0], 1 * sizeof(int));
  
  if(temp[0] == -1){
  	disp_info->display_mode = DISP_MODE_HDMI_LCD; 
  }else{
  	disp_info->display_mode = temp[0];
  }
         
  temp[0] = temp[1] = temp[2] = temp[3] = -1;
  act_xmlp_get_config("boot_disp_cfg.hdmi_cfg",(char*)&temp[0], 4 * sizeof(int));
  
  if(temp[0] == -1){
  	  disp_info->hdmi_mode = 19;
  		disp_info->hdmi_xres = 1280;
  		disp_info->hdmi_yres = 720; 
  		disp_info->de_clk = 330;
  }else{
  	disp_info->hdmi_mode = temp[0];
  	disp_info->hdmi_xres = temp[1];
  	disp_info->hdmi_yres = temp[2];
  	disp_info->de_clk = temp[3]; 	 
 	}
 	temp[0] = temp[1] = temp[2] = temp[3] = -1;
 	act_xmlp_get_config("lcd.resolution",(char*)&temp[0], 2 * sizeof(int));
  if(temp[0] == -1){
  	    disp_info->lcd_xres = 1280;
 		disp_info->lcd_yres = 800;
  		disp_info->lcd_dither = 0x007020;
  }else{
	  disp_info->lcd_xres = temp[0];
	  disp_info->lcd_yres = temp[1];
	  if(temp[2] == 1){
	  	disp_info->lcd_dither = 0x007020;
	  }else if(temp[2] == 2){
	  	disp_info->lcd_dither = 0x002020;
	  }else{
	  	disp_info->lcd_dither = 0;
	  }	
	}	
	
	temp[0] = -1;
	act_xmlp_get_config("lcd.rotate",(char*)&temp[0], 1 * sizeof(int));
  disp_info->lcd_rotate = 0;
  if(temp[0] == -1){
  	 disp_info->lcd_rotate = 0;
  }else{
  	if((temp[0] & NEED_XFLIP) != 0){ //xflip
  		disp_info->lcd_rotate |= VIDEO_CFG_XFLIP;
  	}
  	if((temp[0] & NEED_YFLIP) != 0){ //yflip
  		disp_info->lcd_rotate |= VIDEO_CFG_YFLIP;
  	}
  }

  return 0;
}

static void de_init(void){
  int de_div, display_clk;

  switch (ASOC_GET_IC(gd->arch.dvfs_level))
  {
    case 0x7021:
    case 0x7023:
        act_writel(0x01, SHARESRAM_CTL); // SRAM back to DE.
      break;
    case 0x7029:
    default:
        break;
  }     
    
  // get config info from xml
  de_getconfig(&disp_info);
  
  /*reset de control  block */
  act_writel(act_readl(CMU_DEVRST0) & (~(0x1 << 7)), CMU_DEVRST0);
  act_writel(act_readl(CMU_DEVRST0) | (0x1 << 7), CMU_DEVRST0);
  
  /*DE interface clock enable */
  act_writel(act_readl(CMU_DEVCLKEN0) | (1 << 8), CMU_DEVCLKEN0);   

  // init config path1  for de 
  act_writel(disp_info.lcd_dither, PATH2_CTL);
  act_writel(0x0, PATH2_BK);
  act_writel(0x0, PATH2_EN); 
  // init config path1  for de 
	act_writel(0x0, PATH1_CTL);	
  act_writel(0x0, PATH1_BK);
  act_writel(0x0, PATH1_EN); 
   
  // set DEV_CLK 
  display_clk = (act_readl(CMU_DEVPLL) & 0x7F) * 6;
  
  de_div = (display_clk + 125) / disp_info.de_clk - 1;
  
  act_writel(((de_div + 2) << 8) 
             | (de_div  << 4) 
             | de_div 
             | (1 << 12),CMU_DECLK);// used DEV_CLK ,360 /180 /180
  if(get_lcd_type()==LCD_TYPE_DSI)
  {
          act_writel(act_readl(CMU_DECLK) |(0x1<<13), CMU_DECLK);    

  }
  else
  {
          act_writel(act_readl(CMU_DECLK) & ~(0x1<<13), CMU_DECLK);    
  }

  
  /*set patch 1 config  */    
  act_writel((((disp_info.hdmi_yres - 1) << 16) | (disp_info.hdmi_xres - 1)) ,PATH1_SIZE);//PATH2_SIZE
  
  act_writel(0x00,PATH1_A_COOR);//PATH1_A_COOR(0,0)

    
  /*set patch 2 config  */    
  act_writel((((disp_info.lcd_yres - 1) << 16) | (disp_info.lcd_xres - 1)) ,PATH2_SIZE);//PATH2_SIZE
  
  act_writel(0x00,PATH2_A_COOR);//PATH1_A_COOR(0,0)
  
  
  /*we only used video1 so only config this layer*/

	act_writel(0x04080770,VIDEO1_CFG);//video disable
  act_writel(0x4000,VIDEO1_SCOEF0);
  act_writel(0xff073efc,VIDEO1_SCOEF1);
  act_writel(0xfe1038fa,VIDEO1_SCOEF2);
  act_writel(0xfc1b30f9,VIDEO1_SCOEF3);
  act_writel(0xfa2626fa,VIDEO1_SCOEF4);
  act_writel(0xf9301bfc,VIDEO1_SCOEF5);
  act_writel(0xfa3810fe,VIDEO1_SCOEF6);
  act_writel(0xfc3e07ff,VIDEO1_SCOEF7);
  act_writel(0x20002000, VIDEO1_SR);
  act_writel(((disp_info.hdmi_yres -1) << 16) | (disp_info.hdmi_xres  - 1), VIDEO1_OSIZE);
  
  
  act_writel(0x04080770,VIDEO2_CFG);//video disable
  act_writel(0x4000,VIDEO2_SCOEF0);
  act_writel(0xff073efc,VIDEO2_SCOEF1);
  act_writel(0xfe1038fa,VIDEO2_SCOEF2);
  act_writel(0xfc1b30f9,VIDEO2_SCOEF3);
  act_writel(0xfa2626fa,VIDEO2_SCOEF4);
  act_writel(0xf9301bfc,VIDEO2_SCOEF5);
  act_writel(0xfa3810fe,VIDEO2_SCOEF6);
  act_writel(0xfc3e07ff,VIDEO2_SCOEF7);
  act_writel(0x20002000, VIDEO2_SR);
  act_writel(((disp_info.lcd_yres -1) << 16) | (disp_info.lcd_xres  - 1), VIDEO2_OSIZE);
}

void init_boot_disp(void){
  
  de_init();
    
  switch(disp_info.display_mode){
    case DISP_MODE_LCD:
         lcd_init();
         break;
    case DISP_MODE_HDMI:
         hdmi_init();
         break;
    case DISP_MODE_HDMI_LCD:
         lcd_init();
         hdmi_init();
         break;     
  }
    
//  // detected hdmi 
  if(hdmi_detected() != 1){
    //disp_info.display_mode &= ~DISP_MODE_HDMI;
    hdmi_setmode(disp_info.hdmi_mode);
  }else{
    hdmi_setmode(disp_info.hdmi_mode);
  }
}

void exit_boot_disp(void)
{	
    lcd_close();
}


void disp_dev_open(void){
  switch(disp_info.display_mode){
    case DISP_MODE_LCD:
         lcd_open();
         break;
    case DISP_MODE_HDMI:
         hdmi_open();
         break;
    case DISP_MODE_HDMI_LCD:
         lcd_open();
         hdmi_open();
         break;     
  }   
}

void disp_open_backlight()
{
	if(disp_info.display_mode & DISP_MODE_LCD){
		 back_light_open();
	}
}


void disp_frame_update(display_info * info){

    int tmp = 0;
    int format = info->main_disp_format;
    int mdsb_ctl = 0; 
       
//    adfu_printf("main_disp_addr 0x%x width:%d,height:%d,x:%d,y:%d,bytes_per_pix:%d \n",
//                info->main_disp_addr, info->main_disp_img_w,info->main_disp_img_h,
//                info->main_disp_pos_x, info->main_disp_pos_y,
//                info->main_disp_format); 
//                
//    adfu_printf("main_disp_addr 0x%x width:%d,height:%d,x:%d,y:%d,bytes_per_pix:%d \n",
//                info->hdmi_disp_addr, info->hdmi_disp_img_w,info->hdmi_disp_img_h,
//                info->hdmi_disp_pos_x, info->hdmi_disp_pos_y,
//                info->hdmi_disp_format);     
                
    if(disp_info.display_mode & DISP_MODE_LCD){
    	  format = info->main_disp_format;
        tmp = (info->main_disp_pos_y << 16) | info->main_disp_pos_x;
        act_writel(tmp, PATH2_A_COOR);
        tmp = ((info->main_disp_img_h - 1) << 16) | (info->main_disp_img_w - 1);
        act_writel(tmp, VIDEO2_ISIZE);    
        if(format == 4){
            //RGBA 888
            act_writel((act_readl(VIDEO2_CFG) & (~0x03)) | 0x05, VIDEO2_CFG);
            act_writel(info->main_disp_img_w/2, VIDEO2_STR);
        }else if(format == 2){
            //RGB565
            act_writel(act_readl(VIDEO2_CFG) & (~0x03), VIDEO2_CFG);
            act_writel(info->main_disp_img_w/4, VIDEO2_STR);
        }
        act_writel((act_readl(VIDEO2_CFG) | disp_info.lcd_rotate), VIDEO2_CFG);
        
        act_writel(info->main_disp_addr, VIDEO2_FB_0);
        
        act_writel(act_readl(PATH2_CTL) | 0x01, PATH2_CTL);
        act_writel(0x1, PATH2_EN);        
        mdsb_ctl = 0x10;
        if(get_lcd_type()==LCD_TYPE_DSI)
        {
        	act_writel(0x00000400, OUTPUT_CON);
        }
        else
        {
    		act_writel(0x00000000, OUTPUT_CON);
        }
        act_writel(0x1, PATH2_FCR);
    }
    if(disp_info.display_mode & DISP_MODE_HDMI){ 
    	    
    	  format = info->hdmi_disp_format;
        tmp = (info->hdmi_disp_pos_y << 16) | info->hdmi_disp_pos_x;
        act_writel(tmp, PATH1_A_COOR);
        tmp = ((info->hdmi_disp_img_h - 1) << 16) | (info->hdmi_disp_img_w - 1);
        act_writel(tmp, VIDEO1_ISIZE);
        
        tmp = (((info->hdmi_disp_img_h * 8192  +  disp_info.hdmi_yres - 1)/ disp_info.hdmi_yres)  << 16) 
               | ((info->hdmi_disp_img_w * 8192  +  disp_info.hdmi_xres - 1)/ disp_info.hdmi_xres);	
	      act_writel(tmp, VIDEO1_SR);
        if(format == 4){
            //RGBA 888
            act_writel((act_readl(VIDEO1_CFG) & (~0x03)) | 0x05, VIDEO1_CFG);
            act_writel(info->hdmi_disp_img_w/2, VIDEO1_STR);
        }else if(format == 2){
            //RGB565
            act_writel(act_readl(VIDEO1_CFG) & (~0x03), VIDEO1_CFG);
            act_writel(info->hdmi_disp_img_w/4, VIDEO1_STR);
        }
        act_writel(info->hdmi_disp_addr, VIDEO1_FB_0);
        if(info->hdmi_disp_addr != 0){
        	act_writel(act_readl(PATH1_CTL) | 0x01, PATH1_CTL);
      	}        
	      act_writel(0x1, PATH1_EN);    
	      mdsb_ctl |= 0x04;
	      act_writel(0x1, PATH1_FCR); 
      	
    }
    act_writel(0x6, DE_INTEN);
}

int get_hdmi_status(void)
{
  if(disp_info.display_mode & DISP_MODE_HDMI){
  	if(disp_info.lcd_xres < disp_info.lcd_yres){
  		return 2;
  	}else{
  		return 1;
  	}  	
  }else{
  	return 0;
  }
}
