#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <asm/setup.h>
#include "mxml/mxml.h"
#include "mxml_heap/mxml_malloc.h"
#include "xml_pinctrl.h"
#include "xml_parser.h"
#include "xml_parser_i.h"
#include "xml_get_cfg.h"


static mxml_node_t *_search_item(mxml_node_t *top, mxml_node_t *node,
        char *element_name, char *element_attr_name, char *element_attr_value)
{
    int end_flag=1;
    mxml_node_t *first=node, *next;
    char const *value;
    do{
        next = mxmlFindElement (
                                first,
                                top,
                                element_name,
                                NULL,
                                NULL,
                                MXML_DESCEND );
        if(next != NULL)
        {
            value = mxmlElementGetAttr(next, element_attr_name);
            debug("%s: value:%s, element_attr_value:%s\n",
                    __FUNCTION__, value, element_attr_value);
            if(!strcmp(value, element_attr_value))
            {
                end_flag = 0;
            }
            else
            {
                first = next;
            }
        }
        else
        {
            end_flag = 0;
        }
    }while(end_flag);

    return next;
}

/*
    tranfer str(like:"1;2;3;4;5") to arry(like:arry[5]={1,2,3,4,5})
 */
static int _str_to_int_arry(const char *str, int *arry)
{
    int end_flag=1;
    int index=0, len;
    char *first=(char *)str, *next;
    char tmp[12];

    do
    {
        next = strchr(first, ';');
        if(next == NULL)
        {
            arry[index] = strtol(first, NULL, 10);
            debug("%s: arry[%d]:%d\n", __FUNCTION__, index, arry[index]);
            end_flag = 0;
        }
        else
        {
            len = next - first;
            if(len >= 12)
            {
                printf("%s: len:%d too long", __FUNCTION__, len);
            }
            tmp[len] = 0;
            memcpy(tmp, first, len);
            arry[index] = strtol(tmp, NULL, 10);
            debug("%s: arry[%d]:%d\n", __FUNCTION__, index, arry[index]);
            index++;
            next++; //skip ':'
            first = next;
        }
    }while(end_flag);
    return 0;
}

/*
mxml_node_t *node:
char *buff:
int len:
 */
static int _get_config_value(mxml_node_t *node, void *buff, int len)
{
    const char *tmp;
    int arry_flag=0, string_flag=0;
    int ret=0;

    tmp = mxmlElementGetAttr (node, "type");
    debug("%s: type:%s\n", __FUNCTION__, tmp);
    if(!strcmp(tmp, "int"))
    {
        arry_flag = 1;
    }
    else if(!strcmp(tmp, "str"))
    {
        string_flag = 1;
    }
    else
    {
        ret = -1;
        printf("%s: wrong type:%s\n", __FUNCTION__, tmp);
        goto out;
    }
    tmp = mxmlElementGetAttr (node, "value");

    debug("%s: tmp:%s,arry_flag:%d,string_flag:%d",
            __FUNCTION__, tmp,arry_flag,string_flag);

    if(string_flag == 1)
    {
        strcpy(buff,tmp);
    }
    else if(arry_flag == 1)
    {
        //将“1;2;3;4;5;6;7;8”拆成int_arry[0]=1;int_arry[1]=2,etc
        _str_to_int_arry(tmp, (int *)buff);
    }

out:
    return ret;
}

#define SINGLE_ITEM_LEN 32

/*
    get config value according key of config item
    key:support two layer struct,like:lcd.bklight
 */
int act_xmlp_get_config(const char *key, char *buff, int len)
{
    int ret=0;
    int end_flag=1;
    char item_name[SINGLE_ITEM_LEN];
    char *first,*next;
    int item_len;

    mxml_node_t *parent, *top;
    pcfg_t *p_kcfg = &(g_xmlp_gd_blob.pcfg_preconfig);

    top = p_kcfg->tree;

        parent = top;
        first = (char *)key;
        do
        {
            next = strrchr(first, '.');
            if(next == NULL)
            {
                //search item which name matches fist~next
                debug("%s: fist:%s\n", __FUNCTION__, first);
                parent = _search_item(top, parent, "item", "name", first);
                if(parent == NULL)
                {
                    end_flag = 0;
                }
                else
                {
                    _get_config_value(parent, buff, len);
                    end_flag = 0;
                }
            }
            else
            {
                //search top_item which name matches fist~next
                item_len = next-first;
                if(item_len>SINGLE_ITEM_LEN-1)
                {
                    printf("%s: item:%s too long\n", __FUNCTION__, key);
                }
                item_name[item_len]=0;
                memcpy(item_name, first, item_len);
                debug("%s: item_name:%s\n", __FUNCTION__, item_name);
                parent = _search_item(top, parent, "top_item", "name", item_name);
                top = parent;
                if(parent == NULL)
                {
                    end_flag = 0;
                }
                first = ++next;//skip '.'
                debug("%s: first:%s\n", __FUNCTION__, first);
            }
        }while(end_flag);

    if(parent == NULL)
    {
        printf("%s: NOT find item:%s\n", __FUNCTION__, key);
        ret = -1;
    }
    return ret;
}

int act_gpio_get_pre_cfg(char *gpio_name, struct gpio_pre_cfg *m_gpio)
{
    int i;

    for (i=0; i<G_PINCTL_BLOB.gpios_i; i++)
    {
        if (strcmp(G_PINCTL_BLOB.gpios_cfgs[i].name, gpio_name) == 0)
        {
            *m_gpio = G_PINCTL_BLOB.gpios_cfgs[i];
            return 0;
        }
    }
    return -1;
}
