#include <common.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/actions_reg_atc2603.h>

#include "atc260x.h"


/*atc260x_get_bat_voltage
 * volt: return bat voltage value
 * return : 0 success , others :fail
 */
static int _atc260x_get_bat_voltage(int * volt)
{
    int ret =  0;
    int break_charge = 0;

    ret = act_260x_reg_read(atc2603_PMU_CHARGER_CTL0);

    if (ret & PMU_CHARGER_CTL0_ENCH)
    {
        break_charge = 1;
    }

    if (break_charge)
    {
        act_260x_reg_setbits(atc2603_PMU_CHARGER_CTL0,
            PMU_CHARGER_CTL0_ENCH, 0);
    }

    /* wait 100ms */
    mdelay(100);

    ret = act_260x_reg_read(atc2603_PMU_AuxADC_CTL0);
    /* if current status is disabled, enable it and wait 1ms for translate. */
    if ((ret & (1 << ATC260X_AUX_BATV)) == 0)
    {
        act_260x_reg_write(atc2603_PMU_AuxADC_CTL0, 1 << ATC260X_AUX_BATV);
        mdelay(1);
    }

    ret = act_260x_reg_read(atc2603_PMU_BATVADC);
    *volt = ret * 2930 * 2 / 1000;

    if (break_charge)
    {
        act_260x_reg_setbits(atc2603_PMU_CHARGER_CTL0,
            PMU_CHARGER_CTL0_ENCH, PMU_CHARGER_CTL0_ENCH);
    }

    return 0;
}


#define PMU_CHARGER_CTL1_BAT_EXIST_EN       (1 << 5)
#define PMU_CHARGER_CTL1_BAT_EXIST          (1 << 10)
#define ATC260X_BAT_DETECT_DELAY_US         300*1000
#define SPI_WAIT_TIME_US                    200

static int _check_battery_exist_hw(void)
{
    int ret;
    int exist,bat_v;
    int timeout_cnt = 5, i;
    int cnt = 1000;

    /* dectect bit 0 > 1 to start dectecting */
    for(i=0; i<timeout_cnt; i++)
    {
        act_260x_reg_setbits(atc2603_PMU_CHARGER_CTL1,
                PMU_CHARGER_CTL1_BAT_EXIST_EN, PMU_CHARGER_CTL1_BAT_EXIST_EN);

        udelay(SPI_WAIT_TIME_US);
        if((act_260x_reg_read(atc2603_PMU_CHARGER_CTL1) & PMU_CHARGER_CTL1_BAT_EXIST_EN) != 0)
        {
            break;
        }
    }
    if(i == timeout_cnt)
    {
        debug("%s: SPI can not write\n", __FUNCTION__);
    }

    /* wait bat detect over */
    udelay(ATC260X_BAT_DETECT_DELAY_US);

    do
    {
        if((act_260x_reg_read(atc2603_PMU_CHARGER_CTL1) & PMU_CHARGER_CTL1_BAT_DT_OVER) != 0)
        {
            break;
        }
        else
        {
            //debug("bat detect not finished\n");
            udelay(1000);
            cnt--;
        }
    }while(cnt > 0);
    if(cnt == 0)
    {
        debug("%s: bat detect timeout\n", __FUNCTION__);
        return -1;
    }

    ret = act_260x_reg_read(atc2603_PMU_CHARGER_CTL1);
    exist = (ret & PMU_CHARGER_CTL1_BAT_EXIST);

    /* cleare battery detect bit, otherwise cannot charge */
    for(i=0; i<timeout_cnt; i++)
    {
        act_260x_reg_setbits(atc2603_PMU_CHARGER_CTL1, PMU_CHARGER_CTL1_BAT_EXIST_EN, 0);

        udelay(SPI_WAIT_TIME_US);
        if((act_260x_reg_read(atc2603_PMU_CHARGER_CTL1) & PMU_CHARGER_CTL1_BAT_EXIST_EN) == 0)
        {
            break;
        }
    }
    if(i == timeout_cnt)
    {
        debug("%s: SPI can not write\n", __FUNCTION__);
    }

    if(1)//exist)
    {
        _atc260x_get_bat_voltage(&bat_v);
        debug("%s: batv is:%d, pmu detect value is:0x%X\n",  __FUNCTION__, bat_v, exist);
        if (bat_v > 2000)
        {
            //return 1; //bat exsit
        }
    }
    if(exist)
    {
        return 1;
    }
    return 0;
}

