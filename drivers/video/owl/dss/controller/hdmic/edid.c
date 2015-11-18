/*
 * OWL HDMI EDID.
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/10/14: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define DEBUGX
#define pr_fmt(fmt) "hdmic_edid: " fmt

#include <common.h>
#include <libfdt.h>
#include <fdtdec.h>
#include <i2c.h>

#include "hdmi.h"

#define HDMI_DDC_ADDR		(0x60 >> 1)

#define DDC_EDID_ADDR		(0xa0 >> 1)
#define DDC_HDCP_ADDR		(0x74 >> 1)

struct hdmi_ddc_dev {
	int			bus_id;
};

static struct hdmi_ddc_dev	g_ddc_dev;


static int i2c_check_adapter(void)
{
	if (g_ddc_dev.bus_id >= 0) {
		debug("OK!\n");
		return 0;
	} else {
		debug("ERROR!\n");
		return -EFAULT;
	}
}

static int ddc_read(char segment_index, char segment_offset, char *pbuf)
{
	int ret;
	int retry_num = 0;

	i2c_set_bus_num(g_ddc_dev.bus_id);

retry:
	if (retry_num++ >= 3) {
		debug("%s, read error after %dth retry\n",
		      __func__, retry_num - 1);
		return -1;
	}

	debug("%s, retry = %d\n", __func__, retry_num);

	/* set segment index */
	ret = i2c_write(HDMI_DDC_ADDR, segment_index, 1, NULL, 0);
	/*
	 * skip return value checking, because this command has no ACK,
	 * but u-boot i2c framework will return ERROR
	 */

	/* read data */
	ret = i2c_read(DDC_EDID_ADDR, segment_offset, 1, pbuf, 128);
	if (ret < 0) {
		debug("%s, fail to read EDID data(%d)\n", __func__, ret);
		goto retry;
	}

	debug("%s, finished\n", __func__);

	return 0;
}

