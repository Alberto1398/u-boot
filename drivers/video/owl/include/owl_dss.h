/*
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/8: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _OWL_DSS_H_
#define _OWL_DSS_H_

/*=============================================================================
 *				OWL DSS Common
 *===========================================================================*/

/* horizontal&vertical sync high active */
#define DSS_SYNC_HOR_HIGH_ACT		(1 << 0)
#define DSS_SYNC_VERT_HIGH_ACT		(1 << 1)

struct owl_videomode {
	int xres;		/* visible resolution */
	int yres;
	int refresh;		/* vertical refresh rate in hz */

	/*
	 * Timing: All values in pixclocks, except pixclock
	 */

	int pixclock;		/* pixel clock in ps (pico seconds) */
	int hfp;		/* horizontal front porch */
	int hbp;		/* horizontal back porch */
	int vfp;		/* vertical front porch */
	int vbp;		/* vertical back porch */
	int hsw;		/* horizontal synchronization pulse width */
	int vsw;		/* vertical synchronization pulse width */

	int sync;		/* see DSS_SYNC_* */
	int vmode;		/* see DSS_VMODE_* */
};

enum owl_display_type {
	OWL_DISPLAY_TYPE_NONE		= 0,
	OWL_DISPLAY_TYPE_LCD		= 1 << 0,
	OWL_DISPLAY_TYPE_DSI		= 1 << 1,
	OWL_DISPLAY_TYPE_EDP		= 1 << 2,
	OWL_DISPLAY_TYPE_CVBS		= 1 << 3,
	OWL_DISPLAY_TYPE_YPBPR		= 1 << 4,
	OWL_DISPLAY_TYPE_HDMI		= 1 << 5,
	OWL_DISPLAY_TYPE_DUMMY		= 1 << 6,
};

enum owl_color_mode {
	OWL_DSS_COLOR_RGB16		= (1 << 0),
	OWL_DSS_COLOR_BGR16		= (1 << 1),

	OWL_DSS_COLOR_ARGB16		= (1 << 2),
	OWL_DSS_COLOR_ABGR16		= (1 << 3),
	OWL_DSS_COLOR_RGBA16		= (1 << 4),
	OWL_DSS_COLOR_BGRA16		= (1 << 5),

	/* RGB24, 32-bit container */
	OWL_DSS_COLOR_RGB24U		= (1 << 6),

	/* RGB24, 24-bit container */
	OWL_DSS_COLOR_RGB24P		= (1 << 7),

	OWL_DSS_COLOR_ARGB32		= (1 << 8),
	OWL_DSS_COLOR_ABGR32		= (1 << 9),
	OWL_DSS_COLOR_RGBA32		= (1 << 10),
	OWL_DSS_COLOR_BGRA32		= (1 << 11),
	OWL_DSS_COLOR_RGBX32		= (1 << 12),

	OWL_DSS_COLOR_NV21		= (1 << 13),	/* YUV 4:2:0 sp */
	OWL_DSS_COLOR_NU21		= (1 << 14),	/* YVU 4:2:0 sp */
	OWL_DSS_COLOR_NV12		= (1 << 15),	/* YVU 4:2:0 SP */
	OWL_DSS_COLOR_YU12		= (1 << 16),	/* YUV 4:2:0 */
};

/* clockwise rotation angle */
enum owl_rotation {
	OWL_DSS_ROT_0 = 0,
	OWL_DSS_ROT_90,
	OWL_DSS_ROT_180,
	OWL_DSS_ROT_270,
};

enum owl_dither_mode {
	DITHER_DISABLE = 0,
	DITHER_24_TO_16,
	DITHER_24_TO_18,
};

enum owl_blending_type {
	/* no blending */
	OWL_BLENDING_NONE,

	/*
	 * premultiplied, H/W should use the following blending method:
	 * src + (1 - src.a) * dst
	 */
	OWL_BLENDING_PREMULT,

	/*
	 * not premultiplied, H/W should use the following blending method:
	 * src * src.a + (1 - src.a) * dst
	 */
	OWL_BLENDING_COVERAGE
};

void owl_dss_init(const void *blob);

int owl_dss_get_color_bpp(enum owl_color_mode color);

/*=============================================================================
 *				OWL DSS Panel
 *===========================================================================*/

struct owl_panel;

struct owl_panel *owl_panel_get_primary_panel(void);
struct owl_panel *owl_panel_get_second_panel(void);

