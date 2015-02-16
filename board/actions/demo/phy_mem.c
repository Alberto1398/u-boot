#include <common.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/dvfs_level.h>

DECLARE_GLOBAL_DATA_PTR;


int act_pmem_get_max_lcd_size(uint *max_lcd_size)
{
	int ret=0;

    switch (ASOC_GET_IC(gd->arch.dvfs_level))
    {
    case 0x7023:
    case 0x7021:    
        *max_lcd_size  = 1024 * 768;
        break;        
    case 0x7029:
        *max_lcd_size = 1920 * 1280;
        break;     
    default:
		ret=-1;
        break;
    }
	return ret;
}

static int act_pmem_fb_size_init(unsigned int *p_fb_size)
{
    int ret = 0;
	uint temp[2];
	uint lcd_width = 0;
	uint lcd_height = 0;
	uint MAX_LCD_SIZE;
	
	act_pmem_get_max_lcd_size(&MAX_LCD_SIZE);
	
	memset(&temp, -1, sizeof(temp));	
	act_xmlp_get_config("lcd.resolution",(char*)&temp[0], 2 * sizeof(int));
	if(temp[0] != -1 && temp[1] != -1)
	{
		lcd_width = temp[0];
	    lcd_height = temp[1];
		if(lcd_width*lcd_height > MAX_LCD_SIZE)
		{
			*p_fb_size = 0;
			ret = -1;
		}
		else
		{
			*p_fb_size = lcd_width*lcd_height*3*8;
			*p_fb_size = (*p_fb_size + ((1<<20) -1)) & ~((1<<20) -1);
		}
    }
    else 
    {
        *p_fb_size = 32 << 20;   /* default 32MB FB */
    }

	return ret;
}

int act_pmem_init(void)
{
    /* fill gd->bd->cfg_ddr_size ... here. */

    uint32_t tmp_buf[4];
    uint32_t cfg_ddr_size, cfg_fb_size, cfg_gpu_size, cfg_ion_size; // in bytes.
    int ret;

    debug("%s: get pmem size...\n", __FUNCTION__);

    ret = act_xmlp_get_config(
            "pmem.ddr_size", (char *)tmp_buf, sizeof(uint32_t));
    if (ret == 0)
    {
        /* MB -> Byte */
        cfg_ddr_size = tmp_buf[0] << 20;
    }
    else
    {
        if(ACTS_AFINFO_PTR->ddr_param.dram_num != 0)
        {
            /* user has predefined a fixed DDR capacity, use that value. */
            cfg_ddr_size = ACTS_AFINFO_PTR->ddr_param.dram_num * 1024U * 1024U;
        }
        else
        {
            cfg_ddr_size = gd->ram_size;
        }
    }

    cfg_fb_size = 32 << 20; // default size
    ret = act_pmem_fb_size_init(&cfg_fb_size);
    if(ret != 0)
    {
        printf("%s: not support fb_size\n", __FUNCTION__);
        return -1;
    }

    ret = act_xmlp_get_config("pmem.gpu_size", (char *)tmp_buf, sizeof(uint32_t));
    if (ret == 0)
    {
        /* MB -> Byte */
        cfg_gpu_size = tmp_buf[0] << 20;
    }
    else
    {
        cfg_gpu_size = 0;
    }

    ret = act_xmlp_get_config("pmem.ion_size", (char *)tmp_buf, sizeof(uint32_t));
    if (ret == 0)
    {
        /* MB -> Byte */
        cfg_ion_size = tmp_buf[0] << 20;
    }
    else
    {
        if (cfg_ddr_size > (512 << 20))
        {
            cfg_ion_size = 160 << 20;   /* 160MB */
        }
        else if (cfg_ddr_size > (256 << 20))
        {
            cfg_ion_size = 102 << 20;   /* 102MB */
        }
        else
        {
            cfg_ion_size = 64 << 20;   /* 64MB */
        }
    }

    gd->bd->cfg_ddr_size = cfg_ddr_size;
    gd->bd->cfg_fb_size  = cfg_fb_size;
    gd->bd->cfg_gpu_size = cfg_gpu_size;
    gd->bd->cfg_ion_size = cfg_ion_size;

    debug("%s: ddr_size=%u fb_size=%u gpu_size=%u ion_size=%u\n",
            __FUNCTION__,
            cfg_ddr_size, cfg_fb_size, cfg_gpu_size, cfg_ion_size);

    return 0;
}

