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
#ifndef _DSS_DSS_H_
#define _DSS_DSS_H_

#ifndef DEBUG
/* #define DEBUG */
#endif

#include <config.h>
#include <common.h>
#include <command.h>
#include <fdtdec.h>

#include <asm-generic/gpio.h>

#include <owl_dss.h>

/*
 * recommended preline time in us, the time is
 * related to system's irq(or schedule) latency,
 * and, we should set it according to the actual condition.
 */
#define DSS_RECOMMENDED_PRELINE_TIME    (60)
#define DSS_VMODE_NONINTERLACED		0
#define DSS_VMODE_INTERLACED		1
/*
 * DISABLED--enable-->ENABLED
 * ENABLED--disable-->DISABLED--suspend-->SUSPENDED
 * ENABLED<--enable--DISABLED<--resume--SUSPENDED
 */
enum owl_dss_state {
	OWL_DSS_STATE_ENABLED = 0,
	OWL_DSS_STATE_DISABLED,
	OWL_DSS_STATE_SUSPENDED,
};

/*=============================================================================
 *				Display Engine
 *===========================================================================*/
struct owl_de_device;

enum owl_de_hw_id {
	DE_HW_ID_ATM7059TC,
	DE_HW_ID_ATM7059,
	DE_HW_ID_S900,
	DE_HW_ID_S700,
};
enum owl_de_output_format {
	DE_OUTPUT_FORMAT_RGB = 0,
	DE_OUTPUT_FORMAT_YUV,
};

struct owl_de_path_ops {
	int (*enable)(struct owl_de_path *path, bool enable);
	void (*apply_info)(struct owl_de_path *path);
	void (*set_go)(struct owl_de_path *path);

	void (*set_gamma_table)(struct owl_de_path *path);
	void (*get_gamma_table)(struct owl_de_path *path);
	void (*gamma_enable)(struct owl_de_path *path, bool enable);
};

struct owl_de_path {
	const int			id;
	const char			*name;
	const uint32_t			supported_displays;

	struct owl_de_path_info		info;
	struct owl_de_path_ops		*ops;

	int				ref_cnt;
};

struct owl_de_video_ops {
	int (*set_path)(struct owl_de_video *video, struct owl_de_path *path);
	void (*unset_path)(struct owl_de_video *video);

	int (*enable)(struct owl_de_video *video, bool enable);

	void (*apply_info)(struct owl_de_video *video);
};

struct owl_de_video {
	const int			id;
	const char			*name;

	const uint32_t			supported_colors;

	struct owl_de_video_ops		*ops;

	struct owl_de_video_info	info;

	struct owl_de_path		*path;
	int				ref_cnt;
};

struct owl_de_device_ops {
	int (*power_on)(struct owl_de_device *de);
	int (*init)(struct owl_de_device *de);
	void (*dump_regs)(struct owl_de_device *de);
};

struct owl_de_device {
	enum owl_de_hw_id		hw_id;

	const uint8_t			num_paths;
	struct owl_de_path		*paths;

	const uint8_t			num_videos;
	struct owl_de_video		*videos;

	struct owl_de_device_ops	*ops;

	const void			*blob;
	int				node;
	fdt_addr_t			base;
};

int owl_de_register(struct owl_de_device *de);

int owl_de_s900_init(const void *blob);
int owl_de_s700_init(const void *blob);

/*=============================================================================
 *		Display Controller, only used by panel driver
 *===========================================================================*/
struct owl_display_ctrl;

struct owl_display_ctrl_ops {
	int (*add_panel)(struct owl_display_ctrl *ctrl,
			 struct owl_panel *panel);
	int (*remove_panel)(struct owl_display_ctrl *ctrl,
			    struct owl_panel *panel);

	int (*enable)(struct owl_display_ctrl *ctrl);
	void (*disable)(struct owl_display_ctrl *ctrl);

	bool (*hpd_is_panel_connected)(struct owl_display_ctrl *ctrl);

	int (*power_on)(struct owl_display_ctrl *ctrl);
	int (*power_off)(struct owl_display_ctrl *ctrl);

	/* optional */
	int (*aux_read)(struct owl_display_ctrl *ctrl, char *buf, int count);
	int (*aux_write)(struct owl_display_ctrl *ctrl, const char *buf,
			int count);
};

struct owl_display_ctrl {
	const char			*name;
	enum owl_display_type		type;

	void				*data;

	struct owl_panel		*panel;

	struct owl_display_ctrl_ops	*ops;
};

int owl_ctrl_init(void);

/* for display device module */
int owl_ctrl_add_panel(struct owl_panel *panel);
void owl_ctrl_remove_panel(struct owl_panel *panel);

int owl_ctrl_power_on(struct owl_display_ctrl *ctrl);
int owl_ctrl_power_off(struct owl_display_ctrl *ctrl);

