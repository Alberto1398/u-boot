/********************************************************************************
*
* History:
*      <author>    <time>           <version >             <desc>
*      yujing      2011-12-29       1.0                    build this file
********************************************************************************/
/*!
* \file     adfu.c
* \brief    adfu的代码主体
* \author    yujing
*
* \version 1.0
* \date
*******************************************************************************/

/*!
 * \defgroup adfudec
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_funcs.h>
#include <asm/arch/afinfo.h>

#include <asm/arch/adfuserver.h>
#include <asm/arch/adfudec_transfer.h>


extern void usb2isr(void *p_usr_data);


DECLARE_GLOBAL_DATA_PTR;


/*!
 *   my_s_adfudec_info，定义并初始化了adfudec常用的全局变量 \n
 */
s_adfudec_info  my_s_adfudec_info =
{
    //.d_upgrade_error = 0,
    //.d_upgrade_over = 0,
    //.d_usb_discon = 0,
    //.d_switch_fw_flag = 0,
    .d_transfer_stage = CBW_STAGE,
    .d_upgrade_error = 0,
    .d_upgrade_over = 0,
    //.d_out0_max_packet = 64,
    //.d_in0_max_packet = 64,
    //.d_out1_max_packet = 512,
    //.d_in2_max_packet = 512,
    .d_usb_discon = 0,
    //.d_out1_int_flag = 0,
    .d_switch_fw_flag = 0,
    .d_first_enable_out1fifo_flag = 1,
    .d_largest_sdram_addr = 0,
    .d_real_largest_sdram_addr = 0,
    .d_check_error = 0,
    .d_check_warning = 0,
    .d_my_ret_length = 0,
    .d_access_mem_times = 0,
    .pd_upgrade_buffer_addr = 0,
    .d_sdram_pin_scan = 0,
};

/*!
 *   adfu命令结构体 \n
 */
UNION_CBW   my_cbw  ;//__attribute__ ((aligned(4)));

/*!
 *   adfu的命令状态结构体，在此申明并初始化 \n
 */
UNION_CSW   my_csw =
{
    .sCsw =
    {
        .dCSWSignature = CSW_SIGNATURE,
        .bCSWStatus = CSW_STATUS_COMMAND_PASS,
    }
};


/*!
 *   my_switch_func_entry，用于记录switch命令后跳转的地址 \n
 */
switch_func_entry   my_switch_func_entry;

#ifdef CONFIG_ACTS_ADFUS_BOOT_KRNL
/* kernel download records */
adfus_dl_img_rec_t g_adfus_dl_img_rec;
#endif


/******************************************************************************/
/*!
* \par  Description:
*       从堆中申请buffer
* \param        void
* \return       0
* \ingroup      adfudec
* \par          exmaple code
* \code
*               malloc_buffer();
* \endcode
*******************************************************************************/
static int malloc_buffer (void)
{
    return 0;
}

/******************************************************************************/
/*!
* \par  Description:
*       系统变量初始化
* \param        void
* \return       0
* \ingroup      adfudec
* \par          exmaple code
* \code
*               para_init();
* \endcode
*******************************************************************************/
static int para_init (void)
{
#ifdef CONFIG_ACTS_ADFUS_BOOT_KRNL
    memset(&g_adfus_dl_img_rec, 0, sizeof(g_adfus_dl_img_rec));
#endif
    my_switch_func_entry = (switch_func_entry)0;
    return 0;
}

/******************************************************************************/
#if defined(CONFIG_ACTS_GL5207)
static void USB0_init(void)
{
    /* endpoint config: all is bulk type, quad buffering, endpoint is disabled */
//    act_writeb(EPTYPE_BULK | EPMULTIBUFF_DOUBLE | OUTX_INX_CON_VAL, USB0_IN2CON); //in2
//    act_writeb(EPTYPE_BULK | EPMULTIBUFF_DOUBLE | OUTX_INX_CON_VAL, USB0_OUT1CON);  //OUT1

    /**************************************************
    *use the same configuration as ADFU
    ***************************************************/
    udev->ep[1].maxpacket = BULK_HS_PACKET_SIZE;
    udev->ep[2].maxpacket = BULK_HS_PACKET_SIZE;

    /* disable some interrupt: */
    act_writeb(0x0, USB0_USBIEN);
}
#endif

