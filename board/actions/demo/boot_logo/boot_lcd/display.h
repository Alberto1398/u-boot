#ifndef __BOOT_DISP_DRIVER__H__
#define __BOOT_DISP_DRIVER__H__

#define LCD_TYPE_RGB  	1
#define LCD_TYPE_LVDS 	2
#define LCD_TYPE_MIPI	3
#define LCD_TYPE_CPU 	4
#define LCD_TYPE_DSI 	5


typedef struct
{
    unsigned int main_disp_addr;
    unsigned int main_disp_format;
    unsigned int main_disp_img_w;
    unsigned int main_disp_img_h;
    unsigned int main_disp_pos_x;
    unsigned int main_disp_pos_y;
    
    unsigned int hdmi_disp_addr;
    unsigned int hdmi_disp_format;
    unsigned int hdmi_disp_img_w;
    unsigned int hdmi_disp_img_h;
    unsigned int hdmi_disp_pos_x;
    unsigned int hdmi_disp_pos_y;    
} display_info ;


/*
 * notice: back_light_open must be opened later then open lcd ,and the delay time at least 200ms
 */
 
void init_boot_disp(void);
void disp_frame_update(display_info * info);
void disp_dev_open(void);
void disp_open_backlight(void);
void exit_boot_disp(void);

/*this for lcd */
void lcd_init(void);
void lcd_close(void);
void lcd_open(void);
void back_light_close(void);
void back_light_open(void);
int get_lcd_version(void);
int get_lcd_type(void);

/*this for hdmi */
void hdmi_init(void);
int hdmi_detected(void);
void hdmi_setmode(int mode);
void hdmi_open(void);
int get_hdmi_status(void);
#endif
