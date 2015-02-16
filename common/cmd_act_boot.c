/*
 * (C) Copyright 2012
 * Actions Semi .Inc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* 目前的boot流程各个子过程之间的耦合过于紧密了, 拆开成多个命令就要很多的env变量作状态记录,
 * 命令的脚本也会很难写(有分支), 效率也保证不了, 这里全部统一成一个命令来实现.  */

#define DEBUG // TESTCODE

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <fs.h>

#include <asm/arch/actions_arch_common.h>
#include <asm/arch/actions_reg_atc2603.h>
#include <asm/arch/mbr_info.h>
#include <asm/arch/dvfs_level.h>

// from u-boot
extern int do_common_partn_read2(const char *dev_ifname, int dev, int part,
        void *addr, uint blk, uint cnt);
extern int act_afi_get_afinfo_from_blk_storage(const char *dev_ifname, int dev, afinfo_t *p_afinfo_buf);

// from board
extern int display_welcome(const char *dev_ifname, const char *dev_part_str, uint fstype);
extern int display_lpow(const char *dev_ifname, const char *dev_part_str, uint fstype);
extern int display_minicharge(const char *dev_ifname, const char *dev_part_str, uint fstype);
extern uint display_get_hdmi_status(void);
extern void display_exit(void);



DECLARE_GLOBAL_DATA_PTR;


// return: 0:OK  -1:no_card  -2:no_afinfo
static int _check_external_burn_card_and_load_afinfo(afinfo_t *p_afinfo_c)
{
    return act_afi_get_afinfo_from_blk_storage("mmc", 0, p_afinfo_c);
}

// return: 0-no_valid_card; 1-upgrade_card; -1-error;
static int _check_external_upgrade_card(void)
{
    /* try whole_disk first, then partitions */
    static const char *dev_part_str_tbl[] = {"0:0", "0:1", "0:2"};

    const char *dev_ifname, *dev_part_str, *upg_filename;
    uint i;
    int ret;

    dev_ifname = "mmc";
    upg_filename = "update.zip";

    for(i=0; i<ARRAY_SIZE(dev_part_str_tbl); i++)
    {
        dev_part_str = dev_part_str_tbl[i];
        ret = fs_set_blk_dev(dev_ifname, dev_part_str, FS_TYPE_ANY);
        if(ret != 0)
        {
            printf("%s: probe card fs err, ret=%d dev=(%s %s)\n",
                    __FUNCTION__, ret, dev_ifname, dev_part_str);
            continue;
        }
        ret = fs_exists(upg_filename);
        if(ret)
        {
            // found.
            debug("%s: found \"%s\" on %s %s\n",
                    __FUNCTION__, upg_filename, dev_ifname, dev_part_str);
            return 1;
        }
    }

    debug("%s: no \"%s\" found on %s %s\n",
            __FUNCTION__, upg_filename, dev_ifname, dev_part_str);
    return 0;
}

static void _update_afinfo_ddr_delay_param(afinfo_t * p_afinfo)
{
    uint val;

    if(p_afinfo->ddr_param.ddr_auto_scan & 0x1U)
    {
        p_afinfo->ddr_param.ddr_auto_scan &= ~0x1U;

        /*ddr auto scan*/
        val = act_readl(DCU_CLK_DLY) & 0xff;
        debug("%s: wr_dqs/clk_dqs=0x%08x old=0x%08x\n",
                __FUNCTION__, val, p_afinfo->ddr_param.wr_dqs);
        p_afinfo->ddr_param.wr_dqs = (unsigned char)val; //wr_dqs,clk_dqs

        val = act_readl(DCU_RD_DQS_DLY) & 0xff;
        debug("%s: rd_dqs=0x%08x old=0x%08x\n",
                __FUNCTION__, val, p_afinfo->ddr_param.rd_dqs);
        p_afinfo->ddr_param.rd_dqs = (unsigned char)val;//rd_dqs

        val = gd->ram_size >> 20;
        debug("%s: dram_cap=%uMB old=%uMB\n",
                __FUNCTION__, val, p_afinfo->ddr_param.dram_cap);
        p_afinfo->ddr_param.dram_cap = (unsigned short)val;
        debug("%s: dram_num=%u\n",
                __FUNCTION__, p_afinfo->ddr_param.dram_num);
    }
}

static void _merge_card_afinfo(afinfo_t * p_crd_afinfo, afinfo_t const *p_src_afinfo)
{
    uint tmp;
    /*use nand/emmc ddr para for cardburn*/
    if(p_crd_afinfo->ddr_param.ddr_auto_scan & 0x01)
    {
        tmp = p_src_afinfo->ddr_param.ddr_auto_scan ;
        memcpy(&p_crd_afinfo->ddr_param, &p_src_afinfo->ddr_param, sizeof(ddr_param_t));
        p_crd_afinfo->ddr_param.ddr_auto_scan = tmp;
    }
}

