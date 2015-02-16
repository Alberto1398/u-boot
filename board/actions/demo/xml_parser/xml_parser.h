/*
 * xml_parser.h
 *
 *  Created on: 2014-7-11
 *      Author: liangzhixuan
 */

#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__

#include <common.h>
#include <asm/setup.h>

int act_xmlp_init(void);

struct tag * act_xmlp_generate_xml_tags(struct tag *p_tag_buf);

int act_xmlp_upgrade_parse_all(void *p_xml_config, void *p_xml_mfps, void *p_xml_gpios);

int act_xmlp_boot_parse_f(const char *dev_ifname, const char *dev_part_str, uint fstype);
int act_xmlp_boot_parse_r(const char *dev_ifname, const char *dev_part_str, uint fstype);



#endif /* __XML_PARSER_H__ */
