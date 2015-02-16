#include <common.h>
#include <asm/arch/actions_arch_common.h>
#include "mxml/mxml.h"
#include "mxml_heap/mxml_malloc.h"
#include "xml_pinctrl.h"
#include "xml_parser.h"
#include "xml_parser_i.h"


static const struct pinctrl_args pinctrl_args_init =
{
	0,
	0,
	0,
	(unsigned int)G_PINCTL_BLOB.asoc_pinmux_group_datas,
	(unsigned int)G_PINCTL_BLOB.asoc_pinmux_groups,
	(unsigned int)G_PINCTL_BLOB.asoc_pinmux_func_names,
	(unsigned int)G_PINCTL_BLOB.asoc_pinmux_func_groups,
	(unsigned int)G_PINCTL_BLOB.asoc_pinmux_funcs,
	(unsigned int)G_PINCTL_BLOB.pinctrl_map_datas,
	(unsigned int)G_PINCTL_BLOB.pinctrl_maps,
	0,
	(unsigned int)G_PINCTL_BLOB.gpios_cfgs,
};

#if 0
int print_xml_tree(pcfg_t *p_pcfg)
{
    int ret=0, loop_flag=1, i;
    mxml_node_t *node, *top;
    mxml_attr_t *attr;          /* Cirrent attribute */

    top = p_pcfg->tree;
    node = p_pcfg->tree;

    do
    {
        node = mxmlWalkNext (node, top, MXML_DESCEND);
        if(node == NULL)
        {
            loop_flag = 0;
            break;
        }
        for (i = node->value.element.num_attrs, attr = node->value.element.attrs;
           i > 0;
           i --, attr ++)
        {
            printf("    %s=\"%s\"\n", attr->name, attr->value);
        }
    }while(loop_flag);

    return ret;
}
#endif


static int _paser_drv_config_common(pcfg_t *p_pcfg)
{
    void *p_fill;
    unsigned int tree_tailer;

    p_pcfg->tree = mxmlLoadString(NULL, p_pcfg->xml_file_buf, MXML_TEXT_CALLBACK);
    if(p_pcfg->tree == NULL)
    {
        return -1;
    }
#if 0
    {
        // for debug
        char *p = mxmlSaveAllocString(p_pcfg->tree, NULL);
        puts("dump _paser_drv_config_common()\n");
        puts(p);
        mxml_mfree(p);
    }
#endif
    tree_tailer = (unsigned int)mxml_malloc(LARGE_BUF_SIZE);
    if(tree_tailer == 0)
    {
        return -2;
    }

    /* 利用内存分配器的线性分配特性, 得出xml链表树占用的连续空间大小, ugly... */
    p_pcfg->tree_size = tree_tailer-(unsigned int)(p_pcfg->tree);
    p_pcfg->tree_size = PAGE_ALIGN(p_pcfg->tree_size);
    /* 填上中间的空隙,避免下次再做这样操作时, 两堆东西有重叠. */
    do
    {
        p_fill = mxml_malloc(1);
    } while(p_fill == NULL || (uint)p_fill >= tree_tailer);
    mxml_mfree((void *)tree_tailer);
    if(p_fill != NULL)
    {
        mxml_mfree(p_fill);
    }

    return 0;
}
int paser_config(void)
{
    pcfg_t *p_pcfg = &g_xmlp_gd_blob.pcfg_config;
    return _paser_drv_config_common(p_pcfg);
}
int paser_preconfig(void)
{
    pcfg_t *p_pcfg = &g_xmlp_gd_blob.pcfg_preconfig;
    return _paser_drv_config_common(p_pcfg);
}

static int __maybe_unused _dump_gpiocfg(void)
{
    int ret=0;
    int i;

	printf("----------gpios_cfgs[%d]----------\n", G_PINCTL_BLOB.gpios_i);
    for(i=0; i<G_PINCTL_BLOB.gpios_i; i++)
    {
        printf("%d:%d:%d:%d:%d:%s\n", \
                G_PINCTL_BLOB.gpios_cfgs[i].iogroup, \
                G_PINCTL_BLOB.gpios_cfgs[i].pin_num, \
                G_PINCTL_BLOB.gpios_cfgs[i].gpio_dir, \
                G_PINCTL_BLOB.gpios_cfgs[i].init_level, \
                G_PINCTL_BLOB.gpios_cfgs[i].active_level, \
                G_PINCTL_BLOB.gpios_cfgs[i].name \
                                );
    }

    return ret;
}

