#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/dvfs_level.h>

#include "act_boot_pic.h"
#include "boot_lcd/display.h"


#define MBRC_INFO(FMT, ARGS...)  printf(FMT, ## ARGS)
#define MBRC_ERR(FMT, ARGS...)  printf(FMT, ## ARGS)


/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;

// return file_size, < 0 if error
static int _get_pic_file(const char *dev_ifname, const char *dev_part_str, uint fstype,
        char const *pic_name, void *lcd_buf)
{
    int ret;

    // check alignment
    BUG_ON(((uint)lcd_buf & (ARCH_DMA_MINALIGN -1U)) != 0);

    ret = fs_set_blk_dev(dev_ifname, dev_part_str, fstype);
    if(ret != 0)
    {
        MBRC_ERR("%s: probe fs err, ret=%d\n", __FUNCTION__, ret);
        return -1;
    }

    ret = fs_read(pic_name, (ulong)lcd_buf, 0, 0);
    if(ret <= 0)
    {
        MBRC_ERR("%s: read file %s err, ret=%d\n", __FUNCTION__, pic_name, ret);
        return -1;
    }
    return ret;
}


void dbg_info(void)
{
#if 0
    MBRC_INFO("de status information  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    MBRC_INFO("CMU_DISPLAYPLL =%x\n", act_readl(CMU_DISPLAYPLL));
    MBRC_INFO("CMU_DEVPLL =%x\n", act_readl(CMU_DEVPLL));
    MBRC_INFO("CMU_DECLK =%x\n", act_readl(CMU_DECLK));
    MBRC_INFO("DE_INTEN =%x\n", act_readl(DE_INTEN));
    MBRC_INFO("DE_STAT =%x\n", act_readl(DE_STAT));
    MBRC_INFO("\n\n");

    MBRC_INFO("path1 information  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    MBRC_INFO("PATH1_CTL =%x\n", act_readl(PATH1_CTL));
    MBRC_INFO("PATH1_FCR =%x\n", act_readl(PATH1_FCR));
    MBRC_INFO("PATH1_EN =%x\n", act_readl(PATH1_EN));
    MBRC_INFO("PATH1_BK =%x\n", act_readl(PATH1_BK));
    MBRC_INFO("PATH1_SIZE =%x\n", act_readl(PATH1_SIZE));
    MBRC_INFO("PATH1_A_COOR=%x\n", act_readl(PATH1_A_COOR));
    MBRC_INFO("\n\n");
    MBRC_INFO("path2 information  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    MBRC_INFO("PATH2_CTL =%x\n", act_readl(PATH2_CTL));
    MBRC_INFO("PATH2_FCR =%x\n", act_readl(PATH2_FCR));
    MBRC_INFO("PATH2_EN =%x\n", act_readl(PATH2_EN));
    MBRC_INFO("PATH2_BK =%x\n", act_readl(PATH2_BK));
    MBRC_INFO("PATH2_SIZE =%x\n", act_readl(PATH2_SIZE));
    MBRC_INFO("PATH2_A_COOR=%x\n", act_readl(PATH2_A_COOR));
    MBRC_INFO("\n\n");

    MBRC_INFO("video layer 1  information  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    MBRC_INFO("VIDEO1_BASE =%x\n", act_readl(VIDEO1_BASE));
    MBRC_INFO("VIDEO1_CFG =%x\n", act_readl(VIDEO1_CFG));
    MBRC_INFO("VIDEO1_ISIZE = 0x%x\n", act_readl(VIDEO1_ISIZE));
    MBRC_INFO("VIDEO1_OSIZE = 0x%x\n", act_readl(VIDEO1_OSIZE));
    MBRC_INFO("VIDEO1_SR = 0x%x\n", act_readl(VIDEO1_SR));
    MBRC_INFO("VIDEO1_SCOEF0 =%x\n", act_readl(VIDEO1_SCOEF0));
    MBRC_INFO("VIDEO1_SCOEF1 =%x\n", act_readl(VIDEO1_SCOEF1));
    MBRC_INFO("VIDEO1_SCOEF2 =%x\n", act_readl(VIDEO1_SCOEF2));
    MBRC_INFO("VIDEO1_SCOEF3 =%x\n", act_readl(VIDEO1_SCOEF3));
    MBRC_INFO("VIDEO1_SCOEF4 =%x\n", act_readl(VIDEO1_SCOEF4));
    MBRC_INFO("VIDEO1_SCOEF5 =%x\n", act_readl(VIDEO1_SCOEF5));
    MBRC_INFO("VIDEO1_SCOEF6 =%x\n", act_readl(VIDEO1_SCOEF6));
    MBRC_INFO("VIDEO1_SCOEF7 =%x\n", act_readl(VIDEO1_SCOEF7));
    MBRC_INFO("VIDEO1_FB_0 =%x\n", act_readl(VIDEO1_FB_0));
    MBRC_INFO("VIDEO1_FB_1 =%x\n", act_readl(VIDEO1_FB_1));
    MBRC_INFO("VIDEO1_FB_2 =%x\n", act_readl(VIDEO1_FB_2));
    MBRC_INFO("VIDEO1_FB_RIGHT_0 =%x\n", act_readl(VIDEO1_FB_RIGHT_0));
    MBRC_INFO("VIDEO1_FB_RIGHT_1 =%x\n", act_readl(VIDEO1_FB_RIGHT_1));
    MBRC_INFO("VIDEO1_FB_RIGHT_2 =%x\n", act_readl(VIDEO1_FB_RIGHT_2));
    MBRC_INFO("VIDEO1_STR =%x\n", act_readl(VIDEO1_STR));
    MBRC_INFO("VIDEO1_REMAPPING =%x\n", act_readl(VIDEO1_REMAPPING));
    MBRC_INFO("\n\n");
    MBRC_INFO("video layer 2  information  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    MBRC_INFO("VIDEO2_BASE =%x\n", act_readl(VIDEO2_BASE));
    MBRC_INFO("VIDEO2_CFG =%x\n", act_readl(VIDEO2_CFG));
    MBRC_INFO("VIDEO2_ISIZE = 0x%x\n", act_readl(VIDEO2_ISIZE));
    MBRC_INFO("VIDEO2_OSIZE = 0x%x\n", act_readl(VIDEO2_OSIZE));
    MBRC_INFO("VIDEO2_SR = 0x%x\n", act_readl(VIDEO2_SR));
    MBRC_INFO("VIDEO2_SCOEF0 =%x\n", act_readl(VIDEO2_SCOEF0));
    MBRC_INFO("VIDEO2_SCOEF1 =%x\n", act_readl(VIDEO2_SCOEF1));
    MBRC_INFO("VIDEO2_SCOEF2 =%x\n", act_readl(VIDEO2_SCOEF2));
    MBRC_INFO("VIDEO2_SCOEF3 =%x\n", act_readl(VIDEO2_SCOEF3));
    MBRC_INFO("VIDEO2_SCOEF4 =%x\n", act_readl(VIDEO2_SCOEF4));
    MBRC_INFO("VIDEO2_SCOEF5 =%x\n", act_readl(VIDEO2_SCOEF5));
    MBRC_INFO("VIDEO2_SCOEF6 =%x\n", act_readl(VIDEO2_SCOEF6));
    MBRC_INFO("VIDEO2_SCOEF7 =%x\n", act_readl(VIDEO2_SCOEF7));
    MBRC_INFO("VIDEO2_FB_0 =%x\n", act_readl(VIDEO2_FB_0));
    MBRC_INFO("VIDEO2_FB_1 =%x\n", act_readl(VIDEO2_FB_1));
    MBRC_INFO("VIDEO2_FB_2 =%x\n", act_readl(VIDEO2_FB_2));
    MBRC_INFO("VIDEO2_FB_RIGHT_0 =%x\n", act_readl(VIDEO2_FB_RIGHT_0));
    MBRC_INFO("VIDEO2_FB_RIGHT_1 =%x\n", act_readl(VIDEO2_FB_RIGHT_1));
    MBRC_INFO("VIDEO2_FB_RIGHT_2 =%x\n", act_readl(VIDEO2_FB_RIGHT_2));
    MBRC_INFO("VIDEO2_STR =%x\n", act_readl(VIDEO2_STR));
    MBRC_INFO("VIDEO2_REMAPPING =%x\n", act_readl(VIDEO2_REMAPPING));
    MBRC_INFO("\n\n");

    MBRC_INFO("OUTPUT_STAT = %x\n",act_readl(OUTPUT_STAT));
    MBRC_INFO("OUTPUT_CON = %x\n",act_readl(OUTPUT_CON));

    MBRC_INFO(" LCD infomations  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    MBRC_INFO("CMU_LCDCLK =%x\n", act_readl(CMU_LCDCLK));
    MBRC_INFO("LCD_CTL =%x\n", act_readl(LCD_CTL));
    MBRC_INFO("LCD_SIZE =%x\n", act_readl(LCD_SIZE));
    MBRC_INFO("LCD_STATUS =%x\n", act_readl(LCD_STATUS));
    MBRC_INFO("LCD_TIM0 =%x\n", act_readl(LCD_TIM0));
    MBRC_INFO("LCD_TIM1 =%x\n", act_readl(LCD_TIM1));
    MBRC_INFO("LCD_TIM2 =%x\n", act_readl(LCD_TIM2));
    MBRC_INFO("LCD_COLOR =%x\n", act_readl(LCD_COLOR));
    MBRC_INFO("LCD_CPU_CTL =%x\n", act_readl(LCD_CPU_CTL));
    MBRC_INFO("LCD_CPU_CMD =%x\n", act_readl(LCD_CPU_CMD));
    MBRC_INFO("LCD_TEST_P0 =%x\n", act_readl(LCD_TEST_P0));
    MBRC_INFO("LCD_TEST_P1 =%x\n", act_readl(LCD_TEST_P1));
    MBRC_INFO("LCD_IMG_XPOS =%x\n", act_readl(LCD_IMG_XPOS));
    MBRC_INFO("LCD_IMG_YPOS =%x\n", act_readl(LCD_IMG_YPOS));

    MBRC_INFO("PAD_CTL =%x\n", act_readl(PAD_CTL));
    MBRC_INFO("MFP_CTL2 =%x\n", act_readl(MFP_CTL2));
    MBRC_INFO("MFP_CTL1 =%x\n", act_readl(MFP_CTL1));
    MBRC_INFO("MFP_CTL0 =%x\n", act_readl(MFP_CTL0));
    MBRC_INFO("PAD_DRV1 =%x\n", act_readl(PAD_DRV1));
    MBRC_INFO("CMU_DEVCLKEN0 =%x\n", act_readl(CMU_DEVCLKEN0));
    MBRC_INFO("CMU_DEVCLKEN1 =%x\n", act_readl(CMU_DEVCLKEN1));

    MBRC_INFO("CMU_PWM0CLK =%x\n", act_readl(CMU_PWM0CLK));
    MBRC_INFO("PWM_CTL0 =%x\n", act_readl(PWM_CTL0));
    MBRC_INFO("GPIO_BOUTEN =%x\n", act_readl(GPIO_BOUTEN));
    MBRC_INFO("GPIO_BINEN =%x\n", act_readl(GPIO_BINEN));
    MBRC_INFO("GPIO_BDAT =%x\n", act_readl(GPIO_BDAT));

    MBRC_INFO("LVDS_CTL =%x\n", act_readl(LVDS_CTL));
    MBRC_INFO("LVDS_ALG_CTL0 =%x\n", act_readl(LVDS_ALG_CTL0));
    MBRC_INFO("HDMI_TX_1 =%x\n", act_readl(HDMI_TX_1));


#endif
}

/******************************************************************************/
/*!
* \par  Description:
*     Î¢Ãî¼¶ÑÓÊ±
* \param[in]    int num ÑÓÊ±³¤¶È
* \par
* Í¨¹ýcp0_count¼ÆÊ±
* \code
* \endcode
*******************************************************************************/

static int check_lcd_is_invalid(void)
{
    int ret, temp[2];
    uint lcd_width = 1920;
    uint lcd_height = 1280;
    uint MAX_LCD_SIZE = 0;

    ret = act_pmem_get_max_lcd_size(&MAX_LCD_SIZE);
    if(ret != 0)
    {
        return -1;
    }

    memset(&temp, -1, sizeof(temp));
    act_xmlp_get_config("lcd.resolution",(char*)&temp[0], 2 * sizeof(int));
    if(temp[0] != -1 && temp[1] != -1)
    {
        lcd_width = temp[0];
        lcd_height = temp[1];
    }
    if(lcd_width * lcd_height > MAX_LCD_SIZE)
    {
        return -1;
    }
    return 0;
}


static uint8_t lcd_init_flag = 0;
static uint8_t s_hdmi_status = 0;

static int lcd_init_1(int *hdmi_status)
{
    int ret=0;
    int lcd_version;

    if(lcd_init_flag)
    {
        return 0;
    }

    ret = check_lcd_is_invalid();
    if(ret < 0)
    {
         MBRC_ERR("%s: sorry, lcd not support\n", __FUNCTION__);
         return ret;
    }

    lcd_version = get_lcd_version();
    if(lcd_version != 0x01cd)
    {
        MBRC_ERR("%s: lcd_version not match\n", __FUNCTION__);
        return -1;
    }

    init_boot_disp();
    MBRC_INFO("init_boot_disp called ok \n");

    s_hdmi_status = get_hdmi_status();
    if(hdmi_status != NULL)
    {
        *hdmi_status = s_hdmi_status;
    }

    // adjust_max_cpufreq();

    disp_dev_open();
    return ret;
}

void lcd_init_2(void)
{
//    unsigned long timea, timeb;

    if(lcd_init_flag)
    {
        return ;
    }
    //boot_mdelay(100);
    disp_open_backlight();

    lcd_init_flag = 1;
    return ;
}

static int display_pic(const char *dev_ifname, const char *dev_part_str, uint fstype,
        char const *pic_name)
{
    int ret=0;
    void *fb_start_addr = (void *)(gd->bd->cfg_ddr_size - gd->bd->cfg_fb_size/2U - sizeof(boot_pic_info_t));
    void *lcd_buf = fb_start_addr;
    boot_pic_info_t *p_boot_pic_info=NULL;
    display_info disp_info;

    memset(&disp_info, 0, sizeof(disp_info));

    ret = _get_pic_file(dev_ifname, dev_part_str, fstype, pic_name, lcd_buf);
    if(ret < 0)
    {
        MBRC_ERR("%s: get %s fail\n", __FUNCTION__, pic_name);
        return ret;
    }

    ret = lcd_init_1(NULL);
    if(ret < 0)
    {
        return ret;
    }

    p_boot_pic_info = (boot_pic_info_t *)lcd_buf;
//    dump_mem(p_boot_pic_info, 32, 0, 1);
    MBRC_INFO("%s: width:%d,height:%d,x:%d,y:%d,bytes_per_pix:%d,length:%d\n",
            __FUNCTION__,
            p_boot_pic_info->width, p_boot_pic_info->height,
            p_boot_pic_info->x, p_boot_pic_info->y,
            p_boot_pic_info->bytes_per_pix, p_boot_pic_info->length
    );

    disp_info.main_disp_addr  = (unsigned int)lcd_buf + sizeof(boot_pic_info_t);
    disp_info.main_disp_format = p_boot_pic_info->bytes_per_pix;
    disp_info.main_disp_img_w = p_boot_pic_info->width;
    disp_info.main_disp_img_h = p_boot_pic_info->height;
    disp_info.main_disp_pos_x = p_boot_pic_info->x;
    disp_info.main_disp_pos_y = p_boot_pic_info->y;

    p_boot_pic_info = (boot_pic_info_t *)lcd_buf;

    disp_info.hdmi_disp_addr  = (unsigned int)lcd_buf + sizeof(boot_pic_info_t);
    disp_info.hdmi_disp_format = p_boot_pic_info->bytes_per_pix;
    disp_info.hdmi_disp_img_w = p_boot_pic_info->width;
    disp_info.hdmi_disp_img_h = p_boot_pic_info->height;
    disp_info.hdmi_disp_pos_x = p_boot_pic_info->x;
    disp_info.hdmi_disp_pos_y = p_boot_pic_info->y;

    disp_frame_update(&disp_info);

    lcd_init_2();

    dbg_info();

    return ret;
}

#if 1   /* dual LOGO support, HDMI LOGO support */

#ifdef CONFIG_ACTS_STORAGE_NAND
extern int ReadExtensionType(int type, unsigned char * buf_data, int size);
int get_bootlogo_flag(unsigned char * flag)
{
    return ReadExtensionType(5, flag, 4);//#define MISC_INFO_SWITCH_NAME 5
}
#endif

int display_welcome(const char *dev_ifname, const char *dev_part_str, uint fstype)
{
    int ret=0;
    void *fb_start_addr = (void *)(gd->bd->cfg_ddr_size - gd->bd->cfg_fb_size/2U - sizeof(boot_pic_info_t));
    void *lcd_buf = fb_start_addr;
    void *hdmi_buf = (uint8_t *)fb_start_addr - gd->bd->cfg_fb_size/2U;
    int hdmi_actived = 0;
    char const *boot_logo_filename;

    boot_pic_info_t *p_boot_pic_info=NULL;

    display_info disp_info;

    memset(&disp_info, 0, sizeof(disp_info));

    ret = lcd_init_1(&hdmi_actived);
    if(ret < 0)
    {
        return ret;
    }

    //boot dual logo exchange
    boot_logo_filename = "boot_pic.bin";
#if defined(CONFIG_ACTS_STORAGE_NAND)
    {
        unsigned char boot_logo_sel[4];
        unsigned int logo_change = 0;
        ret = act_xmlp_get_config(
                "dual_logo.support", (char *)(&logo_change), sizeof(unsigned int));
        if(ret == 0)
        {
            MBRC_INFO("dual_logo.support is:%d\n", logo_change);
            if(logo_change == 1)
            {
                ret = get_bootlogo_flag(boot_logo_sel);
                if(ret > 0 && boot_logo_sel[0] == '1')
                {
                    boot_logo_filename = "boot_p2.bin";
                }
                else
                {
                    boot_logo_filename = "boot_pic.bin";
                }
            }
        }
    }
#endif
    ret = _get_pic_file(dev_ifname, dev_part_str, fstype, boot_logo_filename, lcd_buf);
    if(ret < 0)
    {
        MBRC_ERR("%s: get %s fail\n", __FUNCTION__, boot_logo_filename);
        return ret;
    }

    p_boot_pic_info = (boot_pic_info_t *)lcd_buf;
//    dump_mem(p_boot_pic_info, 32, 0, 1);
    MBRC_INFO("%s: width:%d,height:%d,x:%d,y:%d,bytes_per_pix:%d,length:%d\n",
            __FUNCTION__,
            p_boot_pic_info->width, p_boot_pic_info->height,
            p_boot_pic_info->x, p_boot_pic_info->y,
            p_boot_pic_info->bytes_per_pix, p_boot_pic_info->length);

    disp_info.main_disp_addr  = (unsigned int)lcd_buf + sizeof(boot_pic_info_t);
    disp_info.main_disp_format = p_boot_pic_info->bytes_per_pix;
    disp_info.main_disp_img_w = p_boot_pic_info->width;
    disp_info.main_disp_img_h = p_boot_pic_info->height;
    disp_info.main_disp_pos_x = p_boot_pic_info->x;
    disp_info.main_disp_pos_y = p_boot_pic_info->y;

    MBRC_INFO("%s: main_disp_addr 0x%x width:%d,height:%d,x:%d,y:%d,bytes_per_pix:%d,length:%d\n",
            __FUNCTION__,
            disp_info.main_disp_addr, disp_info.main_disp_img_w,disp_info.main_disp_img_h,
            disp_info.main_disp_pos_x, disp_info.main_disp_pos_y,
            p_boot_pic_info->bytes_per_pix, p_boot_pic_info->length);

    if(hdmi_actived == 2)
    {
        ret = _get_pic_file(dev_ifname, dev_part_str, fstype, "hdmilogo.bin", hdmi_buf);
        MBRC_INFO("boot_pic_hdmi.bin ret = %d",ret);
        if(ret >= 0)
        {
            p_boot_pic_info = (boot_pic_info_t *)hdmi_buf;
            disp_info.hdmi_disp_addr  = (unsigned int)hdmi_buf + sizeof(boot_pic_info_t);
            disp_info.hdmi_disp_format = p_boot_pic_info->bytes_per_pix;
            disp_info.hdmi_disp_img_w = p_boot_pic_info->width;
            disp_info.hdmi_disp_img_h = p_boot_pic_info->height;
            disp_info.hdmi_disp_pos_x = p_boot_pic_info->x;
            disp_info.hdmi_disp_pos_y = p_boot_pic_info->y;
        }
    }
    else if(hdmi_actived == 1)
    {
        MBRC_INFO("hdmi_actived  = %d",hdmi_actived);
        disp_info.hdmi_disp_addr  = disp_info.main_disp_addr;
        disp_info.hdmi_disp_format = disp_info.main_disp_format;
        disp_info.hdmi_disp_img_w = disp_info.main_disp_img_w;
        disp_info.hdmi_disp_img_h = disp_info.main_disp_img_h;
        disp_info.hdmi_disp_pos_x = disp_info.main_disp_pos_x;
        disp_info.hdmi_disp_pos_y = disp_info.main_disp_pos_y;
    }

    disp_frame_update(&disp_info);

    lcd_init_2();

    dbg_info();

    return 0;
}
#else
int display_welcome(const char *dev_ifname, const char *dev_part_str, uint fstype)
{
    return display_pic(dev_ifname, dev_part_str, fstype, "boot_pic.bin");
}
#endif

int display_lpow(const char *dev_ifname, const char *dev_part_str, uint fstype)
{
    return display_pic(dev_ifname, dev_part_str, fstype, "lpow_pic.bin");
}
int display_minicharge(const char *dev_ifname, const char *dev_part_str, uint fstype)
{
    return display_pic(dev_ifname, dev_part_str, fstype, "mini_pic.bin");
}

uint display_get_hdmi_status(void)
{
    return s_hdmi_status;
}

void display_exit(void)
{
    MBRC_INFO("exit LOGO display\n");
    if(lcd_init_flag != 0)
    {
        lcd_init_flag = 0;
        exit_boot_disp();
    }
}
