#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "../boot_lcd_cfg.h"
#include "../display.h"

#include "asoc_hdmi.h"


#define FB_VMODE_NONINTERLACED  0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced*/
#define FB_MODE_IS_STANDARD	2

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

struct asoc_hdmi hdmi;
static struct hdmi_settings default_v_settings = {
    .vid = VID1280x720P_50_16VS9,
    .hdmi_mode = HDMI_MODE_HDMI,
    .video2to1scaler = 0,
    .hdmi_src = DE,
    .pixel_encoding = VIDEO_PEXEL_ENCODING_YCbCr444,
    .color_xvycc = YCC601_YCC709,
    .deep_color = DEEP_COLOR_24_BIT,
    ._3d = _3D_NOT,
};
	
static struct video_parameters_t video_parameters[] = {
	               /*vid                                                                                      ar       SCAN_MODE              vivsync                           vialseof                        viadlse                         dipccr_30                        vhsync_p                    
                                                                             vid_str           Video2to1Scaler      colorimetry      victl                            vivhsync                          vialseef                       dipccr_24                      dipccr_36                    vhsync_inv*/
    {VID640x480P_60_4VS3,           "VID640x480P_60_4VS3",           0x0, 0x01, 0x01, 0x0, 0x031f20c0, 0x00000000, 0x120c05f,  0x00202022, 0x00000000, 0x030f008f, 0x00000701, 0x00000702,  0x00000703, 0x00000000, 0x00000006},	

	//{VID720x480P_59P94_4VS3,    "VID720x480P_59P94_4VS3",    0x0, 0x01, 0x01, 0x0, 0x035920c0, 0x00000000, 0x0b00503d, 0x00209029, 0x00000000, 0x03490079, 0x00000701, 0x00000701, 0x00000702, 0x00000000, 0x00000006},	
	//{VID720x480P_59P94_16VS9,   "VID720x480P_59P94_16VS9",  0x0, 0x10, 0x01, 0x0, 0x035920c0, 0x00000000, 0x0b00503d, 0x00209029, 0x00000000, 0x03490079, 0x00000701, 0x00000701, 0x00000702, 0x00000000, 0x00000006},	
    {VID720x480P_60_4VS3,            "VID720x480P_60_4VS3",          0x0, 0x01, 0x01, 0x0, 0x035920c0, 0x00000000, 0x0b00503d, 0x00209029, 0x00000000, 0x03490079, 0x00000701, 0x00000701, 0x00000702, 0x00000000, 0x00000006},	
    {VID720x480P_60_16VS9,          "VID720x480P_60_16VS9",         0x0, 0x10, 0x01, 0x0, 0x035920c0, 0x00000000, 0x0b00503d, 0x00209029, 0x00000000, 0x03490079, 0x00000701, 0x00000701, 0x00000702, 0x00000000, 0x00000006},	

    {VID1280x720P_60_16VS9,        "VID1280x720P_60_16VS9",        0x0, 0x10, 0x10, 0x0, 0x06712ed0, 0x00000000, 0x042ed027, 0x002e8018, 0x00000000, 0x06030103, 0x00001107, 0x00001108, 0x00001108, 0x00000000, 0x00000000},
    {VID1280X720P_59P94_16VS9,  "VID1280X720P_59P94_16VS9", 0x0, 0x10, 0x01, 0x0, 0x06712ed0, 0x00000000, 0x042ed027, 0x002e8018, 0x00000000, 0x06030103, 0x00001107, 0x00001108, 0x00001108, 0x00000000, 0x00000000},
	//the followings need modify
	//{VID720x480I_59P54_4VS3,      "VID720x480I_59P54_4VS3",       0x1, 0x01, 0x01, 0x1, 0x36b320c0, 0x0010c109, 0x0500207b, 0x00104014, 0x0020b11b, 0x068d00ed, 0x00001105, 0x0,            0x0,            0x00000000, 0x00000000},
	//{VID720x480I_59P54_16VS9,    "VID720x480I_59P54_16VS9",     0x1, 0x10, 0x01, 0x1, 0x36b320c0, 0x0010c109, 0x0500207b, 0x00104014, 0x0020b11b, 0x068d00ed, 0x00001105, 0x0,            0x0,            0x00000000, 0x00000006},
	//{VID720x480I_60_4VS3,           "VID720x480I_60_4VS3",             0x1, 0x01, 0x01, 0x1, 0x36b320c0, 0x0010c109, 0x0500207b, 0x00104014, 0x0020b11b, 0x068d00ed, 0x00001105, 0x0,            0x0,            0x00000000, 0x00000000},
	//{VID720x480I_60_16VS9,         "VID720x480I_60_16VS9",            0x1, 0x10, 0x01, 0x1, 0x36b320c0, 0x0010c109, 0x0500207b, 0x00104014, 0x0020b11b, 0x068d00ed, 0x00001105, 0x0,            0x0,            0x00000000, 0x00000000},

