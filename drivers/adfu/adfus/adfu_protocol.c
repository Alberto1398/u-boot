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

#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_reg_leopard.h>
#include <asm/arch/actions_reg_rw.h>
#include <asm/arch/actions_arch_funcs.h>
#include <asm/arch/afinfo.h>

#include <asm/arch/adfuserver.h>



/******************************************************************************/
/*!
* \par  Description:
*       adfu cmd = switch_fw，即将设备切换到另一个程序中
* \param        void
* \return       0
* \ingroup      adfudec
* \par          exmaple code
* \code
*               my_switch_fw();
* \endcode
*******************************************************************************/
static uint8_t s__my_switch_fw_pending_flag = 0;
static int _my_switch_fw (void)
{
      unsigned int  ram_addr;
      unsigned int  *   pdAddr;

      pdAddr = (unsigned int *) my_cbw.sCbw.cCBWCB;
      pdAddr ++;         //new protocol
      pdAddr ++;         //new protocol
      ram_addr = *pdAddr;

      INFO("%s: ram_addr=0x%x\n", __FUNCTION__, ram_addr);

      my_switch_func_entry = (switch_func_entry)ram_addr;

      s__my_switch_fw_pending_flag = 1;

      return 0;
}

/******************************************************************************/
/*!
* \par  Description:
*       adfu cmd = access_internal_ram，即访问设备的ram
* \param        void
* \return       0
* \ingroup      adfudec
* \par          exmaple code
* \code
*               my_access_internal_ram();
* \endcode
*******************************************************************************/
static unsigned int s_my_access_internal_ram_addr = 0;
static unsigned int s_my_access_internal_ram_size = 0;
//
static void _my_access_internal_ram_post_handler(void)
{
    if(my_s_adfudec_info.d_transfer_stage == DATA_OUT_STAGE)
    {
        unsigned int __attribute__((unused)) ram_addr;
        unsigned int __attribute__((unused)) length;

        ram_addr = s_my_access_internal_ram_addr;
        length = s_my_access_internal_ram_size;

#ifdef CONFIG_ACTS_ADFUS_BOOT_KRNL
        if(ram_addr == 0x30008000U)
        {
            struct andr_img_hdr *p = (struct andr_img_hdr *)ram_addr;
            if(memcmp(p->magic, ANDR_BOOT_MAGIC, sizeof(p->magic)) == 0)
            {
                INFO("Receive boot_img_hdr @ 0x%x\n", (uint)p);
                g_adfus_dl_img_rec.p_boot_img_hdr = p;
            }
        }
#endif
    }
}
//
static void _my_access_internal_ram(void)
{
    unsigned int    ram_addr, length;
    char  direction;
    char  *pdAddr;

    pdAddr = (char *) my_cbw.sCbw.cCBWCB;
    direction = (char)(((*pdAddr) & 0x80) >> 7);
    pdAddr = pdAddr + 4;
    length = *((unsigned int *) pdAddr);
    pdAddr = pdAddr + 4;
    ram_addr = *((unsigned int *) pdAddr);

    INFO("%s: dir=%c ram_addr=0x%x length=%u\n",
            __FUNCTION__, (direction==0)?'w':'r', ram_addr, length);

    if(direction != 0) //in
    {
        my_s_adfudec_info.d_transfer_stage = DATA_IN_STAGE;
#if defined(CONFIG_ACTS_GL5207)
        USB0_epin2_sendata(length,(unsigned char  *)ram_addr);
#elif defined(CONFIG_ACTS_GL5202)
        actions_data_in(length,(unsigned int)ram_addr);
#endif
    }
    else //out
    {
#ifdef CONFIG_ACTS_ADFUS_BOOT_KRNL
        // filter special doenload stubs
        if((ram_addr&0xffff0000U) == 0x12340000U)
        {
            void *p_new_buf;

            p_new_buf = malloc(length + 64U); /* 预留64byte做后备. */
            if(p_new_buf == NULL)
            {
                INFO("Attempt to receive (token:0x%x size:%u) but no mem\n",
                        ram_addr, length);
                hang();
            }
            memset(p_new_buf, 0, length + 64U);

            switch(ram_addr & 0xffffU)
            {
            case 0:
                INFO("Receive xml_pads @ 0x%x\n", (uint)p_new_buf);
                g_adfus_dl_img_rec.p_xml_config = p_new_buf;
                break;
            case 1:
                INFO("Receive xml_mfps @ 0x%x\n", (uint)p_new_buf);
                g_adfus_dl_img_rec.p_xml_mfps = p_new_buf;
                break;
            case 2:
                INFO("Receive xml_gpios @ 0x%x\n", (uint)p_new_buf);
                g_adfus_dl_img_rec.p_xml_gpios = p_new_buf;
                break;
            default:
                INFO("Unknown dl_token: 0x%x\n", ram_addr);
                hang();
            }
            ram_addr = (uint)p_new_buf;
        }
#endif

        my_s_adfudec_info.d_transfer_stage = DATA_OUT_STAGE;
#if defined(CONFIG_ACTS_GL5207)
        USB0_epout1_receivedata(length,(unsigned char  *)ram_addr);
#elif defined(CONFIG_ACTS_GL5202)
        actions_data_out(length,(unsigned int)ram_addr);
#endif

        // call post handler
        s_my_access_internal_ram_addr = ram_addr;
        s_my_access_internal_ram_size = length;
#if defined(CONFIG_ACTS_GL5207)
        // call post handler immediately
        _my_access_internal_ram_post_handler();
#elif defined(CONFIG_ACTS_GL5202)
        // post handler will be called in OUT interrupt
#endif
    }
}


