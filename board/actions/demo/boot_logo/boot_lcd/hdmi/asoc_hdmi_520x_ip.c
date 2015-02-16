#include <common.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/actions_arch_common.h>

#include "asoc_hdmi.h"
#include "asoc_hdmi_5202_reg.h"
#include "asoc_hdmi_520x_packet.h"


int asoc_520x_hdmi_detect_plug(void)
{
	unsigned int status_val;

	status_val = act_readl(HDMI_CR) & HDMI_CR_HPLGSTATUS;
	
	mdelay(2);
	
	status_val &= act_readl(HDMI_CR) & HDMI_CR_HPLGSTATUS;

	if (status_val) 
		return HDMI_PLUGIN;
		
	else 
		return HDMI_PLUGOUT;
		
}

int asoc_520x_hdmi_detect_pending(void)
{
	int reg_val;
	
	reg_val = act_readl(HDMI_CR);

	if ((reg_val & HDMI_CR_ENHPINT)
		&& (reg_val & HDMI_CR_PHPLG)) {
		
		//act_setl(HDMI_CR_PHPLG, HDMI_CR);
                            act_writel(act_readl(HDMI_CR) | HDMI_CR_PHPLG, HDMI_CR);
		return 1;

	}

	return 0;
}

void asoc_520x_hdmi_hpd_enable(void)
{

	/* set debounce */
	act_writel((act_readl(HDMI_CR) | HDMI_CR_HPDEBOUNCE(0xF)) & 
		(~HDMI_CR_PHPLG), HDMI_CR);
	
	/* enable hpd */
	act_writel ((act_readl(HDMI_CR) | HDMI_CR_ENHPINT | 
		HDMI_CR_HPDENABLE) & (~HDMI_CR_PHPLG), HDMI_CR);

}

void asoc_520x_hdmi_init_configure(void)
{
	//hdmi module clock enable 
	act_writel(act_readl(CMU_DEVCLKEN1) | (1 << 3), CMU_DEVCLKEN1);	
	
	 /* enable Audio FIFO_FILL  disable wait cycle*/
	act_writel((act_readl(HDMI_CR) | 0x50) & (~HDMI_CR_PHPLG),
		HDMI_CR);
	/*reset HDCP_CR,HDMI_SCR*/
	act_writel(0, HDMI_SCR);
	act_writel(0, HDCP_CR);
	act_writel(0x2a28b1f9, HDCP_KOWR);
	act_writel(0x20e1fe, HDCP_OWR);
	 /* enable hdmi gcp transmission,clear AVMUTE flag*/
	act_writel (HDMI_GCPCR_ENABLEGCP | HDMI_GCPCR_CLEARAVMUTE,
		HDMI_GCPCR);
	/* disable all RAM packet transmission */
	act_writel(0, HDMI_RPCR);

	/*enable hpd*/
	asoc_520x_hdmi_hpd_enable();

}