/******************************************************************************/
/*!
* \par  Description:
*       adfudec的主循环
* \param        void
* \return       ret
* \retval       0 sucess
* \retval       others， failed
* \ingroup      adfudec
* \par          exmaple code
* \code
*               int ret = adfusever();
* \endcode
*******************************************************************************/
static int adfusever (void)
{
    int ret = 0;
    unsigned char bsess_end = 0;

    para_init();

    malloc_buffer();

    /* init USB stage */
    my_s_adfudec_info.d_transfer_stage = CBW_STAGE;

    /* init USB */
#if defined(CONFIG_ACTS_GL5207)
    USB0_init();
    irq_install_handler(67, usb2isr, NULL);
#elif defined(CONFIG_ACTS_GL5202)
    usb3_init();
    irq_install_handler(56, usb3isr, NULL);
#endif

#ifdef CONFIG_SPL_BUILD
    /* in SPL stage, cpu interrupt is always disabled.
     * but in main stage, cpu interrupt is enabled and
     * all interrupt source is disabled until a handler is registered. */
    local_irq_enable();
#endif

    INFO("adfusever enter loop.\n");
    do
    {
        local_irq_disable();
        bsess_end = 0;
        if (*((uint volatile *)&my_s_adfudec_info.d_switch_fw_flag) != 0)
        {
            INFO("about to switch FW...\n");

#if defined(CONFIG_ACTS_GL5202)
            /* reset the usb controller (gl5202 need this) */
            act_cmu_reset_modules(32+14); // usb3
#endif

            /* ADFU server 分两个版本,
             * SPL的版本直接跳到main阶段的uboot即可, main的版本需要调用bootm转到内核. */
#ifdef CONFIG_ACTS_ADFUS_BOOT_KRNL
            act_boot_krnl_from_adfus(
                    g_adfus_dl_img_rec.p_boot_img_hdr,
                    g_adfus_dl_img_rec.p_xml_config,
                    g_adfus_dl_img_rec.p_xml_mfps,
                    g_adfus_dl_img_rec.p_xml_gpios);
#else
            INFO("...\n\n\n\n\n\n\n\n\n\n\n\n");
            my_switch_func_entry((void*)gd, gd->arch.p_afinfo);
#endif
            INFO("error, switch_fw return\n");
            hang();
        }
        else
        {
            if (my_s_adfudec_info.d_first_enable_out1fifo_flag == 1)
            {
                //因brom的switch命令没有清out1xcs，所以此为首次清掉
                my_s_adfudec_info.d_first_enable_out1fifo_flag = 0;
            }
        }
        local_irq_enable();
    }
    while ((bsess_end==0) && (my_s_adfudec_info.d_upgrade_error==0) && (my_s_adfudec_info.d_upgrade_over==0));
    //while ((bsess_end==0));

    if (bsess_end != 0)
    {
        my_s_adfudec_info.d_upgrade_error = USB_PLUG_OUT_ERROR;
    }
    else if (my_s_adfudec_info.d_upgrade_error != 0)
    {
        INFO("my_s_adfudec_info.d_upgrade_error is %d!\n", my_s_adfudec_info.d_upgrade_error);
    }
    else if (my_s_adfudec_info.d_upgrade_over == 1)
    {
        INFO("my_s_adfudec_info.d_upgrade_over is %d!\n", my_s_adfudec_info.d_upgrade_over);
    }
    else
    {
        ;
    }

    ret = - my_s_adfudec_info.d_upgrade_error;

    return ret;
}

/******************************************************************************/
/*!
* \par  Description:
*       adfudec的入口函数, 由IntAdfudec.S调用。
* \param        void
* \return       void
* \ingroup      adfudec
* \par          exmaple code
* \code
*               //由汇编调用。
* \endcode
*******************************************************************************/
void adfuserver_main (void)
{
    INFO("adfuserver start\n");
    adfusever();
    INFO("adfuserver stop\n");
}