/******************************************************************************/
/*!
* \par  Description:
*       UsbOtgEp1OutDeal为代码入口，然后解析命令并执行相应操作\n
*       从adfudec.c的adfuserver中调用
* \param        void
* \return       0
* \ingroup      adfudec
* \par          exmaple code
* \code
*              UsbOtgEp1OutDeall();
* \endcode
*******************************************************************************/
#if defined(CONFIG_ACTS_GL5207)
int Usb3Ep1OutDeal(void)
{
    UCHAR i, j, k;
    USHORT wLength;

    UCHAR * pbBuffer;
    UCHAR * pbData;

    if(my_s_adfudec_info.d_upgrade_error != 0) return 0;

    if(my_s_adfudec_info.d_transfer_stage == CBW_STAGE)
    {
        if(!(act_readb(USB0_OUT1CS) & 0x02))
        {
            wLength = act_readw(USB0_OUT1BCL);
            j = wLength / 4;
            k = 0;
            for(i = 0; i < j; i++) //data may not be alligned by word
            {
                my_cbw.all[i] = act_readl(USB0_FIFO1DATA);
                k += 4;
            }
            pbBuffer = (UCHAR *) (&my_cbw.all[j]);
            pbData = (UCHAR *) USB0_FIFO1DATA;
            while(k < wLength)
            {
                *pbBuffer = *pbData;
                pbBuffer++;
                pbData++;
                k++;
            }
            act_writeb(0, USB0_OUT1CS);

            if(my_cbw.sCbw.dCBWSignature != CBW_SIGNATURE)
            {
                my_s_adfudec_info.d_upgrade_error = USB_CBW_SIGN_ERROR;
                return 0;
            }
            else
            {
                my_csw.sCsw.dCSWTag = my_cbw.sCbw.dCBWTag;
                my_csw.sCsw.bCSWStatus = CSW_STATUS_COMMAND_PASS;
            }

            /* parse CBWCB: */
            switch(my_cbw.sCbw.cCBWCB[0] & 0x7F)      //new protocol
            //switch(my_cbw.sCbw.bOperationCode)      //old protocol
            {
            case ACCESS_INTERNAL_RAM:
                _my_access_internal_ram();
                my_s_adfudec_info.d_transfer_stage = CSW_STAGE;
                my_csw.sCsw.bCSWStatus = (UCHAR) CSW_STATUS_COMMAND_PASS;
                break;

            case SWITCH_FW:
                my_s_adfudec_info.d_transfer_stage = CSW_STAGE;
                _my_switch_fw();
                //USB0_epin2_sendata(sizeof(CSW),(unsigned char  *)&(my_csw.sCsw));
                my_csw.sCsw.bCSWStatus = (UCHAR) CSW_STATUS_COMMAND_PASS;
                break;

            default:
                my_s_adfudec_info.d_transfer_stage = CSW_STAGE;

                /*send eror CSW*/
                my_csw.sCsw.bCSWStatus = CSW_STATUS_COMMAND_FAIL;

                //USB0_epin2_sendata(sizeof(CSW),(uint32_t)&(my_csw.sCsw));
                break;
            }

            //send csw
            my_csw.sCsw.dCSWSignature = CSW_SIGNATURE;
            my_csw.sCsw.dCSWTag = my_cbw.sCbw.dCBWTag;
            my_csw.sCsw.bReserved[0] = 0;
            my_csw.sCsw.bReserved[1] = 0;
            my_csw.sCsw.bReserved[2] = 0;
            my_csw.sCsw.bReserved[3] = 0;
            //my_csw.sCsw.bCSWStatus = (UCHAR)CSW_STATUS_COMMAND_PASS;
            //sUsbOtgInfo.sEp1InDealWith.pData =  (UCHAR *)(&(my_csw.uCsw.all[0]));
            //sUsbOtgInfo.sEp1InDealWith.wLength = CSW_LENGTH;
            //UsbOtgEp2SendData(sUsbOtgInfo.sEp1InDealWith.wLength, sUsbOtgInfo.sEp1InDealWith.pData);
            USB0_epin2_sendata(sizeof(CSW), (unsigned char *) &(my_csw.sCsw));

            my_s_adfudec_info.d_transfer_stage = CBW_STAGE;

            /***********************************************************
             *如果在这里DDR初始需要时间太长的话，可能会出现timout
             ***********************************************************/

            if(s__my_switch_fw_pending_flag != 0) /* do this after CSW */
            {
                my_s_adfudec_info.d_switch_fw_flag = 1;
            }
        }
    }
    return 0;
}
#endif