static int _parser_gpiocfg(pcfg_t *p_pcfg)
{
    int ret=0;
    const char *element_attr;
    mxml_node_t *node, *top;

    top = p_pcfg->tree;
	node = mxmlFindElement (top,top, "gpio" , NULL, NULL, MXML_DESCEND);

    do
    {
    	if(node->type == MXML_ELEMENT)
    	{
	        element_attr = mxmlElementGetAttr (node, "name");
	        if(element_attr == NULL)
	        {
	            continue;
	        }
	        strcpy(G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].name, element_attr);

	        element_attr = mxmlElementGetAttr (node, "pad");
	        if(element_attr == NULL)
	        {
	            continue;
	        }
	        element_attr = element_attr+4;
	        G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].iogroup = *element_attr - 'A';
	        element_attr++;
	        G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].pin_num = strtol(element_attr, NULL, 10);

	        element_attr = mxmlElementGetAttr (node, "dir");
	        if(element_attr == NULL)
	        {
	            continue;
	        }
	        G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].gpio_dir = strtol(element_attr, NULL, 10);

	        element_attr = mxmlElementGetAttr (node, "init");
	        if(element_attr)
	        {
	            G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].init_level = strtol(element_attr, NULL, 10);
	        }

	        element_attr = mxmlElementGetAttr (node, "active");
	        if(element_attr)
	        {
	            G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].active_level = strtol(element_attr, NULL, 10);
	            G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].unactive_level = \
	                    (G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].active_level == 1)? 0:1;
	        }

			element_attr = mxmlElementGetAttr (node, "boot");
	        if(element_attr)
	        {
	            G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].boot = strtol(element_attr, NULL, 10);
				//printf("gpios_cfgs[%d].boot=%d\n",
	            //    G_PINCTL_BLOB.gpios_i, G_PINCTL_BLOB.gpios_cfgs[G_PINCTL_BLOB.gpios_i].boot);
	        }
	        G_PINCTL_BLOB.gpios_i++;
	    }
    } while ((node != NULL) && (node = node->next));

    return ret;
}

#define     GPIO_OUTEN_BASE     GPIO_AOUTEN
#define     GPIO_INEN_BASE		GPIO_AINEN
#define     GPIO_DAT_BASE		GPIO_ADAT
#define 	GPIO_REG_SIZE		0xc

static int _gpio_direction_output(int iogroup, int pin_num, int level)
{
	int gpio_inen_reg_addr, gpio_outen_reg_addr, gpio_dat_reg_addr;
	
	gpio_inen_reg_addr = GPIO_INEN_BASE + iogroup*GPIO_REG_SIZE;
	gpio_outen_reg_addr = GPIO_OUTEN_BASE + iogroup*GPIO_REG_SIZE;
	gpio_dat_reg_addr = GPIO_DAT_BASE + iogroup*GPIO_REG_SIZE;

//	printf("%s,gpio_outen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_outen_reg_addr, act_readl(gpio_outen_reg_addr));
//	printf("%s,gpio_inen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_inen_reg_addr, act_readl(gpio_inen_reg_addr));
//	printf("%s,gpio_dat_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_dat_reg_addr, act_readl(gpio_dat_reg_addr));
	
	//disable input
	act_writel(act_readl(gpio_inen_reg_addr) & ~(1 << pin_num), gpio_inen_reg_addr);
	
	//set dat
	if(level == 0)
	{
		act_writel(act_readl(gpio_dat_reg_addr) & ~(1 << pin_num), gpio_dat_reg_addr);
	}
	else
	{
		act_writel(act_readl(gpio_dat_reg_addr) | (1 << pin_num), gpio_dat_reg_addr);
	}

	//enable output
	act_writel(act_readl(gpio_outen_reg_addr) | (1 << pin_num), gpio_outen_reg_addr);

//	printf("%s,gpio_outen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_outen_reg_addr, act_readl(gpio_outen_reg_addr));
//	printf("%s,gpio_inen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_inen_reg_addr, act_readl(gpio_inen_reg_addr));
//	printf("%s,gpio_dat_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_dat_reg_addr, act_readl(gpio_dat_reg_addr));
	return 0;
}

