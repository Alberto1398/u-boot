/*
 * Abstraction of OWL Display Engine
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
#define DEBUGX
#define pr_fmt(fmt) "owl_de: " fmt

#include <dss.h>

static struct owl_de_device *cur_de;

/*=============================================================================
			external functions for others
 *===========================================================================*/

/*
 * global functions
 */

int owl_de_register(struct owl_de_device *de)
{
	debug("%s\n", __func__);

	if (de == NULL) {
		error("de is NULL\n");
		return -1;
	}

	if (cur_de != NULL) {
		error("another de is already registered\n");
		return -1;
	}

	cur_de = de;

	cur_de->base = fdtdec_get_addr(cur_de->blob, cur_de->node, "reg");
	if (cur_de->base == FDT_ADDR_T_NONE) {
		error("Cannot find reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, cur_de->base);

	/* result check, TODO */
	if (cur_de->ops && cur_de->ops->power_on)
		cur_de->ops->power_on(cur_de);

	/* result check, TODO */
	if (cur_de->ops && cur_de->ops->init)
		cur_de->ops->init(cur_de);

	return 0;
}

int owl_de_get_path_num(void)
{
	return cur_de->num_paths;
}

int owl_de_get_video_num(void)
{
	return cur_de->num_videos;
}

/*
 * functions for de path
 */

struct owl_de_path *owl_de_path_get_by_type(enum owl_display_type type)
{
	struct owl_de_path *path = NULL;
	int i;

	debug("%s, type = %d\n", __func__, type);

	for (i = 0; i < owl_de_get_path_num(); i++) {
		if ((cur_de->paths[i].supported_displays & type) != 0) {
			path = &cur_de->paths[i];
			break;
		}
	}

	return path;
}

struct owl_de_path *owl_de_path_get_by_id(int id)
{
	struct owl_de_path *path = NULL;
	int i;

	debug("%s, id = %d\n", __func__, id);

	for (i = 0; i < owl_de_get_path_num(); i++) {
		if (cur_de->paths[i].id == id) {
			path = &cur_de->paths[i];
			break;
		}
	}

	return path;
}

int owl_de_path_enable(struct owl_de_path *path, bool enable)
{
	debug("%s, path %d, enable %d\n", __func__, path->id, enable);

	if (path->info.type == OWL_DISPLAY_TYPE_DUMMY)
		return 0;

	return path->ops->enable(path, true);
}

void owl_de_path_get_info(struct owl_de_path *path,
			struct owl_de_path_info *info)
{
	debug("%s, path %d\n", __func__, path->id);
	memcpy(info, &path->info, sizeof(struct owl_de_path_info));
}

void owl_de_path_set_info(struct owl_de_path *path,
			struct owl_de_path_info *info)
{
	debug("%s, path %d\n", __func__, path->id);

	memcpy(&path->info, info, sizeof(struct owl_de_path_info));

	if (info->type == OWL_DISPLAY_TYPE_DUMMY)
		return;

	path->ops->apply_info(path);
}

void owl_de_path_set_go(struct owl_de_path *path)
{
	debug("%s, path%d\n", __func__, path->id);

	if (path->info.type == OWL_DISPLAY_TYPE_DUMMY)
		return;

	path->ops->set_go(path);
}

/*
 * video functions
 */

struct owl_de_video *owl_de_video_get_by_id(int id)
{
	struct owl_de_video *video = NULL;
	int i;

	debug("%s, type = %d\n", __func__, id);

	for (i = 0; i < owl_de_get_video_num(); i++) {
		if (cur_de->videos[i].id == id) {
			video = &cur_de->videos[i];
			break;
		}
	}

	return video;
}

int owl_de_video_set_path(struct owl_de_video *video,
			struct owl_de_path *path)
{
	if (video->path != NULL)
		return -1;

	video->path = path;

	return 0;
}

void owl_de_video_unset_path(struct owl_de_video *video)
{
	video->path = NULL;
}

int owl_de_video_enable(struct owl_de_video *video, bool enable)
{
	debug("%s, %d\n", __func__, enable);

	if (video->path == NULL) {
		error("set a path before enable/disable\n");
		return -1;
	}

	if (video->path->info.type == OWL_DISPLAY_TYPE_DUMMY)
		return 0;

	video->ops->enable(video, enable);

	return 0;
}

void owl_de_video_get_info(struct owl_de_video *video,
			struct owl_de_video_info *info)
{
	debug("%s, video %d\n", __func__, video->id);
	memcpy(info, &video->info, sizeof(struct owl_de_video_info));
}

void owl_de_video_set_info(struct owl_de_video *video,
			struct owl_de_video_info *info)
{
	debug("%s, video %d\n", __func__, video->id);

	memcpy(&video->info, info, sizeof(struct owl_de_video_info));

	if (video->path->info.type == OWL_DISPLAY_TYPE_DUMMY)
		return;

	video->ops->apply_info(video);
}


/*=============================================================================
				test code
 *===========================================================================*/

static int do_de_dump_regs(cmd_tbl_t *cmdtp, int flag,
			int argc, char *const argv[])
{
	if (cur_de && cur_de->ops && cur_de->ops->dump_regs)
		cur_de->ops->dump_regs(cur_de);

	return 0;
}

U_BOOT_CMD(
	de_dump_regs, 1, 0, do_de_dump_regs,
	"dump display engine's registers",
	""
);