static int get_edid_data(u8 block, u8 *buf)
{
	u8 i;
	u8 *pbuf = buf + 128 * block;
	u8 offset = (block & 0x01) ? 128 : 0;

	if (ddc_read(block >> 1, offset, pbuf) < 0) {
		error("read edid error!!!\n");
		return -1;
	}

	debug("Sink: EDID bank %d:\n", block);
	debug("0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	debug("==============================================\n");
	for (i = 0; i < 8; i++) {
		debug("%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x "
		      "%2x %2x %2x %2x\n",
		      pbuf[i * 16 + 0], pbuf[i * 16 + 1],
		      pbuf[i * 16 + 2], pbuf[i * 16 + 3],
		      pbuf[i * 16 + 4], pbuf[i * 16 + 5],
		      pbuf[i * 16 + 6], pbuf[i * 16 + 7],
		      pbuf[i * 16 + 8], pbuf[i * 16 + 9],
		      pbuf[i * 16 + 10], pbuf[i * 16 + 11],
		      pbuf[i * 16 + 12], pbuf[i * 16 + 13],
		      pbuf[i * 16 + 14], pbuf[i * 16 + 15]);
	}
	debug("==============================================\n");

	return 0;
}

static int edid_checksum(u8 block, u8 *buf)
{
	int i = 0, check_sum = 0;
	u8 *pbuf = buf + 128 * block;

	for (i = 0, check_sum = 0; i < 128; i++) {
		check_sum += pbuf[i];
		check_sum &= 0xFF;
	}

	if (check_sum != 0) {
		error("EDID block %d checksum error\n", block);
		return -1;
	}

	return 0;
}

static int edid_header_check(u8 *pbuf)
{
	if (pbuf[0] != 0x00 || pbuf[1] != 0xFF || pbuf[2] != 0xFF ||
	    pbuf[3] != 0xFF || pbuf[4] != 0xFF || pbuf[5] != 0xFF ||
	    pbuf[6] != 0xFF || pbuf[7] != 0x00) {
		error("EDID block0 header error\n");
		return -1;
	}

	return 0;
}

static int edid_version_check(u8 *pbuf)
{
	debug("EDID version: %d.%d", pbuf[0x12], pbuf[0x13]);

	if ((pbuf[0x12] != 0x01) || (pbuf[0x13] != 0x03)) {
		error("Unsupport EDID format, EDID parsing exit\n");
		return -1;
	}

	return 0;
}

static int parse_dtd_block(struct hdmi_edid *edid, u8 *pbuf)
{
	u32 pclk, sizex, hblanking, sizey, vblanking;
	u32 hsync_offset, hsync_plus, vsync_offset, vsync_plus;
	u32 h_image_size, v_image_size, h_border, v_border;
	u32 pixels_total, frame_rate;

	if (pbuf[0] == 0 && pbuf[1] == 0 && pbuf[2] == 0)
		return 0;

	pclk		= ((u32)pbuf[1] << 8) + pbuf[0];
	sizex		= (((u32)pbuf[4] << 4) & 0x0f00) + pbuf[2];
	hblanking	= (((u32)pbuf[4] << 8) & 0x0f00) + pbuf[3];
	sizey		= (((u32)pbuf[7] << 4) & 0x0f00) + pbuf[5];
	vblanking	= (((u32)pbuf[7] << 8) & 0x0f00) + pbuf[6];
	hsync_offset	= (((u32)pbuf[11] << 2) & 0x0300) + pbuf[8];
	hsync_plus	= (((u32)pbuf[11] << 4) & 0x0300) + pbuf[9];
	vsync_offset	= (((u32)pbuf[11] << 2) & 0x0030) + (pbuf[10] >> 4);
	vsync_plus	= (((u32)pbuf[11] << 4) & 0x0030) + (pbuf[8] & 0x0f);
	h_image_size	= (((u32)pbuf[14] << 4) & 0x0f00) + pbuf[12];
	v_image_size	= (((u32)pbuf[14] << 8) & 0x0f00) + pbuf[13];
	h_border	=  pbuf[15];
	v_border	=  pbuf[16];

	pixels_total = (sizex + hblanking) * (sizey + vblanking);

	if (pixels_total == 0)
		return 0;
	else
		frame_rate = (pclk * 10000) / pixels_total;

	if (frame_rate == 59 || frame_rate == 60) {
		if (sizex == 720 && sizey == 240)
			edid->device_support_vic[VID720x480I_60_4VS3] = 1;

		if (sizex == 720 && sizey == 480)
			edid->device_support_vic[VID720x480P_60_4VS3] = 1;

		if (sizex == 1280 && sizey == 720)
			edid->device_support_vic[VID1280x720P_60_16VS9] = 1;

		if (sizex == 1920 && sizey == 540)
			edid->device_support_vic[VID1920x1080I_60_16VS9] = 1;

		if (sizex == 1920 && sizey == 1080)
			edid->device_support_vic[VID1920x1080P_60_16VS9] = 1;
	} else if (frame_rate == 49 || frame_rate == 50) {
		if (sizex == 720 && sizey == 288)
			edid->device_support_vic[VID720x576I_50_4VS3] = 1;

		if (sizex == 720 && sizey == 576)
			edid->device_support_vic[VID720x576P_50_4VS3] = 1;

		if (sizex == 1280 && sizey == 720)
			edid->device_support_vic[VID1280x720P_50_16VS9] = 1;

		if (sizex == 1920 && sizey == 540)
			edid->device_support_vic[VID1920x1080I_50_16VS9] = 1;

		if (sizex == 1920 && sizey == 1080)
			edid->device_support_vic[VID1920x1080P_50_16VS9] = 1;
	} else if (frame_rate == 29 || frame_rate == 30) {
		if (sizex == 3840 && sizey == 2160)
			edid->device_support_vic[VID3840x2160p_30] = 1;
	} else if ((frame_rate == 23) || (frame_rate == 24)) {
		if (sizex == 1920 && sizey == 1080)
			edid->device_support_vic[VID1920x1080P_24_16VS9] = 1;
	}

	debug("PCLK = %d\tXsize = %d\tYsize = %d\tFrame_rate = %d\n",
	      pclk * 10000, sizex, sizey, frame_rate);

	return 0;
}

int parse_videodata_block(struct hdmi_edid *edid, u8 *pbuf, u8 size)
{
	int i = 0;

	while (i < size) {
		edid->device_support_vic[pbuf[i] & 0x7f] = 1;
		if (pbuf[i] & 0x80)
			debug("VIC %d(native) support\n", pbuf[i] & 0x7f);
		else
			debug("VIC %d support\n", pbuf[i]);
		i++;
	}

	debug("device_support_vic :\n");

	debug("0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	debug("==============================================\n");
	for (i = 0; i < 8; i++) {
		debug("%2x %2x %2x %2x %2x %2x %2x %2x "
		      "%2x %2x %2x %2x %2x %2x %2x %2x\n",
		      edid->device_support_vic[i * 16 + 0],
		      edid->device_support_vic[i * 16 + 1],
		      edid->device_support_vic[i * 16 + 2],
		      edid->device_support_vic[i * 16 + 3],
		      edid->device_support_vic[i * 16 + 4],
		      edid->device_support_vic[i * 16 + 5],
		      edid->device_support_vic[i * 16 + 6],
		      edid->device_support_vic[i * 16 + 7],
		      edid->device_support_vic[i * 16 + 8],
		      edid->device_support_vic[i * 16 + 9],
		      edid->device_support_vic[i * 16 + 10],
		      edid->device_support_vic[i * 16 + 11],
		      edid->device_support_vic[i * 16 + 12],
		      edid->device_support_vic[i * 16 + 13],
		      edid->device_support_vic[i * 16 + 14],
		      edid->device_support_vic[i * 16 + 15]);
	}
	debug("==============================================\n");

	return 0;
}

int parse_audiodata_block(u8 *pbuf, u8 size)
{
	u8 sum = 0;

	while (sum < size) {
		if ((pbuf[sum] & 0xf8) == 0x08) {
			debug("max channel = %d\n", (pbuf[sum]&0x7) + 1);
			debug("SampleRate code = %x\n", pbuf[sum + 1]);
			debug("WordLen code = %x\n", pbuf[sum + 2]);
		}
		sum += 3;
	}

	return 0;
}

static u8 hdmi_4k2k_vic_to_vid[5] = {
	0,		/* HDMI_VIC=0 */
	95,		/* HDMI_VIC=1, 3840x2160p@30Hz */
	94,		/* HDMI_VIC=2, 3840x2160p@25Hz */
	93,		/* HDMI_VIC=3, 3840x2160p@24Hz */
	98,		/* HDMI_VIC=4, 4096x2160p@24Hz */
};

int parse_hdmi_vsdb(struct hdmi_edid *edid, u8 *pbuf, u8 size)
{
	u8 index = 8;
	u8 i, hdmi_vic_len, hdmi_vic;

	/*
	 * Byte0: tag
	 * Byte1 Byte2 Byte3: 24bit IEEE Registration Identifier(0x000c03)
	 * Byte4 Byte5: Source Physical Address for CEC
	 *		Sink device version and max tmds rate
	 * Byte 6: Supports_AI;DC_48bit;DC_36bit;DC_30bit;DC_Y444;
	 *	Rsvd;Rsvd;DVI_Daul
	 * Byte7: Max_TMDS_Clock
	 */
	if (pbuf[0] == 0x03 && pbuf[1] == 0x0c && pbuf[2] == 0x00) {
		/* check if it's HDMI VSDB */
		edid->hdmi_mode = HDMI_HDMI;
		debug("Find HDMI Vendor Specific DataBlock\n");
	} else {
		edid->hdmi_mode = HDMI_DVI;
		return 0;
	}

	if (size <= 8)
		return 0;

	/*
	 * Byte8: Latency_Fields_Present;
	 *	I_Latency_Fields_Present;
	 *	HDMI_Video_Present; Rsvd; CNC[3:0]
	 */
	if ((pbuf[7] & 0x20) == 0)	/* no video preesnt */
		return 0;

	if ((pbuf[7] & 0x40) == 1)
		index += 2;

	if ((pbuf[7] & 0x80) == 1)
		index += 2;

	/*
	 * pbuf[index], 3D_Present;3D_Multi_Present(2bit);
	 *	Image_Size(2bit);Rsvd;Rsvd;Rsvd
	 * pbuf[index + 1], HDMI_VIC_LEN(3bit);HDMI_3D_LEN(5bit)
	 */

	/* 3D, TODO */

	/* HDMI 4K2K VID */
	hdmi_vic_len = (pbuf[index + 1] >> 5) & 0x7;
	for (i = 0; i < hdmi_vic_len; i++) {
		hdmi_vic = pbuf[index + 1 + i];
		if (hdmi_vic >= 5)
			continue;

		debug("hdmi_vic %d, vid %d\n", hdmi_vic,
		      hdmi_4k2k_vic_to_vid[hdmi_vic]);
		edid->device_support_vic[hdmi_4k2k_vic_to_vid[hdmi_vic]] = 1;
	}

	return 0;
}

/*===========================================================================
 *			APIs to HDMI core and HDCP
 *=========================================================================*/

int hdmi_ddc_init(const void *blob)
{
	int ret, node, parent;

	debug("%s\n", __func__);

	g_ddc_dev.bus_id = -1;

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,hdmi-edid");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}

	/* get bus ID */
	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		error("%s, Cannot find node parent\n", __func__);
		return -1;
	} 

	g_ddc_dev.bus_id = i2c_get_bus_num_fdt(parent);
	if (g_ddc_dev.bus_id < 0) {
		error("can not get bus id\n");
		return -EINVAL;
	}
	debug("%s, bus id %d\n", __func__, g_ddc_dev.bus_id);

	/*
	 * I2C probe
	 */
	i2c_set_bus_num(g_ddc_dev.bus_id);
	ret = i2c_probe(HDMI_DDC_ADDR);
	if (ret < 0)
		debug("probe slave device failed(%d)!\n", ret);

	return 0;
}

int hdmi_edid_parse(struct hdmi_edid *edid)
{
	u8 block_count;
	u8 bsum, tag, len;
	u8 *ebuf, *ptmp;

	u32 i, offset;

	debug("%s\n", __func__);

	memset(edid->device_support_vic, 0, sizeof(edid->device_support_vic));
	memset(edid->edid_buf, 0, sizeof(edid->edid_buf));

	/*
	 * some test VIDs, cannot parse from EDID now, TODO
	 */
#ifdef HDMI_VID_3840_1080_60
	edid->device_support_vic[127] = 1;	/* 127, 3840x1080@60 */
#endif
#ifdef HDMI_VID_2560_1024_75
	edid->device_support_vic[126] = 1;	/* 126, 2560x1024@75 */
#endif
#ifdef HDMI_VID_2560_1024_60
	edid->device_support_vic[125] = 1;	/* 125, 2560x1024@60 */
#endif
#ifdef HDMI_VID_1280_1024_60
	edid->device_support_vic[124] = 1;	/* 124, 1280x1024@60 */
#endif

	edid->hdmi_mode = HDMI_HDMI;
	edid->ycbcr444_support = 0;
	edid->read_ok = 0;

	ebuf = edid->edid_buf;

	if (i2c_check_adapter()) {
		error("iic adapter error!!!\n");
		goto err0;
	}

	if (get_edid_data(0, ebuf) != 0) {
		error("get_edid_data error!!!\n");
		goto err0;
	}

	if (edid_checksum(0, ebuf) != 0) {
		error("edid_checksum error!!!\n");
		goto err0;
	}

	if (edid_header_check(ebuf) != 0) {
		error("edid_header_check error!!!\n");
		goto err0;
	}

	if (edid_version_check(ebuf) != 0) {
		error("edid_version_check error!!!\n");
		goto err0;
	}

	parse_dtd_block(edid, ebuf + 0x36);
	parse_dtd_block(edid, ebuf + 0x48);

	block_count = ebuf[0x7E];

	if (block_count == 0) {
		error("block_count is 0!!!\n");
		goto err1;
	}

	if (block_count > 4)
		block_count = 4;

	for (i = 1; i <= block_count; i++) {
		get_edid_data(i, ebuf);

		if (edid_checksum(i, ebuf) != 0)
			continue;

		if (ebuf[0x80 * i + 0] != 2)
			continue;

		if (ebuf[0x80 * i + 1] >= 1) {
			if (ebuf[0x80*i + 3] & 0x20) {
				edid->ycbcr444_support = 1;
				debug("support YCbCr44 output\n");
			}
		}

		offset = ebuf[0x80 * i + 2];
		if (offset > 4) {
			/* deal with reserved data block */
			bsum = 4;
			while (bsum < offset) {
				tag = ebuf[0x80 * i + bsum] >> 5;
				len = ebuf[0x80 * i + bsum] & 0x1f;

				if (len > 0 && (bsum + len + 1) > offset) {
					debug("len or bsum size error\n");
					continue;
				}

				ptmp = ebuf + 0x80 * i + bsum + 1;

				if (tag == 1)		/* ADB */
					parse_audiodata_block(ptmp, len);
				else if (tag == 2)	/* VDB */
					parse_videodata_block(edid, ptmp, len);
				else if (tag == 3)	/* vendor specific */
					parse_hdmi_vsdb(edid, ptmp, len);

				bsum += (len + 1);
			}
		} else {
			debug("no data in reserved block%d\n", i);
		}

		if (offset >= 4) {
			/* deal with 18-byte timing block */
			if (offset == 4) {
				edid->hdmi_mode = HDMI_DVI;
				debug("dvi mode\n");
			}

			while (offset < (0x80 - 18)) {
				parse_dtd_block(edid, ebuf + 0x80 * i + offset);
				offset += 18;
			}
		} else {
			debug("no datail timing in block%d\n", i);
		}
	}

err1:
	edid->read_ok = 1;

	debug("end\n");
	return 0;

err0:
	error("read edid err0\n");
	return -1;
}