    {VID720x576P_50_4VS3,           "VID720x576P_50_4VS3",             0x0, 0x01, 0x01,0x0, 0x035f2700, 0x00000000, 0x0427003f,  0x0026b02b, 0x00000000, 0x03530083, 0x00000701, 0x00000701, 0x00000702, 0x00000000, 0x00000006},
    {VID720x576P_50_16VS9,         "VID720x576P_50_16VS9",           0x0, 0x10, 0x01, 0x0, 0x035f2700, 0x00000000, 0x0427003f, 0x0026b02b, 0x00000000, 0x03530083, 0x00000701, 0x00000701, 0x00000702, 0x00000000, 0x00000006},

    {VID1280x720P_50_16VS9,       "VID1280x720P_50_16VS9",          0x0, 0x10, 0x10, 0x0, 0x07bb2ed0, 0x00000000, 0x042ed027, 0x002e8018, 0x00000000, 0x06030103, 0x00001107, 0x00001108, 0x00001108, 0x00000000, 0x00000000},

    {VID720x576I_50_4VS3,           "VID720x576I_50_4VS3",                0x1, 0x01, 0x01, 0x1, 0x36bf2700, 0x0013b138, 0x0227007d, 0x00135015, 0x0026e14e, 0x06a70107, 0x00001105, 0x00001107, 0x00001108, 0x00000000, 0x00000006},
    {VID720x576I_50_16VS9,         "VID720x576I_50_16VS9",              0x1, 0x10, 0x01, 0x1, 0x36bf2700, 0x0013b138, 0x0227007d, 0x00135015, 0x0026e14e, 0x06a70107, 0x00001105, 0x00001107, 0x00001108, 0x00000000, 0x00000006},

    {VID1440x480P_60_4VS3,         "VID1440x480P_60_16VS9",          0x1, 0x01, 0x01, 0x0, 0x06b320c0, 0x00000000, 0x0b00507b, 0x00209029, 0x00000000, 0x069300f3, 0x00001105, 0x00001106,0x00001108, 0x00000000, 0x00000006},
    {VID1440x480P_60_16VS9,       "VID1440x480P_60_16VS9",          0x1, 0x10, 0x01, 0x0, 0x06b320c0, 0x00000000, 0x0b00507b, 0x00209029, 0x00000000, 0x069300f3, 0x00001105, 0x00001106,0x00001108, 0x00000000, 0x00000006},

    {VID1440x576P_50_4VS3,         "VID1440x576P_50_4VS3",            0x1, 0x01, 0x01, 0x0, 0x06bf2700, 0x00000000, 0x0427007f, 0x0026b02b, 0x00000000, 0x06a70107, 0x00001105, 0x00001107,0x00001108, 0x00000000, 0x00000006},
    {VID1440x576P_50_16VS9,       "VID1440x576P_50_16VS9",          0x1, 0x10, 0x01, 0x0, 0x06bf2700, 0x00000000, 0x0427007f, 0x0026b02b, 0x00000000, 0x06a70107, 0x00001105, 0x00001107,0x00001108, 0x00000000, 0x00000006},

    {VID1920x1080I_60_16VS9,      "VID1920x1080I_60_16VS9",          0x0, 0x10, 0x10, 0x1, 0x18974640, 0x00237232, 0x0446402b, 0x0022f013, 0x00462246, 0x083f00bf, 0x00001105, 0x00001107,0x00001108, 0x00000000, 0x00000000},
	//{VID1920x1080I_59P94_16VS9, "VID1920x1080I_59P94_16VS9",  0x0, 0x10, 0x01, 0x1, 0x18974640, 0x00237232, 0x0446402b, 0x0022f013, 0x00462246, 0x083f00bf, 0x00001105, 0x00001107,0x00001108, 0x00000000, 0x00000000},
 
