/* 因为用到了QuickBoot, 以及有XML的依赖, 这里不直接使用bootm命令来加载内核,
 * 而是改为自己实现. */

#define DEBUG // TESTCODE

#include <common.h>
#include <malloc.h>
#include <android_image.h>
#include <sha1.h>
#include <fs.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/setup.h>
#include <asm/arch/actions_arch_common.h>
#include <asm/arch/mbr_info.h>
#include <asm/arch/dvfs_level.h>

#include "xml_parser/xml_parser.h"
#include "xml_parser/xml_get_cfg.h"

#ifdef CONFIG_SERIAL_TAG
#error "Do not define CONFIG_SERIAL_TAG, we use ourself's"
#endif
#ifdef CONFIG_CMDLINE_TAG
#error "Do not define CONFIG_CMDLINE_TAG, we use ourself's"
#endif
#ifdef CONFIG_SETUP_MEMORY_TAGS
#error "Do not define CONFIG_SETUP_MEMORY_TAGS, we use ourself's"
#endif

// from miscinfo module (*.a)
extern int ReadStorage(int type, char * buf, int size);

// from u-boot
extern int do_common_partn_read2(const char *dev_ifname, int dev, int part,
        void *addr, uint blk, uint cnt);
extern int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images);


/* Pointer to the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;


static char *_strcat_bootarg(char *pbuf, char const *pnew)
{
    uint len;
    len = strlen(pbuf);
    if(len == 0)
    {
        strcpy(pbuf, pnew);
    }
    else
    {
        if(pbuf[len-1] != ' ')
        {
            pbuf[len] = ' ';
            pbuf[len+1] = 0;
        }
        strcat(pbuf, pnew);
    }
    return pbuf;
}

int act_append_to_bootarg_env(char const *p_new)
{
    char *p_cmdline_buf, *p_old_cmdline;
    int ret;

    if(p_new == NULL || *p_new == 0)
        return -1;

    p_old_cmdline = getenv("bootargs");
    if(p_old_cmdline != NULL)
    {
        p_cmdline_buf = malloc(strlen(p_old_cmdline) + strlen(p_new) + 4);
        if(p_cmdline_buf == NULL)
        {
            printf("%s: no mem\n", __FUNCTION__);
            return -1;
        }
        strcpy(p_cmdline_buf, p_old_cmdline);
        _strcat_bootarg(p_cmdline_buf, p_new);
        ret = setenv("bootargs", p_cmdline_buf);
        free(p_cmdline_buf);
    }
    else
    {
        ret = setenv("bootargs", p_new);
    }
    if(ret != 0)
    {
        printf("%s: failed to append \"%s\" to bootarg env\n",
                __FUNCTION__, p_new);
    }
    return ret;
}


struct act_swsusp_header
{
    char reserved[4096U - 20 - sizeof(u64) - sizeof(int) - sizeof(unsigned int) -
                  sizeof(u32)];
    u32 crc32;
    u64 image;
    int contin_pfn_start;
    unsigned int flags; /* Flags to pass to the "boot" kernel */
    char    orig_sig[10];
    char    sig[10];
} __attribute__((packed));