int act_pmu_get_charge_plugin_status(int * status, int* wall, int* vbus)
{
    int ret = 0;
    int wall_mv = 0 ,vbus_mv = 0;

    ret = act_260x_reg_read(atc2603_PMU_AuxADC_CTL0);
    /* if current status is disabled, enable it and wait 1ms for translate. */
    if ((ret & (2 << 10)) == 0)
    {
        act_260x_reg_write(atc2603_PMU_AuxADC_CTL0, 2 << 10);
        mdelay(1);
    }

    ret = act_260x_reg_read(atc2603_PMU_WALLVADC);
    wall_mv = (ret * 2930 * 2 + ret * 2930 / 2) / 1000;

    ret = act_260x_reg_read(atc2603_PMU_VBUSVADC);
    vbus_mv = (ret * 2930 * 2 + ret * 2930 / 2) / 1000;

    if((wall_mv>=3000) || (vbus_mv>=3000))
    {
        *status = 1; // charge active
    }
    else
    {
        *status = 0; // no change
    }

    *wall = wall_mv;
    *vbus = vbus_mv;

    return 0;
}

static int _get_soft_battery_cap(int* cap)
{
    static const unsigned int unloaded_table_bat_val2percentage[21][2] =
    {
        {3500,0},
        {3600,5},
        {3675,10},
        {3700,15},
        {3725,20},
        {3750,25},
        {3775,30},
        {3800,35},
        {3820,40},
        {3840,45},
        {3860,50},
        {3880,55},
        {3900,60},
        {3920,65},
        {3940,70},
        {3960,75},
        {3980,80},
        {4000,85},
        {4020,90},
        {4040,95},
        {4100,100},
    };

    /*???§?§?|¨??¨¤???vbat |¨???1?¨o???è|¨??????§?|?§?§Y?§?é?*/
    int vbat, ret, len, i;
    unsigned short cap_stored = 0, tmp=0;
    unsigned short cap_mapped = 0;
    int is_first = 0;

    ret = _atc260x_get_bat_voltage(&vbat);
    if(ret != 0)
    {
        return ret;
    }

    /*read battery cap stored in PMU_SYS_CTL9 bit[15:8]*/
    tmp = (act_260x_reg_read(atc2603_PMU_SYS_CTL9) & 0xff00)>>8;

    if((tmp & (0x1<<7)) == 0)
    {
        /*is first boot*/
        is_first = 1;
    }
    else
    {
        is_first = 0;
        cap_stored = (tmp & (~(0x1<<7)));
    }

    len = ARRAY_SIZE(unloaded_table_bat_val2percentage);
    for(i=0; i<len; i++)
    {
        if(vbat <= unloaded_table_bat_val2percentage[i][0])
        {
            cap_mapped = unloaded_table_bat_val2percentage[i][1];
            break;
        }
    }

    if(i==len)
    {
        cap_mapped = 100;
    }

    if(is_first)
    {
        *cap = cap_mapped;
    }
    else
    {
        *cap = min(cap_stored, cap_mapped);
    }

    return 0;
}