void asoc_520x_hdmi_tx_config(struct hdmi_ip_data *ip_data)
{
	unsigned int vid = ip_data->v_cfg.settings->vid;
	unsigned int _3d = ip_data->v_cfg.settings->_3d;
	unsigned int deep_color = ip_data->v_cfg.settings->deep_color;

	switch (vid) {	
	case VID640x480P_60_4VS3:
		if (_3d == _3D) { 
			act_writel(0x918ae903, HDMI_TX_1); 
			act_writel(0x1df00fc1, HDMI_TX_2);
			
		} else {
			act_writel(0x918ee903, HDMI_TX_1); 
			act_writel(0x1df00fc1, HDMI_TX_2);
		}
			
		break;
		
	case VID720x576P_50_4VS3: 
	case VID720x576P_50_16VS9: 
	case VID720x480P_60_4VS3: 
	case VID720x480P_60_16VS9:
	case VID720x480I_60_4VS3: 
	case VID720x480I_60_16VS9:
	case VID720x576I_50_4VS3:
	case VID720x576I_50_16VS9:
		if (_3d == _3D) { 
			act_writel(0x918ae903, HDMI_TX_1); 
			act_writel(0x1df00fc1, HDMI_TX_2);
			
		} else {
			act_writel(0x918ee903, HDMI_TX_1); 
			act_writel(0x1df00fc1, HDMI_TX_2);
		}
			
		break;
		
	case VID1440x480P_60_4VS3: 
	case VID1440x480P_60_16VS9: 
	case VID1440x576P_50_4VS3: 
	case VID1440x576P_50_16VS9:
		if (_3d == _3D) { 
			act_writel(0xb18ae987, HDMI_TX_1); 
			act_writel(0x1df00fc1, HDMI_TX_2);
			
		} else {
			act_writel(0x918ae903, HDMI_TX_1);
			act_writel(0x1df00fc1, HDMI_TX_2);
			
		}
		
		break;
	case VID1280x720P_60_16VS9:
	case VID1280x720P_50_16VS9:
	case VID1280X720P_59P94_16VS9: 
	case VID1920x1080I_60_16VS9: 
	case VID1920x1080I_50_16VS9: 
	case VID1920x1080P_24_16VS9: 
	case VID1920x1080P_25_16VS9: 
	case VID1920x1080P_30_16VS9:
		if (_3d == _3D) { 
			act_writel(0xb182eb9e, HDMI_TX_1); 
			act_writel(0x1ffb0fc1, HDMI_TX_2);
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0x918ae90a, HDMI_TX_1); 
				act_writel(0x1dfa0fc1, HDMI_TX_2);
			}
			
			if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0x918ae903, HDMI_TX_1); 
				act_writel(0x1df20fc1, HDMI_TX_2);
			}
			
			if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0xb18ae987, HDMI_TX_1); 
				act_writel(0x1df20fc1, HDMI_TX_2);
			}
			
		}

		break;
	case VID1920x1080P_60_16VS9:
	case VID1920x1080P_50_16VS9:
	case VID1920x1080P_59P94_16VS9:
		if (_3d == _3D) { 
			act_writel(0xb182e987, HDMI_TX_1); 
			act_writel(0x1df20fc1, HDMI_TX_2);
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0xb182eb9e, HDMI_TX_1); 
				act_writel(0x1ffa0fc1, HDMI_TX_2);
			}
			
			if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0xb182e99a, HDMI_TX_1); 
				act_writel(0x1ffb0fc1, HDMI_TX_2);
			}
			
			if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0xb182ebaa, HDMI_TX_1); 
				act_writel(0x1ffb0fc1, HDMI_TX_2);
			}
			
		}
		break;

	default:
		break;
	}

	
}

void  asoc_520x_hdmi_phy_enable(struct hdmi_ip_data *ip_data)
{

	//enable tmds output&HDMI output
	act_writel(act_readl(TMDS_EODR0) | 0x80000000, TMDS_EODR0);
	//act_setl(HDMI_CR_ENABLEHDMI & ~HDMI_CR_PHPLG, HDMI_CR);
	act_writel((act_readl(HDMI_CR) | HDMI_CR_ENABLEHDMI) & ~HDMI_CR_PHPLG,
		HDMI_CR);
	asoc_520x_hdmi_tx_config(ip_data);

}

int  asoc_520x_hdmi_src_set(struct hdmi_ip_data *ip_data)
{
	int hdmi_src = ip_data->v_cfg.settings->hdmi_src;
	
	if (hdmi_src == VITD) {
		//act_setl(HDMI_ICR_VITD(0x809050) | HDMI_ICR_ENVITD, HDMI_ICR);
		act_writel(act_readl(HDMI_ICR) 
		    | (HDMI_ICR_VITD(0x809050) | HDMI_ICR_ENVITD),
		        HDMI_ICR);

	} else  if (hdmi_src == DE){
		//act_clearl(HDMI_ICR_ENVITD, HDMI_ICR);
		act_writel(act_readl(HDMI_ICR) & (~HDMI_ICR_ENVITD), HDMI_ICR);
		
	} else {
		return -1;
	}

	return 0;
		
}

int asoc_520x_hdmi_set_mode (struct hdmi_ip_data *ip_data)
{	
	unsigned int hdmi_mode = ip_data->v_cfg.settings->hdmi_mode; 
	
	if (hdmi_mode == HDMI_MODE_HDMI) {
		//act_setl(HDMI_SCHCR_HDMI_MODESEL, HDMI_SCHCR);
		act_writel(act_readl(HDMI_SCHCR) | HDMI_SCHCR_HDMI_MODESEL, HDMI_SCHCR);

	} else if (hdmi_mode == HDMI_MODE_DVI) {
		//act_clearl(HDMI_SCHCR_HDMI_MODESEL, HDMI_SCHCR);
		act_writel(act_readl(HDMI_SCHCR) & ~HDMI_SCHCR_HDMI_MODESEL, HDMI_SCHCR);
		
	} else {
		return -1;
		
	}

	return 0;
}