// return: !=0 - error or no quick boot,  0 - quick boot activated
static int _chk_and_prepare_quickboot(const char *dev_ifname, const char *dev_part_str, int fstype,
        uint32_t *p_qb_krnl_load_offset)
{
    struct act_swsusp_header *p_sp_hdr;
    char const *p_qb_cmdline;
    int ret, storage_dev;

    if(gd->arch.boot_mode != ACTS_BOOT_MODE_NORMAL)
    {
        return 1;
    }

    p_sp_hdr = memalign(ARCH_DMA_MINALIGN,
            (sizeof(struct act_swsusp_header) + 511) & ~511U);
    if(p_sp_hdr == NULL)
    {
        printf("%s: no mem\n", __FUNCTION__);
        return -1;
    }

    // check QuickBoot signature
    // note that in u-boot, partition number start from #1.
    storage_dev = (int)simple_strtoul(dev_part_str, NULL, 16);
    ret = do_common_partn_read2(dev_ifname, storage_dev, ACTS_LOGIC_PARTN_SWSUSP +1,
            p_sp_hdr, 0, (sizeof(struct act_swsusp_header) + 511) / 512U);
    if(ret != 0)
    {
        printf("%s: failed to read susp_partn %u\n",
                __FUNCTION__, ACTS_LOGIC_PARTN_SWSUSP);
        free(p_sp_hdr);
        return -1;
    }
    if(memcmp(p_sp_hdr->sig, "S1SUSPEND", sizeof(p_sp_hdr->sig)) != 0)
    {
        printf("%s: no quick boot\n", __FUNCTION__);
        free(p_sp_hdr);
        return 1;
    }
    *p_qb_krnl_load_offset = p_sp_hdr->contin_pfn_start * 4096;
    free(p_sp_hdr);

    // append quick_boot kernel cmdline
    ret = file_exists(dev_ifname, dev_part_str, "boot.prt", fstype);
    if(ret == 0)
    {
        p_qb_cmdline = "androidboot.mode=quickboot governor=performance";
    }
    else
    {
        p_qb_cmdline = "loglevel=3 androidboot.mode=quickboot governor=performance";
    }
    ret = act_append_to_bootarg_env(p_qb_cmdline);
    if(ret != 0)
    {
        return ret;
    }

    return 0; // quick_boot activated.
}

static char * _append_cmdline_str(char *pbuf, char const *p_new_cmdline)
{
    BUG_ON(pbuf==NULL || *pbuf!=0);
    if(*(pbuf-1) != ' ' && *p_new_cmdline != ' ')
    {
        *pbuf++ = ' ';
    }
    while ((*pbuf++ = *p_new_cmdline++) != '\0');
    return pbuf -1U;
}

/* note: buffer at lease 32 bytes */
static int _get_sn_string(char *pbuf, uint buf_size)
{
    uint __maybe_unused i, __maybe_unused badsnflag;
    int __maybe_unused ret;

    if(buf_size < 32U)
    {
        printf("%s: buffer too small\n", __FUNCTION__);
        return -1;
    }
    memset(pbuf, 0, buf_size);

#if defined(CONFIG_ACTS_FOR_BOOT) && defined(CONFIG_ACTS_STORAGE_NAND)
    /* only NAND have miscinfo functions */
    badsnflag = 1;
    if(ACTS_AFINFO_PTR->boot_dev == ACTS_BOOTDEV_NAND)
    {
        ret = ReadStorage(0, pbuf, 16); // should no more than 32
        if(ret > 0)
        {
            if(strlen(pbuf) >= 16)
            {
                badsnflag = 0;

                for(i=0; i < 16; i++)
                {
                    if((pbuf[i] >= '0' && pbuf[i] <= '9') ||
                            (pbuf[i] >= 'A' && pbuf[i] <= 'F'))
                    {
                        continue;
                    }
                    else if((pbuf[i] >= 'a' && pbuf[i] <= 'f'))
                    {
                        pbuf[i] = pbuf[i] - 32;
                    }
                    else
                    {
                        badsnflag = 1;
                        break;
                    }
                }
            }
        }
    }
    if(badsnflag == 0)
    {
        debug("%s: got valid sn from miscinfo\n", __FUNCTION__);
    }
    else
#endif
    {
        if(strlen((char*)(ACTS_AFINFO_PTR->sn)) >= 16)
        {
            memcpy(pbuf, ACTS_AFINFO_PTR->sn, 16);
            pbuf[0] = 'A';    //sn:A0xxxxxxxxxxxxxx
            pbuf[1] = '0';    //use random one
            puts("no valid sn in miscinfo, use sn data in afi\n");
        }
        else
        {
            puts("no valid sn found\n");
            return -1;
        }
    }

    return 0;
}

