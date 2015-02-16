
#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <asm/setup.h>
#include "mxml/mxml.h"
#include "mxml_heap/mxml_malloc.h"
#include "xml_pinctrl.h"
#include "xml_get_cfg.h"
#include "xml_parser.h"
#include "xml_parser_i.h"


// global, shared by xml_parser.c & xml_parser_common.c
act_xml_blob_t g_xmlp_gd_blob __aligned(32);

static void *s_xmlp_heap_base = 0;


/* generate ATAG_XML & ATAG_PINCTRL */
struct tag * act_xmlp_generate_xml_tags(struct tag *p_tag_buf)
{
    struct tag *p_tag = p_tag_buf;

    // ATAG_XML
    if(g_xmlp_gd_blob.pcfg_config.tree != NULL &&
            g_xmlp_gd_blob.pcfg_config.tree_size != 0)
    {
        p_tag->hdr.tag = ATAG_XML;
        p_tag->hdr.size = tag_size(tag_xml);

        p_tag->u.xml.xml_buf_start = (uint)(g_xmlp_gd_blob.pcfg_config.tree);
        p_tag->u.xml.xml_buf_len   = g_xmlp_gd_blob.pcfg_config.tree_size;
        p_tag->u.xml.bin_cfg_xml_buf_start = (uint)(g_xmlp_gd_blob.pcfg_preconfig.tree);
        p_tag->u.xml.bin_cfg_xml_buf_len   = g_xmlp_gd_blob.pcfg_preconfig.tree_size;

        p_tag = tag_next(p_tag);
    }
    else
    {
        puts("act_xmlp not init, no ATAG_XML provided\n");
    }

    // ATAG_PINCTRL
    if(g_xmlp_gd_blob.pinctrl_blob.pinctrl_args.addr_asoc_pinmux_funcs != 0)
    {
        p_tag->hdr.tag = ATAG_PINCTRL;
        p_tag->hdr.size = tag_size(tag_pinctrl);

        p_tag->u.pinctrl.pinctrl_buf_start = (uint)(&g_xmlp_gd_blob.pinctrl_blob);
        p_tag->u.pinctrl.pinctrl_buf_len   = sizeof(g_xmlp_gd_blob.pinctrl_blob);

        p_tag = tag_next(p_tag);
    }
    else
    {
        puts("act_xmlp not init, no ATAG_PINCTRL provided\n");
    }

    return p_tag;
}