static void _gpio_direction_input(int iogroup, int pin_num)
{
	int gpio_inen_reg_addr,gpio_outen_reg_addr;

	gpio_inen_reg_addr = GPIO_INEN_BASE + iogroup*GPIO_REG_SIZE;		
	gpio_outen_reg_addr = GPIO_OUTEN_BASE + iogroup*GPIO_REG_SIZE;

//	printf("%s,gpio_outen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_outen_reg_addr, act_readl(gpio_outen_reg_addr));
//	printf("%s,gpio_inen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_inen_reg_addr, act_readl(gpio_inen_reg_addr));

	//disable output
	act_writel(act_readl(gpio_outen_reg_addr) & ~(1 << pin_num), gpio_outen_reg_addr);
	
	//enable input
	act_writel(act_readl(gpio_inen_reg_addr) | (1 << pin_num), gpio_inen_reg_addr);
	
//	printf("%s,gpio_outen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_outen_reg_addr, act_readl(gpio_outen_reg_addr));
//	printf("%s,gpio_inen_reg_addr[0x%x]=0x%x\n", __FUNCTION__, gpio_inen_reg_addr, act_readl(gpio_inen_reg_addr));
	return ;
}

static int _set_boot_gpio_val(struct gpio_pre_cfg *p_gpio_pre_cfg)
{
	int ret=0;
	int gpio_iogroup, gpio_pin_num;
	
	gpio_iogroup = p_gpio_pre_cfg->iogroup;
	gpio_pin_num = p_gpio_pre_cfg->pin_num;
	
	switch(p_gpio_pre_cfg->gpio_dir)
	{
		case 0://output
			_gpio_direction_output(gpio_iogroup, gpio_pin_num, p_gpio_pre_cfg->init_level);
			break;
		case 1://input
			_gpio_direction_input(gpio_iogroup, gpio_pin_num);
			break;
		case 2://output & input
			break;
		default:
			break;
	}
	return ret;
}

int set_boot_gpio(void)
{
	int i;
	int boot_gpio_cnt=0;
	
	for(i=0; i<GPIO_CFG_MAX; i++)
	{
//		printf("gpios_cfgs[%d],name:%s,boot:%d\n", i, gpios_cfgs[i].name, gpios_cfgs[i].boot);
		if((G_PINCTL_BLOB.gpios_cfgs[i].name[0] != 0) && (G_PINCTL_BLOB.gpios_cfgs[i].boot == 1))
		{
			_set_boot_gpio_val(&G_PINCTL_BLOB.gpios_cfgs[i]);
			boot_gpio_cnt++;
		}
	}
	return 0;
}

int paser_gpiocfgs(void)
{
    int ret=0;
    pcfg_t *p_pcfg;

    p_pcfg = &g_xmlp_gd_blob.pcfg_gpiocfgs;

    p_pcfg->tree = mxmlLoadString(NULL, p_pcfg->xml_file_buf, MXML_TEXT_CALLBACK);
    if(p_pcfg->tree == NULL)
    {
        return -1;
    }

    _parser_gpiocfg(p_pcfg);
    //_dump_gpiocfg();

    G_PINCTL_BLOB.pinctrl_args.gpios_i = G_PINCTL_BLOB.gpios_i;

    return ret;
}

static int _parser_pinctrl_group_sub(mxml_node_t *top)
{
    static const char group_sub_name[][16]=
    {
        "mfp_cfg",
        "padctl_cfg",
        "pull_cfg",
        "drv_cfg",
    };
    static const unsigned int regsbase[]={MFP_CTL0, PAD_CTL, PAD_PULLCTL0, PAD_DRV0};

    int ret=0;
	int i, regs_i=0;
	unsigned int reg, mask, value;
	mxml_node_t *node;
	const char *element_attr;
	struct asoc_regcfg regcfg;

	//get node of mfp_cfg
	for(i=0; i<4; i++)
	{
	    node = mxmlFindElement (top,top, group_sub_name[i] , NULL, NULL, MXML_DESCEND);
	    if(node == NULL)
	    {
	    	continue;
	    }
	    node = mxmlFindElement (node,node, "register" , NULL, NULL, MXML_DESCEND);
	    if(node == NULL)
	    {
	    	continue;
	    }  
	    do
	    {
	    	if(node->type == MXML_ELEMENT)
	    	{    		
		        element_attr = mxmlElementGetAttr (node, "reg");
		        if(element_attr == NULL)
		        {	        	
		            continue;
		        }
				reg = strtol(element_attr, NULL, 16);
				regcfg.reg = regsbase[i] + (reg*4);

		        element_attr = mxmlElementGetAttr (node, "mask");
		        if(element_attr == NULL)
		        {	        	
		            continue;
		        }
				mask = strtoul(element_attr, NULL, 16);
				regcfg.mask = mask;

		        element_attr = mxmlElementGetAttr (node, "value");
		        if(element_attr == NULL)
		        {	        	
		            continue;
		        }
		        value = strtoul(element_attr, NULL, 16);
		        regcfg.val = value;

		        G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].regcfgs[regs_i] = regcfg;
		    	regs_i++;
		    }
        } while ((node != NULL) && (node = node->next));
	}
	G_PINCTL_BLOB.asoc_pinmux_groups[G_PINCTL_BLOB.groups_i].regcfgs =
	        G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].regcfgs;
	G_PINCTL_BLOB.asoc_pinmux_groups[G_PINCTL_BLOB.groups_i].nregcfgs = regs_i;

    return ret;
}