static int _collect_aux_krnl_cmdline(char *p_cmdline_buf)
{
    char tmp_buffer[128];
    char *p_buf, *p_curr;
    int ret;

    p_buf = malloc(64 * 1024);
    if(p_buf == NULL)
    {
        printf("%s: mxml no mem\n", __FUNCTION__);
        return -1;
    }
    p_buf[0] = ' ';
    p_buf[1] = 0;
    p_curr = p_buf + 1;

    // console
    {
        struct uart_config const *p_uart_cfg = &(ACTS_AFINFO_PTR->uart_config);

        if (p_uart_cfg->enable)
        {
            sprintf(tmp_buffer,  "console=ttyS%u,%lu earlyprintk loglevel=%u",
                    p_uart_cfg->chan, p_uart_cfg->baudrate, p_uart_cfg->loglevel);
            p_curr = _append_cmdline_str(p_curr, tmp_buffer);
        }
        else
        {
            p_curr = _append_cmdline_str(p_curr, "console=ttyS5,115200 earlyprintk");
        }
    }

    // dual logo
    {
        uint32_t logo_change = 0;

        ret = act_xmlp_get_config("dual_logo.support", (char *)(&logo_change), sizeof(logo_change));
        if(ret == 0 && logo_change == 1)
        {
            debug("%s: dual_logo.support is:%d\n", __FUNCTION__, logo_change);
            p_curr = _append_cmdline_str(p_curr, "androidboot.supportduallogo=1");
        }
        else
        {
            p_curr = _append_cmdline_str(p_curr, "androidboot.supportduallogo=0");
        }
    }

    // selinux
    {
        int32_t selinux_enable = 0;
        ret = act_xmlp_get_config("selinux.enable", (char *)(&selinux_enable), sizeof(unsigned int));
        if(ret != 0 || selinux_enable == 0)
        {
            p_curr = _append_cmdline_str(p_curr, "selinux=0");
        }
    }

    // boot mode
    {
        switch(gd->arch.boot_mode)
        {
        case ACTS_BOOT_MODE_NORMAL:
            // quick_boot cmdline is already attached.
            // do nothing.
            break;
        case ACTS_BOOT_MODE_CHARGER:
            p_curr = _append_cmdline_str(p_curr, "androidboot.mode=charger");
            break;
        case ACTS_BOOT_MODE_RECOVERY:
            if(ACTS_AFINFO_PTR->boot_key_scan_mode == 0)
            {
                p_curr = _append_cmdline_str(p_curr, "androidboot.mode=recovery1");
            }
            else
            {
                p_curr = _append_cmdline_str(p_curr, "androidboot.mode=recovery2");
            }
        }
    }

    // hdmi states
    // append "lowfreq=1" (in case HDMI is active) or "lowfreq=0" (HDMI not active),
    // already done in show-LOGO stage.

    // serial_number ...
    {
        uint tmplen;

        memset(tmp_buffer, 0, sizeof(tmp_buffer));
        strcpy(tmp_buffer, "androidboot.serialno=");
        tmplen = strlen("androidboot.serialno=");
        ret = _get_sn_string(tmp_buffer + tmplen, sizeof(tmp_buffer)-tmplen);
        if(ret == 0)
        {
            p_curr = _append_cmdline_str(p_curr, tmp_buffer);
        }
    }

    // boot_dev
    {
        strcpy(tmp_buffer, "androidboot.bootdev=");
        switch(ACTS_AFINFO_PTR->boot_dev)
        {
        case ACTS_BOOTDEV_NAND:
            strcat(tmp_buffer,  "nand");
            break;
        case ACTS_BOOTDEV_SD0:
        case ACTS_BOOTDEV_SD1:
        case ACTS_BOOTDEV_SD2:
        case ACTS_BOOTDEV_SD02SD2:
            strcat(tmp_buffer,  "sd");
            break;
        default:
            printf("%s: unknown boot_dev %u\n",
                    __FUNCTION__, ACTS_AFINFO_PTR->boot_dev);
            goto label_err;
        }
        p_curr = _append_cmdline_str(p_curr, tmp_buffer);
    }

    // dvfs level
    {
        sprintf(tmp_buffer,  "androidboot.dvfslevel=0x%x", ASOC_GET_IC(gd->arch.dvfs_level));
        p_curr = _append_cmdline_str(p_curr, tmp_buffer);
    }

    // atc260x spi bus
    {
        ret = act_pmu_helper_get_attached_spi_bus();
        if(ret >= 0)
        {
            sprintf(tmp_buffer,  "atc260x_spi=%u", ret);
            p_curr = _append_cmdline_str(p_curr, tmp_buffer);
        }
    }

    _strcat_bootarg(p_cmdline_buf, &p_buf[1]);
    free(p_buf);
    return 0;

    label_err:
    free(p_buf);
    return -1;
}

