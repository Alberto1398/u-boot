/*
 * OWL GDP(generic Dummy Panel)
 *
 * Copyright (c) 2015 Actions Semi Co., Ltd.
 *
 * Author: Lipeng<lipeng@actions-semi.com>
 *
 * Change log:
 *	2015/8/8: Created by Lipeng.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#define pr_fmt(fmt) "owl_panel_gdp: " fmt

#include <dss.h>

static struct owl_panel owl_panel_gdp = {
	.desc = {
		.name = "panel_gdp",
		.type = OWL_DISPLAY_TYPE_DUMMY,
	},
};

int owl_panel_gdp_init(const void *blob)
{
	int node;

	int ret = 0;

	/*
	 * DTS match
	 */
	node = fdt_node_offset_by_compatible(blob, 0, "actions,panel-gdp");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	/* no need care about return value */
	owl_panel_parse_panel_info(blob, node, &owl_panel_gdp);

	ret = owl_panel_register(&owl_panel_gdp);
	if (ret < 0) {
		error("failed to register panel\n");
		return ret;
	}

	return 0;
}
