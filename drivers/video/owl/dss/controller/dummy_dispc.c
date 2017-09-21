/*
 * a dummy display controller
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
#define pr_fmt(fmt) "owl_dummy_dispc: " fmt

#include <common.h>

#include <dss.h>

static struct owl_display_ctrl owl_dummy_dipsc_ctrl = {
	.name = "dummy_dispc",
	.type = OWL_DISPLAY_TYPE_DUMMY,
};

int owl_dummy_dispc_init(const void *blob)
{
	int ret = 0;
	int node;

	/* DTS match */
	node = fdt_node_offset_by_compatible(blob, 0,
					     "actions,s900-dummy-dispc");
	if (node < 0) {
		debug("no match in DTS\n");
		return 0;
	}
	debug("%s\n", __func__);

	ret = owl_ctrl_register(&owl_dummy_dipsc_ctrl);
	if (ret < 0)
		goto err_ctrl_register;

	return 0;

err_ctrl_register:

	return ret;
}