void setup_board_tags(struct tag **in_params, bootm_headers_t *images)
{
    struct tag *p_tag, *p_ret;

    // CORE MEMORY INITRD2 END is done outside.

    p_tag = *in_params;

    // ATAG_CMDLINE
    {
        char *p;
        char *p_buf = p_tag->u.cmdline.cmdline;

        // cmdline in boot.img
        *p_buf = 0;
        _strcat_bootarg(p_buf, (char*)images->cmdline_start);

        // cmdline in EVN
        p = getenv("bootargs");
        if(p != NULL)
        {
            _strcat_bootarg(p_buf, p);
        }

        // cmdline generated
        _collect_aux_krnl_cmdline(p_buf);

        // debug
        debug("%s: krnl cmdline: %s\n", __FUNCTION__, p_buf);

        if(*p_buf != 0) // empty str?
        {
            p_tag->hdr.tag = ATAG_CMDLINE;
            p_tag->hdr.size =
                    (sizeof (struct tag_header) + strlen(p_buf) + 1 + 4) >> 2;
            p_tag = tag_next (p_tag);
        }
    }

    // ATAG_XML & ATAG_PINCTRL
    {
        p_ret = act_xmlp_generate_xml_tags(p_tag);
        assert(p_ret != NULL);
        p_tag = p_ret;
    }

    // ATAG_DVFSLEVEL
    {
        p_tag->hdr.tag = ATAG_DVFSLEVEL;
        p_tag->hdr.size = tag_size(tag_dvfslevel);

        p_tag->u.dvfslevel.dvfslevel = gd->arch.dvfs_level;
        p_tag->u.dvfslevel.icversion = act_get_icversion();

        p_tag = tag_next(p_tag);
    }

    // ATAG_BOARD_OPTION
    {
        p_tag->hdr.tag = ATAG_BOARD_OPTION;
        p_tag->hdr.size = tag_size(tag_board_opt);

        p_tag->u.board_opt.board_opt = ACTS_AFINFO_PTR->board_opt.cur_opt;
        p_tag->u.board_opt.board_opt_flags =
                ACTS_AFINFO_PTR->board_opt.opt_flag[ACTS_AFINFO_PTR->board_opt.cur_opt];

        p_tag = tag_next(p_tag);
    }

    // ATAG_PMEM_INFO
    if(gd->bd->cfg_ddr_size || gd->bd->cfg_fb_size ||
            gd->bd->cfg_gpu_size || gd->bd->cfg_ion_size)
    {
        p_tag->hdr.tag = ATAG_PMEM_INFO;
        p_tag->hdr.size = tag_size(tag_pmem_info);

        p_tag->u.pmem_info.ddr_size = gd->bd->cfg_ddr_size;
        p_tag->u.pmem_info.fb_size  = gd->bd->cfg_fb_size;
        p_tag->u.pmem_info.gpu_size = gd->bd->cfg_gpu_size;
        p_tag->u.pmem_info.ion_size = gd->bd->cfg_ion_size;

        p_tag = tag_next(p_tag);
    }

    // ATAG_BOOT_DEV
    {
        p_tag->hdr.tag = ATAG_BOOT_DEV;
        p_tag->hdr.size = tag_size(tag_boot_dev);

        p_tag->u.boot_dev.boot_dev = ACTS_AFINFO_PTR->boot_dev;

        p_tag = tag_next(p_tag);
    }

    // ATAG_BOOT_AFINFO
    {
        p_tag->hdr.tag = ATAG_BOOT_AFINFO;
        p_tag->hdr.size = tag_size(tag_boot_afinfo);

        p_tag->u.afinfo.afinfo_buf_start = (uint)ACTS_AFINFO_PTR;
        p_tag->u.afinfo.afinfo_buf_len = AFINFO_LENGTH;

        p_tag = tag_next(p_tag);
    }

    *in_params = p_tag;

    //__asm__ __volatile__("b .\n\tnop":::"memory");
}

