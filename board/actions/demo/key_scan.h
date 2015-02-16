#ifndef __ADC_KEY_H__
#define __ADC_KEY_H__

#define KEY_RESERVED        0
#define KEY_HOME        102
#define KEY_UP          103
#define KEY_VOLUMEDOWN      114
#define KEY_VOLUMEUP        115
#define KEY_MENU        139 /* Menu (show menu) */

/* board option by adckey */
#define KEY_BOARD_OPTION0       (10)
#define KEY_BOARD_OPTION1       (11)
#define KEY_BOARD_OPTION2       (12)
#define KEY_BOARD_OPTION3       (13)
#define KEY_BOARD_OPTION4       (14)
#define KEY_BOARD_OPTION5       (15)

void act_adc_key_boot_init(void);
uint act_adc_key_scan(void);

int act_key_scan(unsigned char scan_mode);

#endif