int owl_ctrl_enable(struct owl_display_ctrl *ctrl);
void owl_ctrl_disable(struct owl_display_ctrl *ctrl);

/* for display controller driver */
int owl_ctrl_register(struct owl_display_ctrl *ctrl);
void owl_ctrl_unregister(struct owl_display_ctrl *ctrl);

struct owl_display_ctrl *owl_ctrl_find_by_type(enum owl_display_type type);

void owl_ctrl_set_drvdata(struct owl_display_ctrl *ctrl, void *data);
void *owl_ctrl_get_drvdata(struct owl_display_ctrl *ctrl);

int owl_ctrl_aux_read(struct owl_display_ctrl *ctrl, char *buf, int count);
void owl_ctrl_aux_write(struct owl_display_ctrl *ctrl, const char *buf,
		int count);
/*
 * contollers
 */
int owl_lcdc_init(const void *blob);
int owl_dsic_init(const void *blob);
int owl_edpc_init(const void *blob);
int owl_hdmic_init(const void *blob);
int owl_cvbsc_init(const void *blob);
int owl_dummy_dispc_init(const void *blob);

/*=============================================================================
 *				Display Panel
 *===========================================================================*/

#define OWL_PANEL_MAX_VIDEOMODES	(10)

#define OWL_PANEL_SCALE_FACTOR_MIN	(50)
#define OWL_PANEL_SCALE_FACTOR_MAX	(100)

struct owl_panel_ops {
	int (*power_on)(struct owl_panel *panel);
	int (*power_off)(struct owl_panel *panel);
	int (*enable)(struct owl_panel *panel);
	int (*disable)(struct owl_panel *panel);

	/* optional */
	bool (*hpd_is_connected)(struct owl_panel *panel);
};

struct owl_dss_panel_desc {
	const char			*name;
	enum owl_display_type		type;

	uint32_t			bpp;

	uint32_t			power_on_delay;
	uint32_t			power_off_delay;
	uint32_t			enable_delay;
	uint32_t			disable_delay;

	bool				is_primary;
	bool				need_edid;

	/*
	 * scale factor used for scalint output size,
	 * which range is from 50 to 100, which is 0.5~1
	 */
	uint16_t			scale_factor_x;
	uint16_t			scale_factor_y;

	int				gamma_r_val;
	int				gamma_g_val;
	int				gamma_b_val;

	int				preline_num;	/* recommend value */

	struct owl_panel_ops		*ops;
};

struct owl_panel {
	struct owl_dss_panel_desc	desc;

	int				n_modes;
	struct owl_videomode		mode_list[OWL_PANEL_MAX_VIDEOMODES];

	struct owl_videomode		current_mode;
	struct owl_videomode		default_mode;

	int				draw_width;
	int				draw_height;

	enum owl_dss_state		state;
	void				*pdata;

	struct owl_display_ctrl		*ctrl;
};

#define PANEL_IS_PRIMARY(panel)		((panel)->desc.is_primary)
#define PANEL_NEED_EDID(panel)		((panel)->desc.need_edid)

int owl_panel_init(void);

int owl_panel_register(struct owl_panel *panel);
void owl_panel_unregister(struct owl_panel *panel);

struct owl_panel *owl_panel_get_by_name(const char *name);
struct owl_panel *owl_panel_get_by_type(enum owl_display_type type);

int owl_panel_get_preline_num(struct owl_panel *panel);

void owl_panel_get_scale_factor(struct owl_panel *panel,
				uint16_t *x, uint16_t *y);
void owl_panel_set_scale_factor(struct owl_panel *panel,
				uint16_t x, uint16_t y);

int owl_panel_parse_panel_info(const void *blob, int node,
			       struct owl_panel *panel);

/*
 * general panels
 */
int owl_panel_glp_init(const void *blob);
int owl_panel_gmp_init(const void *blob);
int owl_panel_gep_init(const void *blob);
int owl_panel_ghp_init(const void *blob);
int owl_panel_gdp_init(const void *blob);

/*
 * particular panels
 */
int owl_panel_lq055t3sx02_init(const void *blob);
int owl_panel_amoled_init(const void *blob);
int owl_panel_tc358860_init(const void *blob);
/*===========================================================================
				others
 *===========================================================================*/
/*
 * bit operations. TODO
 * gives bitfields as start : end,
 * where start is the higher bit number.
 * For example 7:0
 */
#define REG_MASK(start, end)	(((1 << ((start) - (end) + 1)) - 1) << (end))
#define REG_VAL(val, start, end) (((val) << (end)) & REG_MASK(start, end))
#define REG_GET_VAL(val, start, end) (((val) & REG_MASK(start, end)) >> (end))
#define REG_SET_VAL(orig, val, start, end) (((orig) & ~REG_MASK(start, end))\
						 | REG_VAL(val, start, end))

#endif