static int _parser_pinctrl_group_pads(const char *element_attr)
{
    int ret=0;
    int loop_flags=1;
    int i;
    int value, v_first=0, v_end, first_flag=0;
    int pads_cnt=0;
    char *start=(char *)element_attr;
    char *end;

    do
    {
        value = strtol(start, &end, 10);
        switch (*end)
        {
        	case '~':
        		v_first = value;
        		first_flag = 1;
        		break;
        	case ';':
        	case 0:
        		v_end = value;
        		if(first_flag == 1)
        		{
        			first_flag = 0;
        			for(i=0; i<v_end-v_first+1; i++)
        			{
        			    G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].pins[pads_cnt+i] = v_first+i;
        			}
        			pads_cnt += v_end-v_first+1;
        		}
        		else
        		{
        		    G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].pins[pads_cnt] = value;
        			pads_cnt++;
        		}
        		break;
        	default:
        		break;
        }
        if(*end == 0)
        {
        	loop_flags = 0;
        }
        else
        {
        	end++;
        	start = end;
        }
    }while(loop_flags);
    G_PINCTL_BLOB.asoc_pinmux_groups[G_PINCTL_BLOB.groups_i].pins =
            G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].pins;
    G_PINCTL_BLOB.asoc_pinmux_groups[G_PINCTL_BLOB.groups_i].npins = pads_cnt;

    return ret;
}

static int _parser_pinctrl_group(mxml_node_t *top)
{
    int ret=0;
    const char *element_attr;
    mxml_node_t *node=top;

    do
    {
    	if(node->type == MXML_ELEMENT)
    	{
	        element_attr = mxmlElementGetAttr (node, "name");
	        if(element_attr == NULL)
	        {
	            continue;
	        }

			//handle name
	        strcpy(G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].name, element_attr);
	        G_PINCTL_BLOB.asoc_pinmux_groups[G_PINCTL_BLOB.groups_i].name =
	                G_PINCTL_BLOB.asoc_pinmux_group_datas[G_PINCTL_BLOB.groups_i].name;

	        element_attr = mxmlElementGetAttr (node, "pads");
	        if(element_attr == NULL)
	        {
	            continue;
	        }
	        //handle pads
			_parser_pinctrl_group_pads(element_attr);
	        //handle mfp_cfg & padctl_cfg
	        _parser_pinctrl_group_sub(node);

	        G_PINCTL_BLOB.groups_i++;
	    }
    } while ((node != NULL) && (node = node->next));

    return ret;
}

static void __maybe_unused _dump_pinctrl_groups(void)
{
	int i,j;

	printf("%s,groups_i:%d\n", __FUNCTION__, G_PINCTL_BLOB.groups_i);
	for(i=0; i<G_PINCTL_BLOB.groups_i; i++)
	{
		printf("-------groups_i[%d]-------\n", i);
		printf("name:%s\n", G_PINCTL_BLOB.asoc_pinmux_groups[i].name);
		printf("npins:%d\n", G_PINCTL_BLOB.asoc_pinmux_groups[i].npins);
		for(j=0; j<G_PINCTL_BLOB.asoc_pinmux_groups[i].npins; j++)
		{
			printf("%d ", G_PINCTL_BLOB.asoc_pinmux_groups[i].pins[j]);
		}
		printf("\n");
		printf("nregcfgs:%d\n", G_PINCTL_BLOB.asoc_pinmux_groups[i].nregcfgs);
		for(j=0; j<G_PINCTL_BLOB.asoc_pinmux_groups[i].nregcfgs; j++)
		{
			printf("reg:0x%08x, ", G_PINCTL_BLOB.asoc_pinmux_groups[i].regcfgs[j].reg);
			printf("mask:0x%08x, ", G_PINCTL_BLOB.asoc_pinmux_groups[i].regcfgs[j].mask);
			printf("val:0x%08x\n", G_PINCTL_BLOB.asoc_pinmux_groups[i].regcfgs[j].val);
		}
		printf("\n");
	}
}