int act_xmlp_upgrade_parse_all(void *p_xml_config, void *p_xml_mfps, void *p_xml_gpios)
{
    int ret;

    g_xmlp_gd_blob.pcfg_config.xml_file_buf = p_xml_config;
    g_xmlp_gd_blob.pcfg_preconfig.xml_file_buf = NULL;     // not present in upgrade.
    g_xmlp_gd_blob.pcfg_mfpcfgs.xml_file_buf = p_xml_mfps;
    g_xmlp_gd_blob.pcfg_gpiocfgs.xml_file_buf = p_xml_gpios;

    ret = paser_config();
    if(ret != 0)
    {
        printf("%s: paser_config error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
//    ret = paser_preconfig(); // not present in upgrade.
//    if(ret != 0)
//    {
//        printf("%s: paser_preconfig error, ret=%d\n", __FUNCTION__, ret);
//        return ret;
//    }
    ret = paser_pinctrls();
    if(ret != 0)
    {
        printf("%s: paser_pinctrls error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    ret = paser_gpiocfgs();
    if(ret != 0)
    {
        printf("%s: paser_gpiocfgs error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    //set_boot_gpio();

    return 0;
}

static void * _load_xmlfile_to_buf(const char *dev_ifname, const char *dev_part_str, uint fstype,
        const char *filename)
{
    void *p_tmp_buf, *p_ret_buf;
    uint file_size;
    int ret;

    p_tmp_buf = memalign(ARCH_DMA_MINALIGN, 1 * 1024 * 1024);
    if(p_tmp_buf == NULL)
    {
        printf("%s: sys no mem\n", __FUNCTION__);
        return NULL;
    }

    ret = fs_set_blk_dev(dev_ifname, dev_part_str, fstype);
    if(ret != 0)
    {
        printf("%s: probe fs err, ret=%d\n", __FUNCTION__, ret);
        goto label_err;
    }

    ret = fs_read(filename, (ulong)p_tmp_buf, 0, 0);
    if(ret <= 0)
    {
        printf("%s: read file %s err, ret=%d\n", __FUNCTION__, filename, ret);
        goto label_err;
    }
    file_size = ret;

    p_ret_buf = mxml_malloc(file_size); /* use the internal heap for the return buffer */
    if(p_ret_buf == NULL)
    {
        printf("%s: mxml no mem\n", __FUNCTION__);
        goto label_err;
    }

    memcpy(p_ret_buf, p_tmp_buf, file_size);

    free(p_tmp_buf);
    return p_ret_buf;

    label_err:
    free(p_tmp_buf);
    return NULL;
}

int act_xmlp_boot_parse_f(const char *dev_ifname, const char *dev_part_str, uint fstype)
{
    int ret;

    g_xmlp_gd_blob.pcfg_preconfig.xml_file_buf =
            _load_xmlfile_to_buf(dev_ifname, dev_part_str, fstype, "bin_cfg.xml");
    if(g_xmlp_gd_blob.pcfg_preconfig.xml_file_buf == NULL)
    {
        printf("%s: failed to load %s\n", __FUNCTION__, "bin_cfg.xml");
        return -1;
    }
    ret = paser_preconfig();
    if(ret != 0)
    {
        printf("%s: paser_preconfig error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    debug("%s: load & parse pre_config OK\n", __FUNCTION__);

    g_xmlp_gd_blob.pcfg_mfpcfgs.xml_file_buf =
            _load_xmlfile_to_buf(dev_ifname, dev_part_str, fstype, "pinctrl.xml");
    if(g_xmlp_gd_blob.pcfg_preconfig.xml_file_buf == NULL)
    {
        printf("%s: failed to load %s\n", __FUNCTION__, "pinctrl.xml");
        return -1;
    }
    ret = paser_pinctrls();
    if(ret != 0)
    {
        printf("%s: paser_pinctrls error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    debug("%s: load & parse pinctrls OK\n", __FUNCTION__);

    g_xmlp_gd_blob.pcfg_gpiocfgs.xml_file_buf =
            _load_xmlfile_to_buf(dev_ifname, dev_part_str, fstype, "gpiocfg.xml");
    if(g_xmlp_gd_blob.pcfg_preconfig.xml_file_buf == NULL)
    {
        printf("%s: failed to load %s\n", __FUNCTION__, "gpiocfg.xml");
        return -1;
    }
    ret = paser_gpiocfgs();
    if(ret != 0)
    {
        printf("%s: paser_gpiocfgs error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    debug("%s: load & parse gpiocfgs OK\n", __FUNCTION__);

    set_boot_gpio();
    debug("%s: set_boot_gpio OK\n", __FUNCTION__);

    return 0;
}

/* 因为XML的解释比较慢, 为了加快出LOGO的速度, 将原本一份的"config.xml"砍成两份,
 * LOGO必须的那份小的在act_xmlp_boot_parse_f()里边处理, 剩下的那份在这里处理. */
int act_xmlp_boot_parse_r(const char *dev_ifname, const char *dev_part_str, uint fstype)
{
    int ret;

    g_xmlp_gd_blob.pcfg_config.xml_file_buf =
            _load_xmlfile_to_buf(dev_ifname, dev_part_str, fstype, "config.xml");
    if(g_xmlp_gd_blob.pcfg_config.xml_file_buf == NULL)
    {
        printf("%s: failed to load %s\n", __FUNCTION__, "config.xml");
        return -1;
    }
    ret = paser_config();
    if(ret != 0)
    {
        printf("%s: paser_config() error, ret=%d\n", __FUNCTION__, ret);
        return ret;
    }
    debug("%s: load & parse normal_config OK\n", __FUNCTION__);

    return 0;
}

int act_xmlp_init(void)
{
    uint ret, heap_size;
    void *p_heap_base;

    heap_size = 4 * 1024 * 1024;
    // alloc buffer for internal heap if needed
    p_heap_base = s_xmlp_heap_base;
    if(p_heap_base == 0)
    {
        p_heap_base = memalign(ARCH_DMA_MINALIGN, heap_size);
        if(p_heap_base == NULL)
        {
            puts("no mem for xmlp internal heap\n");
            return -1;
        }
        s_xmlp_heap_base = p_heap_base;
    }
    // init mxml heap
    ret = mxml_init_heap((uint32_t)p_heap_base, heap_size);
    if(ret != 0)
    {
        puts("xmlp heap init err\n");
        return -2;
    }

    // init global blob
    memset(&g_xmlp_gd_blob, 0, sizeof(g_xmlp_gd_blob));

    return 0;
}