static int _chk_android_bootimg_hdr(struct andr_img_hdr *p_abhdr, uint hdr_size)
{
    if(memcmp(p_abhdr->magic, ANDR_BOOT_MAGIC, ANDR_BOOT_MAGIC_SIZE) != 0)
    {
        printf("%s: wrong magic\n", __FUNCTION__);
        return -1;
    }
    if(hdr_size < p_abhdr->page_size)
    {
        printf("%s: hdr too samll (%u)\n", __FUNCTION__, hdr_size);
        return -1;
    }
    if(p_abhdr->page_size < 2048U ||
        (p_abhdr->page_size & (p_abhdr->page_size -1U)) != 0) // power of 2?
    {
        printf("%s: page_size error (%u)\n", __FUNCTION__, p_abhdr->page_size);
        return -1;
    }
    if(p_abhdr->kernel_size == 0 || (p_abhdr->kernel_addr & 3U) != 0)
    {
        printf("%s: wrong krnl param\n", __FUNCTION__);
        return -1;
    }
    if(p_abhdr->ramdisk_size == 0 || (p_abhdr->ramdisk_addr & 3U) != 0)
    {
        printf("%s: wrong initramfs param\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

/* call from ADFU server */
int act_boot_krnl_from_adfus(void *p_boot_img_hdr,
        void *p_xml_config, void *p_xml_mfps, void *p_xml_gpios)
{
    struct andr_img_hdr *p_abhdr;
    bootm_headers_t bootm_hdr;
    int ret;

    if(p_boot_img_hdr==NULL || p_xml_config==NULL ||
            p_xml_mfps==NULL || p_xml_gpios==NULL)
    {
        printf("%s: arguments err\n", __FUNCTION__);
        goto label_err;
    }

    ret = act_xmlp_init();
    if(ret != 0)
    {
        goto label_err;
    }
    ret = act_xmlp_upgrade_parse_all(
            p_xml_config, p_xml_mfps, p_xml_gpios);
    if(ret != 0)
    {
        goto label_err;
    }

    p_abhdr = (struct andr_img_hdr *)p_boot_img_hdr;
    if(_chk_android_bootimg_hdr(p_abhdr, 8192) != 0)
    {
        goto label_err;
    }

    /* setup ATAG space */
    assert(gd->bd != NULL);
    if(gd->bd->bi_boot_params == 0)
    {
        if(p_abhdr->tags_addr != 0)
        {
            gd->bd->bi_boot_params = p_abhdr->tags_addr;
        }
        else
        {
            gd->bd->bi_boot_params = (ulong) malloc(128*1024);
        }
    }

    /* setup bootm_hdr */
    memset(&bootm_hdr, 0, sizeof(bootm_hdr));
    //bootm_hdr.ft_addr = NULL;
    //bootm_hdr.ft_len = 0;     // we do not use dft.
    bootm_hdr.rd_start = (ulong)(p_abhdr->ramdisk_addr);
    bootm_hdr.rd_end   = (ulong)(p_abhdr->ramdisk_addr) + p_abhdr->ramdisk_size;
    bootm_hdr.cmdline_start = (ulong)(p_abhdr->cmdline);
    bootm_hdr.cmdline_end   = (ulong)(p_abhdr->cmdline) + strlen(p_abhdr->cmdline);
    bootm_hdr.ep = (ulong)(p_abhdr->kernel_addr);

    do_bootm_linux(0, 0, NULL, &bootm_hdr);
    puts("do_bootm_linux return!\n");

    label_err:
    return -1;
}

static int _common_read_fs(const char *dev_ifname, const char *dev_part_str, uint fstype,
        const char *filename, uint read_offset, uint read_size, void *pbuf)
{
    int ret;

    debug("%s: dev=%s,%s fstype=%u filename=%s offset=%u size=%u pbuf=0x%p\n",
            __FUNCTION__, dev_ifname, dev_part_str, fstype, filename,
            read_offset, read_size, pbuf);

    ret = fs_set_blk_dev(dev_ifname, dev_part_str, fstype);
    if(ret != 0)
    {
        printf("%s: probe fs err, ret=%d dev_ifname=%s dev_part_str=%s fstype=%u rq_filename=%s\n",
                __FUNCTION__, ret, dev_ifname,  dev_part_str, fstype, filename);
        return ret;
    }
    ret = fs_read(filename, (ulong)pbuf, read_offset, read_size);
    if(ret <= 0)
    {
        printf("%s: read err, ret=%d dev_ifname=%s dev_part_str=%s fstype=%u rq_filename=%s\n",
                __FUNCTION__, ret, dev_ifname,  dev_part_str, fstype, filename);
    }
    return ret;
}

/* call from act_boot command */
int act_boot_krnl(const char *dev_ifname, const char *dev_part_str, uint fstype,
        const char *img_filename, uint disable_qb)
{
    bootm_headers_t bootm_hdr;
    struct andr_img_hdr *p_abhdr;
    uint32_t qb_krnl_loadoffset;
    uint32_t krnl_loadaddr, initramfs_loadaddr, atag_addr;
    int ret;

    /* 检查 QuickBoot. */
    qb_krnl_loadoffset = 0;
    if(! disable_qb)
    {
        ret = _chk_and_prepare_quickboot(dev_ifname, dev_part_str, fstype,
                &qb_krnl_loadoffset);
        if(ret == 0)
        {
            printf("%s: quick_boot activated\n", __FUNCTION__);
        }
        else
        {
            // We do not have quick_boot setup
            qb_krnl_loadoffset = 0;
        }
    }

    // load android image header
    p_abhdr = memalign(ARCH_DMA_MINALIGN, 4096);
    if(p_abhdr == NULL)
    {
        printf("%s: no mem\n", __FUNCTION__);
        return -1;
    }

    krnl_loadaddr = initramfs_loadaddr = atag_addr = 0;
    {
        uint krnl_offset, initramfs_offset, img_alignsize;

        // load android image header
        ret = _common_read_fs(dev_ifname, dev_part_str, fstype,
                img_filename, 0, 4096, p_abhdr);
        if(ret <= 0)
        {
            printf("%s: read hdr err\n", __FUNCTION__);
            goto label_err;
        }
        if(_chk_android_bootimg_hdr(p_abhdr, ret) != 0)
        {
            printf("%s: bootimg head err\n", __FUNCTION__);
            goto label_err;
        }

        krnl_loadaddr      = p_abhdr->kernel_addr + qb_krnl_loadoffset;
        initramfs_loadaddr = p_abhdr->ramdisk_addr + qb_krnl_loadoffset;
        if(p_abhdr->tags_addr != 0)
        {
            atag_addr = p_abhdr->tags_addr + qb_krnl_loadoffset;
        }
        //
        img_alignsize = p_abhdr->page_size;
        krnl_offset = (sizeof(struct andr_img_hdr) + img_alignsize-1U) & ~(img_alignsize-1U);
        initramfs_offset = krnl_offset +
                ((p_abhdr->kernel_size + img_alignsize-1U) & ~(img_alignsize-1U));

        // load kernel
        ret = _common_read_fs(dev_ifname, dev_part_str, fstype, img_filename,
                krnl_offset, p_abhdr->kernel_size, (void*)krnl_loadaddr);
        if(ret <= 0)
        {
            printf("%s: read kernel err\n", __FUNCTION__);
            goto label_err;
        }

        // load initramfs
        ret = _common_read_fs(dev_ifname, dev_part_str, fstype, img_filename,
                initramfs_offset, p_abhdr->ramdisk_size, (void*)initramfs_loadaddr);
        if(ret <= 0)
        {
            printf("%s: read initramfs err\n", __FUNCTION__);
            goto label_err;
        }

        // check sha1
        {
            sha1_context ctx;
            uint32_t sha1_sum[(SHA1_SUM_LEN + sizeof(uint32_t)-1) / sizeof(uint32_t)];

            sha1_starts(&ctx);
            sha1_update(&ctx, (uint8_t*)krnl_loadaddr, p_abhdr->kernel_size);
            sha1_update(&ctx, (uint8_t*)&p_abhdr->kernel_size, sizeof(p_abhdr->kernel_size));
            sha1_update(&ctx, (uint8_t*)initramfs_loadaddr, p_abhdr->ramdisk_size);
            sha1_update(&ctx, (uint8_t*)&p_abhdr->ramdisk_size, sizeof(p_abhdr->ramdisk_size));
            /* skip check second data */
            //sha1_update(&ctx, NULL, 0);
            sha1_update(&ctx, (uint8_t*)&p_abhdr->second_size, sizeof(p_abhdr->second_size));
            sha1_finish(&ctx, (uint8_t*)&sha1_sum);

            if(memcmp(p_abhdr->id, sha1_sum, SHA1_SUM_LEN) != 0)
            {
                printf("%s: SHA1 err, original %08x%08x... but now %08x%08x...\n",
                        __FUNCTION__,
                        p_abhdr->id[0], p_abhdr->id[1],
                        sha1_sum[0], sha1_sum[1]);
                goto label_err;
            }
        }
    }

    /* setup ATAG space */
    assert(gd->bd != NULL);
    if(gd->bd->bi_boot_params == 0)
    {
        if(atag_addr != 0)
        {
            gd->bd->bi_boot_params = atag_addr;
        }
        else
        {
            gd->bd->bi_boot_params = (ulong) malloc(128*1024);
        }
    }

    /* setup bootm_hdr */
    memset(&bootm_hdr, 0, sizeof(bootm_hdr));
    //bootm_hdr.ft_addr = NULL;
    //bootm_hdr.ft_len = 0;     // we do not use dft.
    bootm_hdr.rd_start = initramfs_loadaddr;
    bootm_hdr.rd_end   = initramfs_loadaddr + p_abhdr->ramdisk_size;
    bootm_hdr.cmdline_start = (ulong)(p_abhdr->cmdline);
    bootm_hdr.cmdline_end   = (ulong)(p_abhdr->cmdline) + strlen(p_abhdr->cmdline);
    bootm_hdr.ep = krnl_loadaddr;

    /* note: p_abhdr is not free here, the 'cmdline' field is still in used. */
    // free(p_abhdr);

    do_bootm_linux(0, 0, NULL, &bootm_hdr);
    puts("do_bootm_linux return!\n");

    label_err:
    free(p_abhdr);
    return -1;
}