static int _groups_func_name_func(const char *element_attr);
static int _groups_func_groups_func(const char *element_attr);

static const handle_element_attr groups_func_handle_element_attr[]=
{
	{"name", _groups_func_name_func},
	{"groups", _groups_func_groups_func},
};

static int _groups_func_name_func(const char *element_attr)
{
    int ret=0;

	strcpy(G_PINCTL_BLOB.asoc_pinmux_func_names[G_PINCTL_BLOB.fuctions_i].name, element_attr);
	G_PINCTL_BLOB.asoc_pinmux_funcs[G_PINCTL_BLOB.fuctions_i].name =
	        G_PINCTL_BLOB.asoc_pinmux_func_names[G_PINCTL_BLOB.fuctions_i].name;

    return ret;
}

static int _groups_func_groups_func(const char *element_attr)
{
    int ret=0;
    int j=0;
    int finish_flag=0;
    int str_len;
    char *str=(char *)element_attr;
    char *key;

	do{
		key = strchr(str, ' ');
		if(key != NULL)
		{
			str_len = key-str;
			if(str_len > (32-1))
			{
				printf("%s,%s too long\n", __FUNCTION__, element_attr);
				str_len = 32-1;
			}

			memcpy(G_PINCTL_BLOB.asoc_pinmux_func_groups[G_PINCTL_BLOB.fuctions_i][j], str, str_len);
			G_PINCTL_BLOB.asoc_pinmux_func_groups[G_PINCTL_BLOB.fuctions_i][j][str_len] = 0;
			G_PINCTL_BLOB.asoc_pinmux_func_names[G_PINCTL_BLOB.fuctions_i].groups[j] =
			        G_PINCTL_BLOB.asoc_pinmux_func_groups[G_PINCTL_BLOB.fuctions_i][j];
			key++;
			str = key;
		}
		else
		{
			strcpy(G_PINCTL_BLOB.asoc_pinmux_func_groups[G_PINCTL_BLOB.fuctions_i][j], str);
			G_PINCTL_BLOB.asoc_pinmux_func_names[G_PINCTL_BLOB.fuctions_i].groups[j] =
			        G_PINCTL_BLOB.asoc_pinmux_func_groups[G_PINCTL_BLOB.fuctions_i][j];
			finish_flag = 1;
		}
		j++;
	}while(finish_flag == 0);

	G_PINCTL_BLOB.asoc_pinmux_funcs[G_PINCTL_BLOB.fuctions_i].groups =
	        G_PINCTL_BLOB.asoc_pinmux_func_names[G_PINCTL_BLOB.fuctions_i].groups;
	G_PINCTL_BLOB.asoc_pinmux_funcs[G_PINCTL_BLOB.fuctions_i].ngroups = j;

	return ret;
}

static int _parser_pinctrl_function(mxml_node_t *top)
{
    int ret=0;
    int i;
    const char *element_attr;
    mxml_node_t *node=top;
	int loops=sizeof(groups_func_handle_element_attr)/sizeof(handle_element_attr);

    do
    {
    	if(node->type == MXML_ELEMENT)
    	{
	    	for(i=0; i<loops; i++)
	    	{
		        element_attr = mxmlElementGetAttr (node, groups_func_handle_element_attr[i].name);
		        if(element_attr == NULL)
		        {
                    /* attr 0 must be valid */
                    if (i == 0)
                        goto skip_node;
                    else
    		            continue;
		        }
		        groups_func_handle_element_attr[i].func(element_attr);
		    }
	    	G_PINCTL_BLOB.fuctions_i++;
	    }
skip_node: ;
    } while ((node != NULL) && (node = node->next));


    return ret;
}