    {VID1920x1080I_50_16VS9,      "VID1920x1080I_50_16VS9",           0x0, 0x10, 0x10, 0x1, 0x1a4f4640, 0x00237232, 0x0446402b, 0x0022f013, 0x00462246, 0x083f00bf, 0x00001107, 0x00001108,0x00001108, 0x00000000, 0x00000000},
 
    {VID1920x1080P_60_16VS9,      "VID1920x1080P_60_16VS9",         0x0, 0x10, 0x10, 0x0, 0x08974640, 0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001105, 0x00001107, 0x00001108, 0x00000000, 0x00000000},
    {VID1920x1080P_59P94_16VS9, "VID1920x1080P_59P94_16VS9",  0x0, 0x10, 0x01, 0x0, 0x08974640, 0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001105, 0x00001107, 0x00001108, 0x00000000, 0x00000000},
	
    {VID1920x1080P_50_16VS9,      "VID1920x1080P_50_16VS9",        0x0, 0x10, 0x10, 0x0, 0xa4f4640,  0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001107,  0x00001108, 0x00001108, 0x00000000, 0x00000000},

    {VID1920x1080P_30_16VS9,      "VID1920x1080P_30_16VS9",        0x0, 0x10, 0x10, 0x0, 0x08974640, 0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001105, 0x00001107, 0x00001108, 0x00000000, 0x00000000},
	//{VID1920x1080P_29P97_16VS9, "VID1920x1080P_29P97_16VS9", 0x0, 0x10, 0x01, 0x0, 0x08974640, 0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001107, 0x00001107, 0x00001108, 0x00000000, 0x00000000},

    {VID1920x1080P_25_16VS9,      "VID1920x1080P_25_16VS9",         0x0, 0x10, 0x10, 0x0, 0x0a4f4640, 0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001107, 0x00001108, 0x00001108, 0x00000000, 0x00000000},

    {VID1920x1080P_24_16VS9,      "VID1920x1080P_24_16VS9",        0x0, 0x10, 0x10, 0x0, 0x0abd4640, 0x00000000, 0x0446402b, 0x00460028, 0x00000000, 0x083f00bf, 0x00001107, 0x00001108, 0x00001108, 0x00000000, 0x00000000},
};


static  struct de_video_mode hdmi_display_modes[] = {
                       /*name              xres         pixclock        hbp        vbp          vsw             flag
	                      refresh          yres               hfp         vfp          hsw         vmode        vid*/
    {"HDMI640x480P_60_4VS3", 60, 640, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID640x480P_60_4VS3},
    {"HDMI720x576P_50_4VS3", 50, 720, 576, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID720x576P_50_4VS3},
    {"HDMI720x576P_50_16VS9", 50, 720, 576, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID720x576P_50_16VS9},
	//{"HDMI720x480P_59P94_4VS3", 59.94, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID720x480P_59P94_4VS3},
	//{"HDMIVID720x480P_59P94_16VS9", 59.94, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID720x480P_59P94_16VS9},
	//{"HDMI720x480I_59P54_4VS3", 59.94, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID720x480I_59P54_4VS3},
	//{"HDMI720x480I_59P54_16VS9", 59.94, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID720x480I_59P54_16VS9},
    {"HDMI_720x576I_50_4VS3", 50, 720, 576, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID720x576I_50_4VS3},
    {"HDMI_720x576I_50_16VS9", 50, 720, 576, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID720x576I_50_16VS9},
    {"HDMI_720x480P_60_4VS3", 60, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID720x480P_60_4VS3},
    {"HDMI_720x480P_60_16VS9", 60, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID720x480P_60_16VS9},
	//{"HDMI_720x480I_60_4VS3", 60, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID720x480I_60_4VS3},
	//{"HDMI_720x480I_60_16VS9", 60, 720, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID720x480I_60_16VS9},
    {"HDMI_1440x480P_60_4VS3", 60, 1440, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1440x480P_60_4VS3,},
    {"HDMI_1440x480P_60_16VS9", 60, 1440, 480, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1440x480P_60_16VS9},
    {"HDMI_1440x576P_50_4VS3", 50, 1440, 576, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1440x576P_50_4VS3},
    {"HDMI_1440x576P_50_16VS9", 50, 1440, 576, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1440x576P_50_16VS9},
    {"HDMI_1280x720P_60_16VS9", 60, 1280, 720, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1280x720P_60_16VS9},
    {"HDMI_1280x720P_50_16VS9", 50, 1280, 720, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1280x720P_50_16VS9},
    {"HDMI_1920x1080I_60_16VS9", 60, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID1920x1080I_60_16VS9},
    {"HMDI_1920x1080I_50_16VS9", 50, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID1920x1080I_50_16VS9},
    {"HDMI_1920x1080P_24_16VS9", 24, 1920, 1080, 0, 0, 0, 0, 0, 0, 0,  FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_24_16VS9},
    {"VID1920x1080P_25_16VS9", 25, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_25_16VS9},
    {"HDMI_1920x1080P_30_16VS9", 60, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_30_16VS9},
    {"HDMI_1920x1080P_60_16VS9", 60, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_60_16VS9}, 
    {"HDMI_1920x1080P_50_16VS9", 50, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_50_16VS9},
    {"HDMI_1280X720P_59P94_16VS9", 59.94, 1280, 720, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1280X720P_59P94_16VS9},
	//{"VID1920x1080I_59P94_16VS9", 59.94, 1920, 1080, 0, 0, 0, 0, 0, 0,0, FB_VMODE_INTERLACED, FB_MODE_IS_STANDARD, VID1920x1080I_59P94_16VS9},
	//{"HDMI_1920x1080P_29P97_16VS9", 29.97, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_29P97_16VS9},
    {"HDMI_1920x1080P_59P94_16VS9", 59.94, 1920, 1080, 0, 0, 0, 0, 0, 0, 0, FB_VMODE_NONINTERLACED, FB_MODE_IS_STANDARD, VID1920x1080P_59P94_16VS9} ,
};