/******************************************************************************/
/*!
* \par  Description:
*       UsbOtgEp1OutDeal为代码入口，然后解析命令并执行相应操作\n
*       从adfudec.c的adfuserver中调用
* \param        void
* \return       0
* \ingroup      adfudec
* \par          exmaple code
* \code
*              UsbOtgEp1OutDeall();
* \endcode
*******************************************************************************/
#if defined(CONFIG_ACTS_GL5202)
int Usb3Ep1OutDeal (void *buf)
{
    //INFO("%s: enter\n", __FUNCTION__);

    memcpy((unsigned char *)my_cbw.all, (unsigned char *)buf, 31);

    if (my_s_adfudec_info.d_upgrade_error == 0)
    {
        if (my_cbw.sCbw.dCBWSignature != CBW_SIGNATURE)
        {
            my_s_adfudec_info.d_upgrade_error = USB_CBW_SIGN_ERROR;
            INFO("USB_CBW_SIGN_ERROR!\n");
        }
        else
        {
            my_csw.sCsw.dCSWTag = my_cbw.sCbw.dCBWTag;
            my_csw.sCsw.bCSWStatus = CSW_STATUS_COMMAND_PASS;
        }
    }

    if (my_s_adfudec_info.d_upgrade_error == 0)
    {
        if(my_s_adfudec_info.d_transfer_stage == CBW_STAGE)
        {
            //INFO("%s: handle CBW stage\n", __FUNCTION__);

            /* parse CBWCB: */
#ifndef FOR_OLD_PROTOCOL_OF_ADFU
            //INFO("UsbOtgEp1OutDeal: bOperationCode is %X!\n", my_cbw.sCbw.bOperationCode);
            switch (my_cbw.sCbw.cCBWCB[0] & 0x7F)
#else
            switch (my_cbw.sCbw.bOperationCode)
#endif
            {
            case ACCESS_INTERNAL_RAM:
                _my_access_internal_ram();
                break;

            case SWITCH_FW:
                my_s_adfudec_info.d_transfer_stage = CSW_STAGE;
                _my_switch_fw();
                actions_data_in(sizeof(CSW),(unsigned int)&(my_csw.sCsw));
                break;

            default:
                INFO("USB_UNKNOWN_CMD_ERROR!\n");
                my_s_adfudec_info.d_upgrade_error = USB_UNKNOWN_CMD_ERROR;
                break;
            }
        }
        else if(my_s_adfudec_info.d_transfer_stage == DATA_OUT_STAGE)
        {
            //INFO("%s: handle DATA_OUT stage\n", __FUNCTION__);

            _my_access_internal_ram_post_handler();

            my_s_adfudec_info.d_transfer_stage = CSW_STAGE;
            actions_data_in(sizeof(CSW),(unsigned int)&(my_csw.sCsw));
        }
        else
        {
            my_s_adfudec_info.d_upgrade_error = USB_OUT1_DATA_TRANSFER_ERROR;
            INFO("USB_ERROR, unknown transfer stage!\n");
            hang(); //error here
        }
    }
    else
    {
        INFO("%s: usb error\n", __FUNCTION__);
    }

    //INFO("%s: exit\n", __FUNCTION__);
    return 0;
}

int Usb3Ep2InDeal (void *buf)
{
    //INFO("%s enter\n", __FUNCTION__);

    if (my_s_adfudec_info.d_upgrade_error == 0)
    {
        if (my_cbw.sCbw.dCBWSignature != CBW_SIGNATURE)
        {
            my_s_adfudec_info.d_upgrade_error = USB_CBW_SIGN_ERROR;
            INFO("USB_CBW_SIGN_ERROR!\n");
        }
        else
        {
            my_csw.sCsw.dCSWTag = my_cbw.sCbw.dCBWTag;
            my_csw.sCsw.bCSWStatus = CSW_STATUS_COMMAND_PASS;
        }
    }

    if (my_s_adfudec_info.d_upgrade_error == 0)
    {
        if(my_s_adfudec_info.d_transfer_stage == DATA_IN_STAGE)
        {
            //INFO("%s: handle DATA_IN stage\n", __FUNCTION__);

            my_s_adfudec_info.d_transfer_stage = CSW_STAGE;
            actions_data_in(sizeof(CSW),(unsigned int)&(my_csw.sCsw));
        }
        else if(my_s_adfudec_info.d_transfer_stage == CSW_STAGE)
        {
            //INFO("%s: handle CSW stage\n", __FUNCTION__);

            if(s__my_switch_fw_pending_flag != 0) /* do this after CSW */
            {
                my_s_adfudec_info.d_switch_fw_flag = 1;
            }

            my_s_adfudec_info.d_transfer_stage = CBW_STAGE;
            //prepare the CBW TRB for next transfer
            actions_data_out(512, (unsigned int)buf);
        }
        else
        {
            my_s_adfudec_info.d_upgrade_error = USB_IN2_DATA_TRANSFER_ERROR;
            INFO("USB_ERROR, unknown transfer stage!\n");
            hang(); //error here
        }
    }
    else
    {
        INFO("%s: usb error\n", __FUNCTION__);
    }

    //INFO("%s exit\n", __FUNCTION__);
    return 0;
}
#endif