static void __maybe_unused _dump_pinctrl_functions(void)
{
	int i,j;

	for(i=0; i<G_PINCTL_BLOB.fuctions_i; i++)
	{
		printf("asoc_pinmux_funcs[%d].name:%s, ", i, G_PINCTL_BLOB.asoc_pinmux_funcs[i].name);
		printf("groups=%d:", G_PINCTL_BLOB.asoc_pinmux_funcs[i].ngroups);
		for(j=0; j<G_PINCTL_BLOB.asoc_pinmux_funcs[i].ngroups; j++)
		{
			printf("%s ", G_PINCTL_BLOB.asoc_pinmux_funcs[i].groups[j]);
		}
		printf("\n");
	}
}

static int _groups_mapping_funciton_func(const char *element_attr);
static int _groups_mapping_group_func(const char *element_attr);
static int _groups_mapping_dev_name_func(const char *element_attr);
static int _groups_mapping_state_name_func(const char *element_attr);
static int _groups_mapping_boot_enable_func(const char *element_attr);

static const handle_element_attr groups_mapping_handle_element_attr[]=
{
	{"function", _groups_mapping_funciton_func},
	{"group", _groups_mapping_group_func},
	{"dev_name", _groups_mapping_dev_name_func},
	{"state_name", _groups_mapping_state_name_func},
	{"boot_enable", _groups_mapping_boot_enable_func},
};

static int _groups_mapping_funciton_func(const char *element_attr)
{
    int ret=0;
	strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].function, element_attr);
    return ret;
}

static int _groups_mapping_group_func(const char *element_attr)
{
    int ret=0;
	strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].group, element_attr);
    return ret;
}

static int _groups_mapping_dev_name_func(const char *element_attr)
{
    int ret=0;
	strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].dev_name, element_attr);
    return ret;
}

static int _groups_mapping_state_name_func(const char *element_attr)
{
    int ret=0;
	strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].name, element_attr);
    return ret;
}

static int _groups_mapping_boot_enable_func(const char *element_attr)
{
    int ret=0;
    G_PINCTL_BLOB.pinctrl_bootenable_maps[G_PINCTL_BLOB.mappings_i] = 1;
    return ret;
}

static void _set_pinctrl(char *group)
{   
    int i,j;
    struct asoc_regcfg *tmp;
    
//    printf("%s:_set_pinctrl:%s\n", __FUNCTION__, group);
    for(i=0; i<MAX_ASOC_PINMUX_GROUPS;i++)
    {
        if(!strcmp(G_PINCTL_BLOB.asoc_pinmux_groups[i].name, group))
        {
            for(j=0; j<G_PINCTL_BLOB.asoc_pinmux_groups[i].nregcfgs; j++)
            {
                tmp = G_PINCTL_BLOB.asoc_pinmux_groups[i].regcfgs;
//                printf("tmp->reg:0x%x, tmp->mask:0x%x, tmp->val:0x%x\n",tmp->reg, tmp->mask, tmp->val);
                act_writel((act_readl(tmp->reg) & ~tmp->mask) | tmp->val, tmp->reg);
            }
        }
        else
            continue;
    }
}

static int _groups_mapping_handle(void)
{
	int ret=0;
	//type
	G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].type = PIN_MAP_TYPE_MUX_GROUP;

	//dev_name
	if(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].dev_name[0] == 0)
	{
		strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].dev_name, PINMUX_DEV);
	}
	G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].dev_name =
	        G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].dev_name;
	//ctrl_dev_name
	if(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].ctrl_dev_name[0] == 0)
	{
		strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].ctrl_dev_name, PINMUX_DEV);
	}
	G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].ctrl_dev_name =
	        G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].ctrl_dev_name;
	//name
	if(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].name[0] == 0)
	{
		strcpy(G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].name, PINCTRL_STATE_DEFAULT);
	}
	G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].name =
	        G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].name;
	//group
	G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].data.mux.group =
	        G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].group;
	//function
	G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].data.mux.function =
	        G_PINCTL_BLOB.pinctrl_map_datas[G_PINCTL_BLOB.mappings_i].function;
    
    if(G_PINCTL_BLOB.pinctrl_bootenable_maps[G_PINCTL_BLOB.mappings_i])
    {
        if(strlen(G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].data.mux.group))
        {
            _set_pinctrl(G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].data.mux.group);
        }
        else
        {
            _set_pinctrl(G_PINCTL_BLOB.pinctrl_maps[G_PINCTL_BLOB.mappings_i].data.mux.function);
        }
    }
    return ret;
}