static void *hdmi_find_id(int which_table, int len)
{
    int i;
	
    if (which_table == VIDEO_HDMI) {	
        for (i = 0; i < len; i++) {
            if (video_parameters[i].vid ==default_v_settings.vid) {
                hdmi.ip_data.v_cfg.parameters = &video_parameters [i];
                return  (void *)&video_parameters [i]; 
            }
        }
    } else if (which_table == VIDEO_DE) {
        for (i = 0; i < len; i++) {
            if (hdmi_display_modes[i].vid == default_v_settings.vid) {
                hdmi.ip_data.v_cfg.mode = &hdmi_display_modes[i];
                return  (void *)&hdmi_display_modes [i]; 
            }
        }
    }

    return NULL;
}

void * hdmi_get_id(int which_table)
{
    int len;
	
    if (which_table == VIDEO_HDMI) {
        //len = ARRAY_SIZE(video_parameters);
         len = ARRAY_SIZE(video_parameters);
		
    } else if (which_table == VIDEO_DE) {
        //len = ARRAY_SIZE(hdmi_display_modes);
        len = ARRAY_SIZE(hdmi_display_modes);		
    }  else {
        return NULL;
    }
		
    return hdmi_find_id(which_table, len);
}

/*static int vid_str2int(char *vidstr)
{
    int i;
    for (i = 0; i < ARRAY_SIZE(video_parameters); i++) {
        if (!strcmp(vidstr, video_parameters[i])) {
            return video_parameters[i].vid;
        }
    }
}*/

void hdmi_init(void)
{

    hdmi.ip_data.v_cfg.settings = &default_v_settings;
    
    asoc_520x_hdmi_init_configure();
}

int hdmi_detected(void)
{
    int plug_state = asoc_520x_hdmi_detect_plug();

    return plug_state;
}

void hdmi_setmode(int vid)
{
    
    if (!vid) {
        //从EDID列表中选择最佳分辨率输出
    } else {
        //if (vid在EDID列表中有支持) {
            default_v_settings.vid = vid;
        //} else {
            //从EDID列表中选择最佳分辨率输出
        //}
    }
    
    /*timing config*/
    asoc_520x_hdmi_timing_configure(&hdmi.ip_data);

    /*video para config*/
    struct video_parameters_t *para = hdmi_get_id(VIDEO_HDMI);
    if (para) {
        asoc_520x_hdmi_video_configure(&hdmi.ip_data);
    } 

    /*packet config*/
    asoc_520x_hdmi_gcp_configure(&hdmi.ip_data);
    asoc_520x_hdmi_gen_infoframe(&hdmi.ip_data);

    /*hdmi mode config*/
    asoc_520x_hdmi_set_mode(&hdmi.ip_data);

    /*set hdmi source*/
    asoc_520x_hdmi_src_set(&hdmi.ip_data);
}

void hdmi_open(void)
{
    /*enable hdmi output*/
    asoc_520x_hdmi_phy_enable(&hdmi.ip_data);
}
