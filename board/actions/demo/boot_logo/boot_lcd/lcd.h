#ifndef __GL5201_BOOT_LCD_DRIVER__H__
#define __GL5201_BOOT_LCD_DRIVER__H__

//#define LCD_HEIGHT 480
//#define LCD_WIDTH  800

//#define HFP_SEL		(204-1) 
//#define HBP_SEL		(168-1) 
//#define HSP_SEL		(173-1)  
                                      
//#define VFP_SEL		(44-1)  
//#define VBP_SEL		(35-1)   
//#define VSP_SEL		(20-1) 

#define CMU_DEVCLKEN1_PWM0          (1 << 8)
#define DE_BLKEN_CSC13_MASK         (0x3 << 5)
#define DE_BLKEN_CSC13_DISABLE      (0x0 << 5)


/*
 * notice: back_light_open must be opened later then open lcd ,and the delay time at least 200ms
 */
void init_boot_disp(void);
void lcd_close(void);
void lcd_open(void);
void back_light_close(void);
void back_light_open(void);
int get_lcd_version(void);
#endif