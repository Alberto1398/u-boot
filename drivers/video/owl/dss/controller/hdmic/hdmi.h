/*
 * hdmi.h
 *
 * HDMI header definition for OWL IP.
 *
 * Copyright (C) 2014 Actions Corporation
 * Author: Guo Long  <guolong@actions-semi.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __OWL_HDMI_H
#define __OWL_HDMI_H

#include <asm/io.h>
#include <asm-generic/errno.h>

#include <owl_dss.h>
#include <dss.h>

#define HDMI_EDID_BUF_LEN	(1024)
#define HDMI_EDID_MAX_VID	(256)

enum hdmi_core_hdmi_dvi {
	HDMI_DVI = 0,
	HDMI_HDMI = 1,
	MHL_24BIT = 2,
	MHL_PK = 3
};

enum SRC_SEL {
	VITD = 0,
	DE,
	SRC_MAX
};

enum hdmi_core_packet_mode {
	HDMI_PACKETMODERESERVEDVALUE = 0,
	HDMI_PACKETMODE24BITPERPIXEL = 4,
	HDMI_PACKETMODE30BITPERPIXEL = 5,
	HDMI_PACKETMODE36BITPERPIXEL = 6,
	HDMI_PACKETMODE48BITPERPIXEL = 7
};

enum hdmi_packing_mode {
	HDMI_PACK_10b_RGB_YUV444 = 0,
	HDMI_PACK_24b_RGB_YUV444_YUV422 = 1,
	HDMI_PACK_20b_YUV422 = 2,
	HDMI_PACK_ALREADYPACKED = 7
};

enum hdmi_pixel_format {
	RGB444 = 0,
	YUV444 = 2
};

enum hdmi_deep_color {
	color_mode_24bit = 0,
	color_mode_30bit = 1,
	color_mode_36bit = 2,
	color_mode_48bit = 3
};

enum hdmi_3d_mode {
	HDMI_2D,
	HDMI_3D_LR_HALF,
	HDMI_3D_TB_HALF,
	HDMI_3D_FRAME,
};


/*
 * a configuration structure to convet HDMI resolutoins
 * between vid and owl_video_timings.
 * vid is some fix number defined by HDMI spec,
 * are used in EDID etc.
 */

enum hdmi_vid_table {
	VID640x480P_60_4VS3 = 1,
	VID720x480P_60_4VS3,
	VID1280x720P_60_16VS9 = 4,
	VID1920x1080I_60_16VS9,
	VID720x480I_60_4VS3,
	VID1920x1080P_60_16VS9 = 16,
	VID720x576P_50_4VS3,
	VID1280x720P_50_16VS9 = 19,
	VID1920x1080I_50_16VS9,
	VID720x576I_50_4VS3,
	VID1440x576P_50_4VS3 = 29,
	VID1920x1080P_50_16VS9 = 31,
	VID1920x1080P_24_16VS9,
	VID3840x2160p_24 = 93,
	VID3840x2160p_25,
	VID3840x2160p_30,
	VID4096x2160p_24 = 98,
	VID4096x2160p_25,
	VID4096x2160p_30,

	/* some specail VID */
	VID1280x1024p_60 = 124,
	VID2560x1024p_60 = 125,
	VID2560x1024p_75 = 126,
	VID3840x1080p_60 = 127,
};

enum hdmi_packet_type {
	PACKET_AVI_SLOT		= 0,
	PACKET_AUDIO_SLOT	= 1,
	PACKET_SPD_SLOT		= 2,
	PACKET_GBD_SLOT		= 3,
	PACKET_VS_SLOT		= 4,
	PACKET_HFVS_SLOT	= 5,
	PACKET_MAX,
};


struct hdmi_config {
	int			vid;

	struct owl_videomode	mode;

	bool			interlace;
	int			vstart;	/*Vsync start line */
	bool			repeat;	/*video data repetetion */
};

struct hdmi_edid {
	uint8_t			edid_buf[HDMI_EDID_BUF_LEN];                        
	uint8_t			device_support_vic[HDMI_EDID_MAX_VID];
	uint8_t			hdmi_mode;
	uint8_t			ycbcr444_support;
	bool			read_ok;
};

/*===========================================================================
 *				HDMI IP
 *=========================================================================*/
struct hdmi_ip;

struct hdmi_ip_ops {
	int (*init)(struct hdmi_ip *ip);
	void (*exit)(struct hdmi_ip *ip);

	int (*power_on)(struct hdmi_ip *ip);
	void (*power_off)(struct hdmi_ip *ip);
	bool (*is_power_on)(struct hdmi_ip *ip);

	void (*hpd_enable)(struct hdmi_ip *ip);
	void (*hpd_disable)(struct hdmi_ip *ip);
	bool (*hpd_is_pending)(struct hdmi_ip *ip);
	void (*hpd_clear_pending)(struct hdmi_ip *ip);
	bool (*cable_status)(struct hdmi_ip *ip);

	int (*video_enable)(struct hdmi_ip *ip);
	void (*video_disable)(struct hdmi_ip *ip);
	bool (*is_video_enabled)(struct hdmi_ip *ip);

	/* generate HDMI packect to 'pkt' according to packect type ('no') */
	int (*packet_generate)(struct hdmi_ip *ip, uint32_t no, uint8_t *pkt);

	/* send packect with 'period', 0 period will stop the packect */
	int (*packet_send)(struct hdmi_ip *ip, uint32_t no, int period);
};

struct hdmi_ip_settings {
	int hdmi_src;
	int vitd_color;
	int pixel_encoding;
	int color_xvycc;
	int deep_color;
	int hdmi_mode;
	int mode_3d;
	int prelines;

	int channel_invert;
	int bit_invert;
};

struct hdmi_ip {
	const void			*blob;	/* fdt blob */
	int				node;	/* fdt node */

	fdt_addr_t			base;	/* register address */

	const struct hdmi_ip_ops	*ops;

	struct hdmi_ip_settings		settings;
	const struct hdmi_config	*cfg;

	void				*pdata;
};


static inline void hdmi_ip_writel(struct hdmi_ip *ip,
			const uint16_t idx, uint32_t val)
{
	writel(val, ip->base + idx);
}

static inline uint32_t hdmi_ip_readl(struct hdmi_ip *ip, const uint16_t idx)
{
	return readl(ip->base + idx);
}

int hdmi_ip_register(struct hdmi_ip *ip);
void hdmi_ip_unregister(struct hdmi_ip *ip);

/*===========================================================================
 *				HDMI controller
 *=========================================================================*/
bool hdmic_get_cable_status(struct owl_display_ctrl *ctrl);

/*===========================================================================
 *				HDMI edid
 *=========================================================================*/
int hdmi_ddc_init(const void *blob);

int hdmi_edid_parse(struct hdmi_edid *edid);

/*===========================================================================
 *				HDMI IPs
 *=========================================================================*/

int hdmi_ip_sx00_init(const void *blob);

/*===========================================================================
 *				HDMI packet
 *=========================================================================*/
int hdmi_packet_gen_infoframe(struct hdmi_ip *ip);

#endif
