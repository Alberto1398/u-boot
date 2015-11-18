/*
 * Abstraction of OWL Display Controler
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
#define pr_fmt(fmt) "owl_dss_ctrl: " fmt

#include <dss.h>

#define MAX_CTRLS		(4)

static struct owl_display_ctrl	*owl_ctrls_array[MAX_CTRLS];
static int owl_ctrl_num;

int owl_ctrl_init(void)
{
	debug("%s\n", __func__);

	owl_ctrl_num = 0;

	return 0;
}

int owl_ctrl_register(struct owl_display_ctrl *ctrl)
{
	int i;

	if (ctrl == NULL)
		return -1;

	if (owl_ctrl_num > MAX_CTRLS) {
		error("too many ctrls!\n");
		return -1;
	}

	for (i = 0; i < owl_ctrl_num; i++) {
		if (owl_ctrls_array[i]->type == ctrl->type) {
			error("same type ctrl is already registered!\n");
			return -1;
		}
	}

	owl_ctrls_array[owl_ctrl_num] = ctrl;
	owl_ctrl_num++;

	return 0;
}

void owl_ctrl_unregister(struct owl_display_ctrl *ctrl)
{
	/* TODO */
}

struct owl_display_ctrl *owl_ctrl_find_by_type(enum owl_display_type type)
{
	int i;

	for (i = 0; i < owl_ctrl_num; i++) {
		if (owl_ctrls_array[i]->type == type)
			return owl_ctrls_array[i];
	}

	return NULL;
}

void owl_ctrl_set_drvdata(struct owl_display_ctrl *ctrl, void *data)
{
	ctrl->data = data;
}

void *owl_ctrl_get_drvdata(struct owl_display_ctrl *ctrl)
{
	return ctrl->data;
}

int owl_ctrl_add_panel(struct owl_panel *panel)
{
	struct owl_display_ctrl *ctrl = NULL;
	int i;

	debug("%s\n", __func__);

	/* search for a suitable ctrl */
	for (i = 0; i < owl_ctrl_num; i++) {
		ctrl = owl_ctrls_array[i];

		if (ctrl->type == panel->desc.type && ctrl->panel == NULL) {
			debug("%s: got it\n", __func__);
			ctrl->panel = panel;
			panel->ctrl = ctrl;

			if (ctrl->ops && ctrl->ops->add_panel)
				return ctrl->ops->add_panel(ctrl, panel);
			else
				return 0;
		}
	}

	error("%s failed\n", __func__);
	return -1;
}

void owl_ctrl_remove_panel(struct owl_panel *panel)
{
	struct owl_display_ctrl *ctrl = NULL;
	int i;

	debug("%s\n", __func__);

	/* search for the ctrl this 'panel' belongs */
	for (i = 0; i < owl_ctrl_num; i++) {
		ctrl = owl_ctrls_array[i];

		if (ctrl->panel == panel) {
			debug("%s: got it\n", __func__);
			if (ctrl->ops && ctrl->ops->remove_panel)
				ctrl->ops->remove_panel(ctrl, panel);

			ctrl->panel = NULL;
			break;
		}
	}
}

int owl_ctrl_enable(struct owl_display_ctrl *ctrl)
{
	debug("%s\n", __func__);

	if (ctrl->ops && ctrl->ops->enable) {
		if (ctrl->ops->enable(ctrl) < 0) {
			error("enable failed!\n");
			return -1;
		}
	}

	return 0;
}

void owl_ctrl_disable(struct owl_display_ctrl *ctrl)
{
	debug("%s\n", __func__);

	if (ctrl->ops && ctrl->ops->disable)
		ctrl->ops->disable(ctrl);
}