bool owl_panel_hpd_is_connected(struct owl_panel *panel);

void owl_panel_get_default_mode(struct owl_panel *panel,
				struct owl_videomode *mode);

void owl_panel_get_mode(struct owl_panel *panel, struct owl_videomode *mode);
void owl_panel_set_mode(struct owl_panel *panel, struct owl_videomode *mode);

void owl_panel_get_resolution(struct owl_panel *panel, int *xres, int *yres);
uint32_t owl_panel_get_vmode(struct owl_panel *panel);
void owl_panel_get_draw_size(struct owl_panel *panel, int *xres, int *yres);
int owl_panel_get_bpp(struct owl_panel *panel);
enum owl_display_type owl_panel_get_type(struct owl_panel *panel);
void owl_panel_get_gamma(struct owl_panel *panel, int *gamma_r_val,
				int *gamma_g_val, int *gamma_b_val);

int owl_panel_enable(struct owl_panel *panel);
void owl_panel_disable(struct owl_panel *panel);
bool owl_panel_is_enabled(struct owl_panel *panel);

/*=============================================================================
 *				OWL Display Engine
 *===========================================================================*/
#define OWL_DE_VIDEO_MAX_PLANE		(3)

struct owl_de_path;
struct owl_de_video;

struct owl_de_path_info {
	enum owl_display_type		type;

	uint16_t			width;
	uint16_t			height;
	uint32_t			vmode;
	enum owl_dither_mode		dither_mode;

	/* gammas, dither, hist, etc. TODO */
	bool				gamma_adjust_needed;
	int				gamma_r_val;
	int				gamma_g_val;
	int				gamma_b_val;

	bool				dirty;
};

struct owl_de_video_info {
	/* 3D use 2 plane, L/R, L/R, L/R */
	unsigned long			addr[OWL_DE_VIDEO_MAX_PLANE * 2];

	unsigned int			offset[OWL_DE_VIDEO_MAX_PLANE];
	unsigned int			pitch[OWL_DE_VIDEO_MAX_PLANE];

	enum owl_color_mode		color_mode;

	/* crop window */
	unsigned short			xoff;
	unsigned short			yoff;
	unsigned short			width;
	unsigned short			height;

	/* display window */
	unsigned short			pos_x;
	unsigned short			pos_y;
	unsigned short			out_width;	/* if 0, = width */
	unsigned short			out_height;	/* if 0, = height */

	uint32_t			rotate;

	/*
	 * bending type for the layer,
	 * pls see the commit of enum owl_blending_type
	 */
	enum owl_blending_type		blending;

	/*
	 * Alpha value applied to the whole layer, coordinate with
	 * blending type, the H/W's blending method should be:
	 * if blending == OWL_BLENDING_NONE
	 *	a = 255
	 *	src.rgb = src.rgb
	 * if blending == OWL_BLENDING_PREMULT
	 *	a = alpha
	 *	src.rgb = src.rgb * a / 255 + (255 - a) * dst / 255
	 * if blending == OWL_BLENDING_COVERAGE
	 *	a = src.a * alpha / 255
	 *	src.rgb = src.rgb * a / 255 + (255 - a) * dst / 255
	 */
	unsigned char			alpha;

	bool				dirty;
};

int owl_de_get_path_num(void);
int owl_de_get_video_num(void);


/*
 * path functions
 */
struct owl_de_path *owl_de_path_get_by_type(enum owl_display_type type);
struct owl_de_path *owl_de_path_get_by_id(int id);

int owl_de_path_enable(struct owl_de_path *path, bool enable);

void owl_de_path_get_info(struct owl_de_path *path,
			  struct owl_de_path_info *info);
void owl_de_path_set_info(struct owl_de_path *path,
			  struct owl_de_path_info *info);

void owl_de_path_set_go(struct owl_de_path *path);

/*
 * video functions
 */
struct owl_de_video *owl_de_video_get_by_id(int id);

int owl_de_video_set_path(struct owl_de_video *video,
			struct owl_de_path *path);
void owl_de_video_unset_path(struct owl_de_video *video);

int owl_de_video_enable(struct owl_de_video *video, bool enable);

void owl_de_video_get_info(struct owl_de_video *video,
			   struct owl_de_video_info *info);
void owl_de_video_set_info(struct owl_de_video *video,
			   struct owl_de_video_info *info);

#endif
