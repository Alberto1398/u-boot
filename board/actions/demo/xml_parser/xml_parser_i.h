/*
 * xml_parser.h
 *
 *  Created on: 2014-7-11
 *      Author: liangzhixuan
 */

#ifndef __XML_PARSER_I_H__
#define __XML_PARSER_I_H__

#include <common.h>
#include "mxml/mxml.h"
#include "mxml_heap/mxml_malloc.h"
#include "xml_pinctrl.h"

/* 因为要将这些信息全部通过ATAG传给内核, 这里搞了个struct将它们全部包起来.
 * 原来的实现是将它们放到一个特殊section里边的, 传ATAG时传递section的地址, u-boot这里不那样做. */
typedef struct
{
    struct pinctrl_args pinctrl_args;   // header

    int gpios_i;
    struct gpio_pre_cfg gpios_cfgs[GPIO_CFG_MAX];

    int groups_i;
    int mappings_i;
    int fuctions_i;

    struct asoc_pinmux_group_data asoc_pinmux_group_datas[MAX_ASOC_PINMUX_GROUPS];
    struct asoc_pinmux_group asoc_pinmux_groups[MAX_ASOC_PINMUX_GROUPS];

    struct asoc_pinmux_name asoc_pinmux_func_names[MAX_ASOC_PINMUX_FUNCS];
    char asoc_pinmux_func_groups[MAX_ASOC_PINMUX_FUNCS][8][32];
    struct asoc_pinmux_func asoc_pinmux_funcs[MAX_ASOC_PINMUX_FUNCS];

    struct pinctrl_map_data pinctrl_map_datas[PINCTRL_MAP_MUX_MAX];
    struct pinctrl_map pinctrl_maps[PINCTRL_MAP_MUX_MAX];

    char pinctrl_bootenable_maps[PINCTRL_MAP_MUX_MAX];
} act_xml_pinctrl_blob_t;

typedef struct
{
    act_xml_pinctrl_blob_t pinctrl_blob;  // go first! for correct alignment.

    pcfg_t pcfg_config;
    pcfg_t pcfg_preconfig;
    pcfg_t pcfg_mfpcfgs;
    pcfg_t pcfg_gpiocfgs;


} act_xml_blob_t;


// global, shared by xml_parser.c & xml_parser_common.c
extern act_xml_blob_t g_xmlp_gd_blob;

#define G_PINCTL_BLOB   (g_xmlp_gd_blob.pinctrl_blob)



#endif /* __XML_PARSER_I_H__ */