void asoc_520x_hdmi_timing_configure(struct hdmi_ip_data *ip_data)
{
	unsigned int vid = ip_data->v_cfg.settings->vid;
	unsigned int _3d = ip_data->v_cfg.settings->_3d;
	unsigned int deep_color = ip_data->v_cfg.settings->deep_color;

#ifdef IC_DEBUG
	act_writel(act_readl(CMU_TVOUTPLL1DEBUG) & ~(1 << 23), CMU_TVOUTPLL1DEBUG);
#endif
	act_writel(0x827, CMU_TVOUTPLL0DEBUG);
	act_writel(0xF827, CMU_TVOUTPLL1DEBUG);
	act_writel(0xC027, CMU_DCPPLLDEBUG);

	switch (vid) {
	case VID640x480P_60_4VS3:
		if (_3d == _3D) { 
			act_writel(0x00080008, CMU_TVOUTPLL); 
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0x00080018, CMU_TVOUTPLL); 
			} else if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0x00090018, CMU_TVOUTPLL);
			}else if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0x000a0018, CMU_TVOUTPLL);
			}
			
		}
		break;
		
	case VID720x576P_50_4VS3:
	case VID720x576P_50_16VS9:
	case VID720x480P_60_4VS3 :
	case VID720x480P_60_16VS9:
	case VID720x480I_60_4VS3:
	case VID720x480I_60_16VS9:
	case VID720x576I_50_4VS3:
	case VID720x576I_50_16VS9:
		if (_3d == _3D) { 
			act_writel(0x00080c50, CMU_TVOUTPLL); 
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0x00080c60, CMU_TVOUTPLL); 
			} else if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0x00090c60, CMU_TVOUTPLL);
			}else if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0x000a0c60, CMU_TVOUTPLL);
			}
			
		}
		break;
		
	case VID1440x480P_60_4VS3:
	case VID1440x480P_60_16VS9:
	case VID1440x576P_50_4VS3:
	case VID1440x576P_50_16VS9:
		if (_3d == _3D) { 
			act_writel(0x00080c40, CMU_TVOUTPLL); 
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0x00080c50, CMU_TVOUTPLL); 
			} else if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0x00090c50, CMU_TVOUTPLL);
			}else if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0x000a0c50, CMU_TVOUTPLL);
			}
			
		}
		break;
		
		
	case VID1280x720P_60_16VS9: 
	case VID1280X720P_59P94_16VS9:
	case VID1280x720P_50_16VS9:
	case VID1920x1080I_60_16VS9:
	case VID1920x1080I_50_16VS9:
	case VID1920x1080P_24_16VS9:
	case VID1920x1080P_25_16VS9:
	case VID1920x1080P_30_16VS9:
		if (_3d == _3D) { 
			act_writel(0x00080009, CMU_TVOUTPLL); 
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0x00080019, CMU_TVOUTPLL); 
			} else if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0x00090019, CMU_TVOUTPLL);
			}else if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0x000a0019, CMU_TVOUTPLL);
			}
			
		}
		break;

	case VID1920x1080P_60_16VS9://need check
	case VID1920x1080P_59P94_16VS9:
	case VID1920x1080P_50_16VS9:
		if (_3d == _3D) { 
			act_writel(0x00080009, CMU_TVOUTPLL); 
			
		} else {
			if (deep_color == DEEP_COLOR_24_BIT) {
				act_writel(0x00080009, CMU_TVOUTPLL); 
			} else if (deep_color == DEEP_COLOR_30_BIT) {
				act_writel(0x00090009, CMU_TVOUTPLL);
			}else if (deep_color == DEEP_COLOR_36_BIT) {
				act_writel(0x000a0009, CMU_TVOUTPLL);
			}
			
		}
		break;		
	
	default:
		break;
	}
}

void asoc_520x_hdmi_video_interface_setting(struct hdmi_video_config *video_config)
{
	int video2to1scaler = video_config->settings->video2to1scaler;

	if (video2to1scaler) 
		act_writel(video_config->parameters->victl | (1 << 29), HDMI_VICTL);
	else 	
		act_writel(video_config->parameters->victl, HDMI_VICTL);
		
	/*video interface config*/
	act_writel(video_config->parameters->vivsync, HDMI_VIVSYNC);
    	act_writel(video_config->parameters->vivhsync, HDMI_VIVHSYNC);
	act_writel(video_config->parameters->vialseof, HDMI_VIALSEOF);
	act_writel(video_config->parameters->vialseef, HDMI_VIALSEEF);
	act_writel(video_config->parameters->viadlse, HDMI_VIADLSE);
	act_writel((act_readl(HDMI_SCHCR)
		& ~(HDMI_SCHCR_HSYNCPOLINV | HDMI_SCHCR_VSYNCPOLINV 
			| HDMI_SCHCR_HSYNCPOLIN | HDMI_SCHCR_VSYNCPOLIN))
		| (video_config->parameters->vhsync_p + video_config->parameters->vhsync_inv), 
		HDMI_SCHCR);
		
}