/* only call this function after XML config loaded. */
/* return :  0:normal, -1:batt_low, 1:charge */
int act_pmu_check_power_state(void)
{
    int charge_plugin_status=0;
    int status = 0;
    int bin_cfg_lpow = 0;
    int gauge_mode = 0;
	int adaptor_type = 0;
    int low_power_boot_choice = 0;
    int ret;
    int wall_mv=0, vbus_mv=0;

    act_pmu_get_charge_plugin_status(&charge_plugin_status, &wall_mv, &vbus_mv);
    debug("%s: charge_plugin_status: %d, wall_mv:%d, vbus_mv:%d\n",
            __FUNCTION__, charge_plugin_status, wall_mv, vbus_mv);

	act_xmlp_get_config("charge.support_adaptor_type", (char *)&adaptor_type, sizeof(adaptor_type));
    debug("adaptor_type:~~%d~~\n", adaptor_type);
	
	if ( adaptor_type == 1 ) 
	{
		act_260x_reg_setbits(atc2603_PMU_SYS_CTL5, (1<<8), 0); // 关闭VBUS 唤醒源		
		mdelay(10);
		act_260x_reg_setbits(atc2603_PMU_SYS_CTL0, (1<<15), 0); // 关闭VBUS 唤醒源
	    mdelay(10);
    	debug("%s: ctl0=%x,ctl5=0x%x\n", __FUNCTION__,
            atc260x_reg_read(atc2603_PMU_SYS_CTL0), atc260x_reg_read(atc2603_PMU_SYS_CTL5));	
	}

    /*
    * 2013-9-16 config pwm2 before check power, so as to avoid detect problem when
    * battery enter a state of self-proection.
    */
    act_writel(0x1000, CMU_PWM2CLK); //set pwm div
    /*enable pwm2 clk*/
    act_writel(act_readl(CMU_DEVCLKEN1) | (0x1<<25), CMU_DEVCLKEN1);
    act_writel(act_readl(PWM_CTL2) | (63 << 19)  , PWM_CTL2);   //setup duty cycle


    _atc260x_get_bat_voltage(&status);
    debug("%s: In check_power,batv is:%d\n", __FUNCTION__, status);
    if (status < 3000 && adaptor_type != 1)
    {
        debug("%s: battery is too low, hw detect\n", __FUNCTION__);
        ret = _check_battery_exist_hw();
        if(ret <= 0)
        {
            debug("%s: no battery? boot normally!\n", __FUNCTION__);
            return 0;
        }
    }

    if(charge_plugin_status == 1)
    {
        debug("%s: ~~~~~~~~~~~~charge is plugin\n", __FUNCTION__);

        low_power_boot_choice = act_260x_reg_read(atc2603_PMU_UV_INT_EN) & 1;
        debug("%s: low_power_boot_choice is %d\n", __FUNCTION__, low_power_boot_choice);

        if(low_power_boot_choice == 1)
        {
            return 0;
        }
		if ( adaptor_type == 1 ) 
		{
			if  (wall_mv > 4000 )  // 有adeptor 才进minicharge
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}        
    }

	if ( (wall_mv > 4000) && (adaptor_type == 1) )  // 有adeptor 正常启动。
    {
        return 0;
    }

    gauge_mode = 0;
    act_xmlp_get_config("charge.gauge_mode", (char *)&gauge_mode, sizeof(gauge_mode));
    debug("%s: gauge_mode:~~%d~~\n", __FUNCTION__, gauge_mode);

    bin_cfg_lpow = 0;
    act_xmlp_get_config("charge.low_power", (char *)&bin_cfg_lpow, sizeof(bin_cfg_lpow));
    debug("%s: bin_cfg_lpow:~~%d~~\n", __FUNCTION__, bin_cfg_lpow);

    if(gauge_mode == 0)     /*soft gauge*/
    {
        ret = _get_soft_battery_cap(&status);

        debug("%s: get_soft_battery_cap :~~%d~~\n", __FUNCTION__, status);
        if(ret != 0)
        {
            debug("%s: get_soft_battery_cap err:~~%d~~\n", __FUNCTION__, ret);
        }
    }
    else
    {
        //ret = check_battery(&status);
        ret = _get_soft_battery_cap(&status);
        printf("%s: !!!!!!!!!! gauge_check_battery() not implement!!!!!!!!!!\n",
                __FUNCTION__);

        debug("%s: check_battery :~~%d~~\n", __FUNCTION__, status);
        if(ret != 0)
        {
            debug("%s: check_battery err:~~%d~~\n", __FUNCTION__, ret);
        }
    }

    if(status < bin_cfg_lpow)
    {
        debug("%s: ~~~~~~~~~~~~low power\n", __FUNCTION__);
        return -1;
    }
    return 0;
}