static void _reconfig_onoff_reset_time(void)
{
#define ONOFF_RESET_TIME_SEL0 6
#define ONOFF_RESET_TIME_SEL1 8
#define ONOFF_RESET_TIME_SEL2 10
#define ONOFF_RESET_TIME_SEL3 12

    int ret;
    uint dat, onoff_reset_time_sel=0;
    unsigned int onoff_reset_time;

    ret = act_xmlp_get_config(
            "onff.reset_time", (char *)(&onoff_reset_time), sizeof(unsigned int));
    if(ret != 0)
    {
        return ;
    }
    if(onoff_reset_time > ONOFF_RESET_TIME_SEL2)
    {
        onoff_reset_time_sel = 3;
    }
    else if (onoff_reset_time > ONOFF_RESET_TIME_SEL1)
    {
        onoff_reset_time_sel = 2;
    }
    else if (onoff_reset_time > ONOFF_RESET_TIME_SEL0)
    {
        onoff_reset_time_sel = 1;
    }

    dat = act_260x_reg_read(atc2603_PMU_SYS_CTL2);
    dat &= (~(0x3<<7));
    dat |= (onoff_reset_time_sel<<7);
    act_260x_reg_write(atc2603_PMU_SYS_CTL2, dat);

    return;
}

struct act_bootsrc_opt
{
    char const *boot_dev_ifname;
    char const *boot_dev_partn_str;
    uint boot_dev_fstype;
    char const *boot_img_filename;
    char const *boot_img_filename2;
    uint8_t need_restore_afinfo;
    uint8_t disable_qb;
};

static const struct act_bootsrc_opt s_bootsrc_opt_ext_recovery =
{
    .boot_dev_ifname = "mmc",
    .boot_dev_partn_str = "0:2", // recovery
    .boot_dev_fstype = FS_TYPE_FAT,
    .boot_img_filename = "recovery.img",
    .boot_img_filename2 = "rec_bak.img",
    .need_restore_afinfo = 1,
    .disable_qb = 1
};
static const struct act_bootsrc_opt s_bootsrc_opt_normal =
{
    .boot_dev_ifname = NULL,
    .boot_dev_partn_str = NULL, // misc
    .boot_dev_fstype = FS_TYPE_FAT,
    .boot_img_filename = "boot.img",
    .boot_img_filename2 = NULL,
    .need_restore_afinfo = 0,
    .disable_qb = 0
};
static const struct act_bootsrc_opt s_bootsrc_opt_recovery =
{
    .boot_dev_ifname = NULL,
    .boot_dev_partn_str = NULL, // misc
    .boot_dev_fstype = FS_TYPE_FAT,
    .boot_img_filename = "recovery.img",
    .boot_img_filename2 = "rec_bak.img",
    .need_restore_afinfo = 0,
    .disable_qb = 1
};