static int _parser_pinctrl_mapping(mxml_node_t *top)
{
    int ret=0;
    int i;
    const char *element_attr;
    mxml_node_t *node=top;
	int loops=sizeof(groups_mapping_handle_element_attr)/sizeof(handle_element_attr);

    do
    {
    	if(node->type == MXML_ELEMENT)
    	{
	    	for(i=0; i<loops; i++)
	    	{
		        element_attr = mxmlElementGetAttr (node, groups_mapping_handle_element_attr[i].name);
		        if(element_attr == NULL)
		        {
                    /* attr 0 must be valid */
                    if (i == 0)
                        goto skip_node;
                    else
    		            continue;
		        }
		        groups_mapping_handle_element_attr[i].func(element_attr);
		    }
	        _groups_mapping_handle();
	        G_PINCTL_BLOB.mappings_i++;
    	}
skip_node: ;
    } while ((node != NULL) && (node = node->next));

    return ret;
}

#if 0
int dump_xml_node_next(mxml_node_t *top)
{
    int ret=0;
    mxml_node_t *node=top;

    do
    {
		printf("%s:%d,type:%d\n", __FUNCTION__, __LINE__, node->type);
		if(node->type == MXML_ELEMENT)
		{
			printf("%s:%d,name:%s\n", __FUNCTION__, __LINE__, node->value.element.name);
		}
		else if(node->type == MXML_TEXT)
		{
			printf("%s:%d,whitespace:%d\n", __FUNCTION__, __LINE__, node->value.text.whitespace);
		}
		else
		{
			printf("%s:%d,type:%d\n", __FUNCTION__, __LINE__, node->type);
		}
    } while ((node != NULL) && (node = node->next));

    return ret;
}
#endif

static void __maybe_unused _dump_pinctrl_mappings(void)
{
	int i;

	printf("pinctrl_maps:%d\n", G_PINCTL_BLOB.mappings_i);
	for(i=0; i<G_PINCTL_BLOB.mappings_i; i++)
	{
		printf("----------pinctrl_maps[%d]----------\n", i);
		printf("dev_name:%s\n", G_PINCTL_BLOB.pinctrl_maps[i].dev_name);
		printf("name:%s\n", G_PINCTL_BLOB.pinctrl_maps[i].name);
		printf("type:%d\n", G_PINCTL_BLOB.pinctrl_maps[i].type);
		printf("ctrl_dev_name:%s\n", G_PINCTL_BLOB.pinctrl_maps[i].ctrl_dev_name);
		printf("group:%s\n", G_PINCTL_BLOB.pinctrl_maps[i].data.mux.group);
		printf("function:%s\n", G_PINCTL_BLOB.pinctrl_maps[i].data.mux.function);
	}
}

static int _paser_pinctrl(pcfg_t *p_pcfg)
{
    int ret=0;
    mxml_node_t *node, *top;

    top = p_pcfg->tree;

    //get node of group
    node = mxmlFindElement (top,top, "group" , NULL, NULL, MXML_DESCEND);
    _parser_pinctrl_group(node);
    //_dump_pinctrl_groups();

    //get node of function
    node = mxmlFindElement (top,top, "function" , NULL, NULL, MXML_DESCEND);
    _parser_pinctrl_function(node);
    //_dump_pinctrl_functions();

    //get node of mapping
    node = mxmlFindElement (top,top, "mapping" , NULL, NULL, MXML_DESCEND);
    _parser_pinctrl_mapping(node);
    //_dump_pinctrl_mappings();

    return ret;
}

int paser_pinctrls(void)
{
    int ret=0;
    pcfg_t *p_pcfg;

    p_pcfg = &g_xmlp_gd_blob.pcfg_mfpcfgs;

    p_pcfg->tree = mxmlLoadString(NULL, p_pcfg->xml_file_buf, MXML_TEXT_CALLBACK);
    if(p_pcfg->tree == NULL)
    {
        return -1;
    }

    _paser_pinctrl(p_pcfg);

    G_PINCTL_BLOB.pinctrl_args = pinctrl_args_init;
    G_PINCTL_BLOB.pinctrl_args.groups_i   = G_PINCTL_BLOB.groups_i;
    G_PINCTL_BLOB.pinctrl_args.mappings_i = G_PINCTL_BLOB.mappings_i;
    G_PINCTL_BLOB.pinctrl_args.fuctions_i = G_PINCTL_BLOB.fuctions_i;
    G_PINCTL_BLOB.pinctrl_args.gpios_i    = G_PINCTL_BLOB.gpios_i;
	
    return ret;
}