void asoc_520x_hdmi_pixel_coding(struct hdmi_settings *settings)
{
	/*pixel coding(RGB default)*/
	if (settings->pixel_encoding == VIDEO_PEXEL_ENCODING_YCbCr444) 
		//act_setl(HDMI_SCHCR_PIXELENCFMT(0x2), HDMI_SCHCR);//YUV444
                           act_writel(act_readl(HDMI_SCHCR) | HDMI_SCHCR_PIXELENCFMT(0x2), HDMI_SCHCR);//YUV444

	else 
		//act_clearl(HDMI_SCHCR_PIXELENCFMT(0x3), HDMI_SCHCR);//RGB
		act_writel(act_readl(HDMI_SCHCR) & ~(HDMI_SCHCR_PIXELENCFMT(0x3)), HDMI_SCHCR);//RGB
}

void asoc_520x_hdmi_deepcolor_setting(struct hdmi_video_config *video_config)
{

	/*deep color,8bit default*/
	if (video_config->settings->deep_color == DEEP_COLOR_24_BIT) {//8bit
		act_writel(act_readl(HDMI_SCHCR) & ~HDMI_SCHCR_DEEPCOLOR_MASK, HDMI_SCHCR);
		act_writel(video_config->parameters->dipccr_24, HDMI_DIPCCR);//Max 18 and 4 Island packets in Vertical and horizontal Blanking Interval
			
	} else if (video_config->settings->deep_color == DEEP_COLOR_30_BIT) {//10bit
		act_writel((act_readl(HDMI_SCHCR) & ~HDMI_SCHCR_DEEPCOLOR_MASK) |
			HDMI_SCHCR_DEEPCOLOR(0x1), HDMI_SCHCR);
		act_writel(video_config->parameters->dipccr_30, HDMI_DIPCCR);//Max 18 and 4 Island packets in Vertical and horizontal Blanking Interval

	} else if (video_config->settings->deep_color == DEEP_COLOR_36_BIT) {//12bit
		act_writel((act_readl(HDMI_SCHCR) & ~HDMI_SCHCR_DEEPCOLOR_MASK) |
			HDMI_SCHCR_DEEPCOLOR(0x2), HDMI_SCHCR);
		act_writel(video_config->parameters->dipccr_36, HDMI_DIPCCR);//Max 18 and 4 Island packets in Vertical and horizontal Blanking Interval
	}
}

void asoc_520x_hdmi_3d_setting(struct hdmi_settings *settings)
{
	
	if (settings->_3d == _3D) 
		act_writel(act_readl(HDMI_SCHCR) | HDMI_SCHCR_HDMI_3D_FRAME_FLAG, HDMI_SCHCR);

	 else 
		act_writel(act_readl(HDMI_SCHCR) & ~HDMI_SCHCR_HDMI_3D_FRAME_FLAG, HDMI_SCHCR);

}

void asoc_520x_hdmi_video_configure(struct hdmi_ip_data *ip_data)
{
	
	/*video interface*/
	asoc_520x_hdmi_video_interface_setting(&ip_data->v_cfg);
	/*pixel coding ,include RGB and YUV*/
	asoc_520x_hdmi_pixel_coding(ip_data->v_cfg.settings);
	/*deep color settings,include 8bit/10bit/12bit*/
	asoc_520x_hdmi_deepcolor_setting(&ip_data->v_cfg);
	/*set 3d format*/
	asoc_520x_hdmi_3d_setting(ip_data->v_cfg.settings);

}

int asoc_520x_hdmi_gcp_configure(struct hdmi_ip_data *ip_data)
{
	unsigned int deep_color = ip_data->v_cfg.settings->deep_color;

	if (deep_color == DEEP_COLOR_24_BIT) {
		/*Clear AVMute flag and enable GCP transmission*/
		act_writel(0x0 | 0x80000002, HDMI_GCPCR);
		
	} else if (deep_color == DEEP_COLOR_30_BIT) {
		act_writel(0x40000050 | 0x80000002, HDMI_GCPCR);
		
	}else if (deep_color == DEEP_COLOR_36_BIT) {
		act_writel(0x40000060 | 0x80000002, HDMI_GCPCR);
		
	} else {
		return -1;
	}

	return 0;
}

int  asoc_520x_hdmi_gen_infoframe(struct hdmi_ip_data *ip_data)
{

	struct hdmi_settings *settings = ip_data->v_cfg.settings;
	
	if (asoc_520x_hdmi_gcp_configure(ip_data))
		return -1;
	
	if (hdmi_gen_avi_infoframe(ip_data))
		return -1;
	
	if (settings->color_xvycc != YCC601_YCC709) {
		hdmi_gen_gbd_infoframe(settings);
	}

	if (settings->_3d != _3D_NOT) {
		hdmi_gen_vs_infoframe(settings);
	}

	return 0;
}