static int do_act_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    uint boot_mode, boot_ext_src;
    int ret, power_state;
    afinfo_t *p_afinfo_bak;

    boot_ext_src = 0;
    boot_mode = ACTS_BOOT_MODE_NORMAL;

    /* handle enter_ADFU request (by key_press, combo key mode)
     * enter_ADFU request by PMU_reg is handled in SPL */
    if(ACTS_AFINFO_PTR->boot_key_scan_mode == 0) //combo key mode ?
    {
        if(act_key_scan(ACTS_AFINFO_PTR->boot_key_scan_mode) == KEY_ADFU)
        {
            printf("%s: reset to enter ADFU\n", __FUNCTION__);
            // set enter ADFU flag & reset
            act_pmu_set_enter_adfu_rq_flag(1);
            reset_cpu(0);
        }
    }

    p_afinfo_bak = NULL;

    if(ACTS_AFINFO_PTR->boot_dev == ACTS_BOOTDEV_SD0)
    {
        /* if we boot from an external card, check ddr_param.ddr_auto_scan,
         * update AFINFO if needed. */
        _update_afinfo_ddr_delay_param((afinfo_t *)ACTS_AFINFO_PTR);
    }
    else
    {
        // We boot from internal storage ...
        // check if OS request enter recovery.
        if(act_pmu_chk_enter_recovery_rq_flag(1) == 1)
        {
            puts("OS request enter recovery ...\n");
            boot_mode = ACTS_BOOT_MODE_RECOVERY;
        }
        else
        {
            // normal boot from internal storage ...
            /* check if a boot-able or upgrade-able CARD exist in SD0 slot */
            afinfo_t *p_crd_afinfo = malloc(sizeof(afinfo_t));
            if(p_crd_afinfo == NULL)
            {
                printf("%s: no mem\n", __FUNCTION__);
                return -1;
            }

            puts("checking burn/upgrade card...\n");

            ret = _check_external_burn_card_and_load_afinfo(p_crd_afinfo);
            if(ret == 0)
            {
                puts("found burn card! try enter recovery on this card ...\n");

                // backup original AFINFO
                p_afinfo_bak = malloc(sizeof(afinfo_t));
                if(p_afinfo_bak == NULL)
                {
                    printf("%s: no mem\n", __FUNCTION__);
                    return -1;
                }
                memcpy(p_afinfo_bak, ACTS_AFINFO_PTR, sizeof(afinfo_t));

                /* the AFINFO in CARD need to be merged before use (DDR param ...)*/
                _merge_card_afinfo(p_crd_afinfo, p_afinfo_bak);

                // modify boot_dev, if we are booting from internal MMC
                // the ACT kernel need this trick.
                if(p_crd_afinfo->boot_dev == ACTS_BOOTDEV_SD2)
                {
                    // card-to-card upgrade
                    p_crd_afinfo->boot_dev = ACTS_BOOTDEV_SD02SD2;
                }

                /* replace current AFINFO with the one in external card. */
                memcpy(gd->arch.p_afinfo, p_crd_afinfo, sizeof(afinfo_t));

                boot_ext_src = 1;
                boot_mode = ACTS_BOOT_MODE_RECOVERY;
            }
            else if(ret == -2) /* not burn card */
            {
                ret = _check_external_upgrade_card();
                if(ret > 0)
                {
                    puts("found upgrade card! try enter recovery ...\n");

                    /* upgrade CARD (has update.zip) */
                    boot_ext_src = 0;
                    boot_mode = ACTS_BOOT_MODE_RECOVERY;
                }
            }

            free(p_crd_afinfo);
        }
    }

    while(1) // for retry.
    {
        struct act_bootsrc_opt bsrc_opt;

        // select bootsrc_opt
        if(boot_ext_src != 0)
        {
            bsrc_opt = s_bootsrc_opt_ext_recovery;
        }
        else
        {
            if(boot_mode == ACTS_BOOT_MODE_RECOVERY)
            {
                bsrc_opt = s_bootsrc_opt_recovery;

                if(ACTS_AFINFO_PTR->boot_dev == ACTS_BOOTDEV_NAND)
                {
                    bsrc_opt.boot_dev_ifname = "nandblk";
                    bsrc_opt.boot_dev_partn_str = "0:2"; // recovery
                }
                else
                {
                    bsrc_opt.boot_dev_ifname = "mmc";
                    bsrc_opt.boot_dev_partn_str = "2:2"; // recovery
                }
            }
            else
            {
                bsrc_opt = s_bootsrc_opt_normal;

                if(ACTS_AFINFO_PTR->boot_dev == ACTS_BOOTDEV_NAND)
                {
                    bsrc_opt.boot_dev_ifname = "nandblk";
                    bsrc_opt.boot_dev_partn_str = "0:3"; // misc
                }
                else
                {
                    bsrc_opt.boot_dev_ifname = "mmc";
                    bsrc_opt.boot_dev_partn_str = "2:3"; // misc
                }
            }
        }
        printf("%s: try to boot %s %s %s\n", __FUNCTION__,
                bsrc_opt.boot_dev_ifname,
                bsrc_opt.boot_dev_partn_str,
                bsrc_opt.boot_img_filename);

        /* load & parse XML (front) */
        ret = act_xmlp_init();
        if(ret != 0)
        {
            printf("%s: can not init XML parser\n", __FUNCTION__);
            break;
        }
        ret = act_xmlp_boot_parse_f(
                bsrc_opt.boot_dev_ifname,
                bsrc_opt.boot_dev_partn_str,
                bsrc_opt.boot_dev_fstype);
        if(ret != 0)
        {
            printf("%s: can not parse XML (front)\n", __FUNCTION__);
            goto label_boot_failed;
        }

        /* do misc init that require XML_config_db */
        _reconfig_onoff_reset_time();
        ret = act_pmem_init();
        if(ret != 0)
        {
            printf("%s: _pmem_init failed\n", __FUNCTION__);
            goto label_boot_failed;
        }

        /* get power state (check battery level) */
        power_state = 0; // normal
        if(ACTS_AFINFO_PTR->battery_ignore == 0)
        {
            power_state = act_pmu_check_power_state();

            // check "disable_charger_report" flag
            if(power_state > 0)
            {
                ret = act_pmu_chk_dis_enter_charger_rq_flag(1);
                if(ret != 0 || boot_mode == ACTS_BOOT_MODE_RECOVERY)
                {
                    // no charger.
                }
                else
                {
                    boot_mode = ACTS_BOOT_MODE_CHARGER;
                }
            }
        }

        // if low power, show low_power LOGO & shutdown
        // if changing, show mini_charge LOGO & set bootmode=mini_charge
        // else, show normal LOGO.

        /* display LOGO (normal or low_battery or mini_charge) */
        {
            char *hdmi_state = NULL;

            // call show LOGO here...
            ret = 0;
            switch(boot_mode)
            {
            case ACTS_BOOT_MODE_NORMAL :
            case ACTS_BOOT_MODE_RECOVERY :
                if(power_state < 0)
                {
                    ret = display_lpow(bsrc_opt.boot_dev_ifname,
                            bsrc_opt.boot_dev_partn_str, bsrc_opt.boot_dev_fstype);
                }
                else
                {
                    ret = display_welcome(bsrc_opt.boot_dev_ifname,
                            bsrc_opt.boot_dev_partn_str, bsrc_opt.boot_dev_fstype);
                }
                break;
            case ACTS_BOOT_MODE_CHARGER:
                ret = display_minicharge(bsrc_opt.boot_dev_ifname,
                        bsrc_opt.boot_dev_partn_str, bsrc_opt.boot_dev_fstype);
                break;
            }

            hdmi_state = "lowfreq=0";  // by default, no HDMI.
            if(ret != 0)
            {
                printf("%s: failed to show LOGO\n", __FUNCTION__);
            }
            else
            {
                if(display_get_hdmi_status() != 0)
                {
                    hdmi_state = "lowfreq=1";
                }
            }
            act_append_to_bootarg_env(hdmi_state);
        }

        // if low_battery, shutdown ...
        if(power_state < 0)
        {
            printf("LOW BATTERY!!! shutoff...\n");
            mdelay(3000);
            act_pmu_shutoff();
        }

        /* handle key after show LOGO, for KEY_ADFU / KEY_RECOVERY  */
        {
            switch(act_key_scan(ACTS_AFINFO_PTR->boot_key_scan_mode))
            {
            case KEY_ADFU:
                printf("%s: reset to enter ADFU\n", __FUNCTION__);
                // set enter ADFU flag & reset
                act_pmu_set_enter_adfu_rq_flag(1);
                reset_cpu(0);
                break;
            case KEY_RECOVERY:
                if(boot_ext_src == 0 && boot_mode != ACTS_BOOT_MODE_RECOVERY)
                {
                    boot_mode = ACTS_BOOT_MODE_RECOVERY;
                    continue; // need retry
                }
            }
        }

        /* load & parse XML (rear) */
        ret = act_xmlp_boot_parse_r(
                bsrc_opt.boot_dev_ifname,
                bsrc_opt.boot_dev_partn_str,
                bsrc_opt.boot_dev_fstype);
        if(ret != 0)
        {
            printf("%s: can not parse XML (rear)\n", __FUNCTION__);
            goto label_boot_failed;
        }

        /* update gd->arch.boot_mode */
        gd->arch.boot_mode = boot_mode;

        /* load & boot kernel */
        ret = act_boot_krnl(
                bsrc_opt.boot_dev_ifname,
                bsrc_opt.boot_dev_partn_str,
                bsrc_opt.boot_dev_fstype,
                bsrc_opt.boot_img_filename,
                bsrc_opt.disable_qb);
        if(bsrc_opt.boot_img_filename2 != NULL)
        {
            printf("%s: act_boot_krnl return %d, try another img\n",
                    __FUNCTION__, ret);
            ret = act_boot_krnl(
                    bsrc_opt.boot_dev_ifname,
                    bsrc_opt.boot_dev_partn_str,
                    bsrc_opt.boot_dev_fstype,
                    bsrc_opt.boot_img_filename2,
                    bsrc_opt.disable_qb);
        }
        printf("%s: act_boot_krnl return %d, boot failed\n",
                __FUNCTION__, ret);

        label_boot_failed: ;

        /* boot failed. */
        if(bsrc_opt.need_restore_afinfo)
        {
            memcpy((afinfo_t*)ACTS_AFINFO_PTR, p_afinfo_bak, sizeof(afinfo_t));
        }

        // try next ...
        if(boot_ext_src != 0)
        {
            if(ACTS_AFINFO_PTR->boot_dev == ACTS_BOOTDEV_SD0)
            {
                break; // no more choice.
            }
            boot_ext_src = 0;
            boot_mode = ACTS_BOOT_MODE_NORMAL;
        }
        else
        {
            if(boot_mode != ACTS_BOOT_MODE_RECOVERY)
            {
                boot_mode = ACTS_BOOT_MODE_RECOVERY;
            }
            else
            {
                break; // no more choice.
            }
        }
    }

    if(p_afinfo_bak != NULL)
    {
        free(p_afinfo_bak);
    }

    display_exit();

    printf("%s: no valid bootable config, boot failed.\n", __FUNCTION__);
    return -1;
}

U_BOOT_CMD(act_boot, CONFIG_SYS_MAXARGS, 0, do_act_boot,
        "boot procedure for actions",
        "");
