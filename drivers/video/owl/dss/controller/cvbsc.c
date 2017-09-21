#define DEBUGX
#define pr_fmt(fmt) "cvbs: " fmt
#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>
#include <asm/arch/regs.h>
#include <asm/arch/clk.h>
#include <asm/arch/reset.h>
#include <asm/io.h>
#include <fs.h>
#include <asm/gpio.h>
#include <dss.h>

#include <common.h>
#include <malloc.h>

#include <video_fb.h>
#include <linux/list.h>
#include <linux/fb.h>

#include "cvbsc.h"

#define OWL_TV_MOD_PAL 0
#define OWL_TV_MOD_NTSC 1

#define CVBS_IN		1
#define CVBS_OUT	0

const char *CVBS_SETTING_MODE_PATH = "setting/setting_cvbs_mode";

struct owl_cvbs_fb_videomode {
	const char *name;	/* optional */
	int flag;
	int vid;		/*optional*/

	struct owl_videomode	mode;
};

struct cvbsc_data {
	fdt_addr_t			base;
	struct owl_display_ctrl		*ctrl;

	struct owl_cvbs_fb_videomode *mode;

	bool				cvbs_status;
};

static const struct owl_cvbs_fb_videomode cvbs_display_modes[] = {
	{
		.name = "CVBS-PAL",
		.flag = FB_MODE_IS_STANDARD,
		.vid = OWL_TV_MOD_PAL,
		.mode = {
			.xres = 720,
			.yres = 576,
			.refresh = 25,
			.pixclock = 74074,	/*pico second, 1.e-12s */
			.hfp = 16,
			.hbp = 48,
			.vfp = 2,
			.vbp = 33,
			.hsw = 1,
			.vsw = 1,
			.sync = 0,
			.vmode = DSS_VMODE_INTERLACED,
		}
	},
	{
		.name = "CVBS-NTSC",
		.flag = FB_MODE_IS_STANDARD,
		.vid = OWL_TV_MOD_NTSC,
		.mode = {
			.xres = 720,
			.yres = 480,
			.refresh = 30,
			.pixclock = 74074,	/*pico second, 1.e-12s */
			.hfp = 12,
			.hbp = 68,
			.vfp = 5,
			.vbp = 39,
			.hsw = 1,
			.vsw = 1,
			.sync = 0,
			.vmode = DSS_VMODE_INTERLACED,
		}
	},
};
#define MODE_TIMINGS_LEN	(ARRAY_SIZE(cvbs_display_modes))

struct data_fmt_param {
	const char *name;
	s32 data_fmt;
};
static struct data_fmt_param date_fmts[] = {
	{"PAL", OWL_TV_MOD_PAL},
	{"NTSC", OWL_TV_MOD_NTSC},
};

static u32 string_to_data_fmt(const char *name)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(date_fmts); i++) {
		if (!strcmp(date_fmts[i].name, name))
			return date_fmts[i].data_fmt;
	}

	return -1;
}
void print_cvbsreg(void)
{
	debug("\nfollowing list all tvout register's value!\n");
	debug("register CMU_TVOUTPLL(0x%08x) value is 0x%08x\n",
	      CMU_TVOUTPLL, readl(CMU_TVOUTPLL));
	debug("register CMU_DEVCLKEN0(0x%08x) value is 0x%08x\n",
	      CMU_DEVCLKEN0, readl(CMU_DEVCLKEN0));
	debug("register CMU_DEVCLKEN1(0x%08x) value is 0x%08x\n",
	      CMU_DEVCLKEN1, readl(CMU_DEVCLKEN1));
	debug("register CMU_DEVRST0(0x%08x) value is 0x%08x\n",
	      CMU_DEVRST0, readl(CMU_DEVRST0));
	debug("register CMU_DEVRST1(0x%08x) value is 0x%08x\n",
	      CMU_DEVRST1, readl(CMU_DEVRST1));
	debug("register TVOUT_EN(0x%08x) value is 0x%08x\n",
	      TVOUT_EN, readl(TVOUT_EN));
	debug("register TVOUT_OCR(0x%08x) value is 0x%08x\n",
	      TVOUT_OCR, readl(TVOUT_OCR));
	debug("register TVOUT_STA(0x%08x) value is 0x%08x\n",
	      TVOUT_STA, readl(TVOUT_STA));
	debug("register TVOUT_PRL(0x%08x) value is 0x%08x\n",
	      TVOUT_PRL, readl(TVOUT_PRL));
	debug("register TVOUT_CCR(0x%08x) value is 0x%08x\n",
	      TVOUT_CCR, readl(TVOUT_CCR));
	debug("register TVOUT_BCR(0x%08x) value is 0x%08x\n",
	      TVOUT_BCR, readl(TVOUT_BCR));
	debug("register TVOUT_CSCR(0x%08x) value is 0x%08x\n",
	      TVOUT_CSCR, readl(TVOUT_CSCR));
	debug("register CVBS_MSR(0x%08x) value is 0x%08x\n",
	      CVBS_MSR, readl(CVBS_MSR));
	debug("register CVBS_AL_SEPO(0x%08x) value is 0x%08x\n",
	      CVBS_AL_SEPO, readl(CVBS_AL_SEPO));
	debug("register CVBS_AL_SEPE(0x%08x) value is 0x%08x\n",
	      CVBS_AL_SEPE, readl(CVBS_AL_SEPE));
	debug("register CVBS_AD_SEP(0x%08x) value is 0x%08x\n",
	      CVBS_AD_SEP, readl(CVBS_AD_SEP));
	debug("register CVBS_HUECR(0x%08x) value is 0x%08x\n",
	      CVBS_HUECR, readl(CVBS_HUECR));
	debug("register CVBS_SCPCR(0x%08x) value is 0x%08x\n",
	      CVBS_SCPCR, readl(CVBS_SCPCR));
	debug("register CVBS_SCFCR(0x%08x) value is 0x%08x\n",
	      CVBS_SCFCR, readl(CVBS_SCFCR));
	debug("register CVBS_CBACR(0x%08x) value is 0x%08x\n",
	      CVBS_CBACR, readl(CVBS_CBACR));
	debug("register CVBS_SACR(0x%08x) value is 0x%08x\n",
	      CVBS_SACR, readl(CVBS_SACR));
	debug("register TVOUT_DCR(0x%08x) value is 0x%08x\n",
	      TVOUT_DCR, readl(TVOUT_DCR));
	debug("register TVOUT_DDCR(0x%08x) value is 0x%08x\n",
	      TVOUT_DDCR, readl(TVOUT_DDCR));
	debug("register TVOUT_DCORCTL(0x%08x) value is 0x%08x\n",
	      TVOUT_DCORCTL, readl(TVOUT_DCORCTL));
	debug("register TVOUT_DRCR(0x%08x) value is 0x%08x\n",
	      TVOUT_DRCR, readl(TVOUT_DRCR));
}

/*-----------------------configure cvbs---------------------*/
static void configure_ntsc(void)/*ntsc(480i),pll1:432M,pll0:594/1.001*/
{
	debug("%s\n", __func__);
	writel(CVBS_MSR_CVBS_NTSC_M | CVBS_MSR_CVCKS, CVBS_MSR);

	writel((readl(CVBS_AL_SEPO) & (~CVBS_AL_SEPO_ALEP_MASK)) |
			CVBS_AL_SEPO_ALEP(0x104), CVBS_AL_SEPO);
	writel((readl(CVBS_AL_SEPO) & (~CVBS_AL_SEPO_ALSP_MASK)) |
			CVBS_AL_SEPO_ALSP(0x15), CVBS_AL_SEPO);

	writel((readl(CVBS_AL_SEPE) & (~CVBS_AL_SEPE_ALEPEF_MASK)) |
			CVBS_AL_SEPE_ALEPEF(0x20b), CVBS_AL_SEPE);

	writel((readl(CVBS_AL_SEPE) & (~CVBS_AL_SEPE_ALSPEF_MASK)) |
			CVBS_AL_SEPE_ALSPEF(0x11c), CVBS_AL_SEPE);

	writel((readl(CVBS_AD_SEP) & (~CVBS_AD_SEP_ADEP_MASK)) |
			CVBS_AD_SEP_ADEP(0x2cf), CVBS_AD_SEP);
	writel((readl(CVBS_AD_SEP) & (~CVBS_AD_SEP_ADSP_MASK)) |
			CVBS_AD_SEP_ADSP(0x0), CVBS_AD_SEP);
}

static void configure_pal(void)/*pal(576i),pll1:432M,pll0:594M*/
{
	debug("%s\n", __func__);
	writel(CVBS_MSR_CVBS_PAL_D | CVBS_MSR_CVCKS, CVBS_MSR);

	writel((readl(CVBS_AL_SEPO) & (~CVBS_AL_SEPO_ALEP_MASK)) |
			CVBS_AL_SEPO_ALEP(0x136), CVBS_AL_SEPO);
	writel((readl(CVBS_AL_SEPO) & (~CVBS_AL_SEPO_ALSP_MASK)) |
			CVBS_AL_SEPO_ALSP(0x17), CVBS_AL_SEPO);
	writel((readl(CVBS_AL_SEPE) & (~CVBS_AL_SEPE_ALEPEF_MASK)) |
			CVBS_AL_SEPE_ALEPEF(0x26f), CVBS_AL_SEPE);
	writel((readl(CVBS_AL_SEPE) & (~CVBS_AL_SEPE_ALSPEF_MASK)) |
			CVBS_AL_SEPE_ALSPEF(0x150), CVBS_AL_SEPE);

	writel((readl(CVBS_AD_SEP) & (~CVBS_AD_SEP_ADEP_MASK)) |
			CVBS_AD_SEP_ADEP(0x2cf), CVBS_AD_SEP);
	writel((readl(CVBS_AD_SEP) & (~CVBS_AD_SEP_ADSP_MASK)) |
			CVBS_AD_SEP_ADSP(0x0), CVBS_AD_SEP);
}

/**
 * @tv_mode: see enum TV_MODE_TYPE
 * This function sets register for CVBS(PAL/NTSC)
 */

static int configure_cvbs(struct cvbsc_data *cvbsc)
{
	int current_vid = cvbsc->mode->vid;
	debug("%s, vid\n", __func__, current_vid);
	switch (current_vid) {
	case OWL_TV_MOD_PAL:
		configure_pal();
		break;

	case OWL_TV_MOD_NTSC:
		configure_ntsc();
		break;
	default:
		debug("error! mode not supported\n");
		return -1;
	}
	return 0;
}

void cvbs_show_colorbar(void)
{
	configure_cvbs(OWL_TV_MOD_PAL);

	/*enable color bar ,cvbs HDAC*/
	writel(readl(TVOUT_OCR) | TVOUT_OCR_DACOUT | TVOUT_OCR_DAC3
			| TVOUT_OCR_INACEN | TVOUT_OCR_INREN, TVOUT_OCR);

	writel(readl(CMU_TVOUTPLL) | CMU_TVOUTPLL_PLL1EN |
			CMU_TVOUTPLL_TK0SS | CMU_TVOUTPLL_CVBS_PLL1FSS(0x4),
			CMU_TVOUTPLL);
	/*eable cvbs output*/
	writel(readl(TVOUT_EN) | TVOUT_EN_CVBS_EN, TVOUT_EN);
}

static int cvbs_parse_property(void *blob, int node, int *bootvid)
{
	const char *resolution;
	int len;
	unsigned int vid = 0;
	debug("%s\n", __func__);

	resolution = fdt_getprop(blob, node, "mode", &len);
	debug("%s: resolution  = %s\n", __func__, resolution);

	vid = string_to_data_fmt(resolution);
	debug("boot cvbs vid =%d\n", vid);
	return 0;
}


/*
 * detecte cvbs status
 * */
static bool get_cvbs_state(struct cvbsc_data *cvbsc)
{
	unsigned int status_val, boot_status;
	uint32_t temp;

	status_val = readl(TVOUT_STA) & TVOUT_STA_DAC3ILS;
	/*
	 * the mdelay is a must, or can`t get cvbs real status
	 * */
	mdelay(600);
	status_val = readl(TVOUT_STA) & TVOUT_STA_DAC3ILS;

	if (status_val) {
		writel(boot_status, TVOUT_OCR);
		return true;
	}
	return false;
}
int cvbs_read_usr_cfg_file(const char *file_name, char *buf)
{
	char *devif, *bootdisk;
	char dev_part_str[16];

	int val = 0;

	loff_t actread;

	devif = getenv("devif");
	bootdisk = getenv("bootdisk");
	if (!devif || !bootdisk) {
		error("no device\n");
		return -1;
	}

	snprintf(dev_part_str, 16, "%s:%s", bootdisk, ANDROID_DATA_PART);

	if (fs_set_blk_dev(devif, dev_part_str, FS_TYPE_EXT) != 0) {
		error("fs_set_blk_dev (%s %s) fialed\n", devif, dev_part_str);
		return -1;
	}

	fs_read(file_name, (unsigned long)buf, 0, 64, &actread);
	if (actread <= 0) {
		error("read %s failed\n", file_name);
		return -1;
	}

	val = simple_strtoul(buf, NULL, 0);

	debug("%s: %s = %d\n", __func__, file_name, val);

	return val;
}

static int skip_atoi(char *s)
{
#define is_digit(c)    ((c) >= '0' && (c) <= '9')
	int i = 0;
	while (is_digit(*s))
		i = i * 10 + *((s)++) - '0';
	return i;
}
static void cvbsc_enable(struct cvbsc_data *cvbsc)
{
	debug("%s\n", __func__);
	writel(readl(TVOUT_EN) | TVOUT_EN_CVBS_EN, TVOUT_EN);
	writel((readl(TVOUT_OCR) | TVOUT_OCR_DAC3 | TVOUT_OCR_INREN) &
			~TVOUT_OCR_DACOUT, TVOUT_OCR);
}
static void cvbsc_auto_detected_plugin(struct cvbsc_data *cvbsc)
{
	debug("%s\n", __func__);
	uint32_t val;

	val = readl(TVOUT_OCR);
	val |= TVOUT_OCR_PI_ADEN;
	val |= TVOUT_OCR_PO_ADEN;
	val |= TVOUT_OCR_PI_IRQEN;
	writel(val, TVOUT_OCR);
	mdelay(100);
}
static void cvbsc_power_on(struct cvbsc_data *cvbsc)
{
	debug("%s\n", __func__);
	int tmp;
	/*assert reset*/
	tmp = readl(CMU_DEVRST0);
	tmp &= (~(0x1 << 3));    /* CVBS */
	writel(tmp, CMU_DEVRST0);

	/*clk enable*/
	tmp = readl(CMU_DEVCLKEN0);
	tmp |= (0x1 << 3);      /* cvbs */
	writel(tmp, CMU_DEVCLKEN0);

	/*set cvbs_pllclk*/
	writel(0x322, CMU_CVBSPLL);
	mdelay(10);
	/*configure tvout_ddcr*/
	writel(0x110050, TVOUT_DDCR);
	writel(0x7 << 8, TVOUT_PRL);
	/*disable befor registering irq handler*/
	writel(0x0, TVOUT_OCR);

	/*deasert rest*/
	tmp = readl(CMU_DEVRST0);
	tmp |= (0x1 << 3);      /* cvbs */
	writel(tmp, CMU_DEVRST0);
}
static int cvbsc_init(struct cvbsc_data *cvbsc)
{
	debug("%s, cable status %d\n", __func__, cvbsc->cvbs_status);

	/*
	 * Usually, in this function we need to check the cvbs status first,
	 * then decide whether to enable cvbs output,
	 * But now, this action was done in advance,
	 * so here probability of undetected error
	 */

	/* config cvbs and enable */
	configure_cvbs(cvbsc);

	cvbsc_enable(cvbsc);
}
int owl_cvbsc_enable(struct owl_display_ctrl *ctrl)
{
	debug("%s\n", __func__);
	struct cvbsc_data *cvbsc = owl_ctrl_get_drvdata(ctrl);

	cvbsc_init(cvbsc);

#ifdef DEBUG
	print_cvbsreg();
#endif

	return 0;
}

void owl_cvbsc_disable(struct owl_display_ctrl *ctrl)
{
	writel(readl(TVOUT_EN) &  ~TVOUT_EN_CVBS_EN, TVOUT_EN);
	writel(readl(TVOUT_OCR)
			& ~(TVOUT_OCR_DAC3 | TVOUT_OCR_INREN), TVOUT_OCR);
}
static void cvbs_update_videomode(struct cvbsc_data *cvbsc)
{
	debug("%s\n", __func__);
	int i;
	struct owl_panel *panel = cvbsc->ctrl->panel;
	struct owl_cvbs_fb_videomode *cvbs_mode;

	struct owl_videomode *mode;
	struct owl_videomode default_mode;

	/* get default mode from panel parse DTS */
	owl_panel_get_default_mode(panel, &default_mode);

	for (i = MODE_TIMINGS_LEN - 1; i >= 0; i--) {
		cvbs_mode = &cvbs_display_modes[i];
		if (cvbs_mode->mode.xres == default_mode.xres &&
		    cvbs_mode->mode.yres == default_mode.xres &&
		    cvbs_mode->mode.yres == default_mode.refresh)
			goto got_it;
	}
#ifdef DEBUG
	mode = &cvbs_mode->mode;
	debug("%d\n", mode->xres);
	debug("%d\n", mode->yres);
	debug("%d\n", mode->refresh);
	debug("%d\n", mode->pixclock);
	debug("%d\n", mode->hfp);
	debug("%d\n", mode->hbp);
	debug("%d\n", mode->vfp);
	debug("%d\n", mode->vbp);
	debug("%d\n", mode->hsw);
	debug("%d\n", mode->vsw);
	debug("%d\n", mode->sync);
	debug("%d\n", mode->vmode);
#endif
got_it:
	owl_panel_set_mode(panel, (struct owl_videomode *)&cvbs_mode->mode);

	cvbsc->mode = cvbs_mode;
}
static int owl_cvbs_add_panel(struct owl_display_ctrl *ctrl,
		struct owl_panel *panel)
{
	struct cvbsc_data *cvbsc = owl_ctrl_get_drvdata(ctrl);
	debug("%s\n", __func__);

	/*TODO*/
	cvbs_update_videomode(cvbsc);

	return 0;
}

static bool owl_cvbsc_hpd_is_panel_connected(struct owl_display_ctrl *ctrl)
{

	struct cvbsc_data *cvbsc = owl_ctrl_get_drvdata(ctrl);

	cvbsc->cvbs_status = get_cvbs_state(cvbsc);

	debug("%s: %d\n", __func__, cvbsc->cvbs_status);

	return cvbsc->cvbs_status;
}

struct owl_display_ctrl_ops owl_cvbs_ctrl_ops = {
	.add_panel = owl_cvbs_add_panel,

	.enable = owl_cvbsc_enable,
	.disable = owl_cvbsc_disable,

	.hpd_is_panel_connected = owl_cvbsc_hpd_is_panel_connected,
};
static struct owl_display_ctrl owl_cvbs_ctrl = {
	.name = "cvbs_ctrl",
	.type = OWL_DISPLAY_TYPE_CVBS,
	.ops = &owl_cvbs_ctrl_ops,
};

struct cvbsc_data owl_cvbsc_data;

int owl_cvbsc_init(const void *blob)
{
	/*------------GET CONFIG FROM SETTING------------*/
	char buf[256] = {0};
	int bootable, bootvid;
	int ret = 0;
	int node;
	int temp;
	struct cvbsc_data *cvbsc = &owl_cvbsc_data;
	debug("%s\n", __func__);

	/*dst match*/
	node = fdt_node_offset_by_compatible(blob, 0, "actions,s700-cvbs");
	if (node < 0) {
		debug("no matcht int dts\n");
		return 0;
	}
	/*parse cvbs properties form DTS*/
	cvbs_parse_property(blob, node, &bootvid);

	cvbsc->base = fdtdec_get_addr(blob, node, "reg");
	if (cvbsc->base == FDT_ADDR_T_NONE) {
		error("Cannot find cvbsc reg address\n");
		return -1;
	}
	debug("%s: base is 0x%llx\n", __func__, cvbsc->base);

/*
	if (!cvbs_read_usr_cfg_file(CVBS_SETTING_MODE_PATH, buf)) {
		bootvid = skip_atoi(&buf[0]);
		if (bootvid == 8)
			bootvid = OWL_TV_MOD_PAL;
		if (bootvid == 9)
			bootvid = OWL_TV_MOD_NTSC;
	}
	debug("vid mode =%d\n", bootvid);
*/
	cvbsc->cvbs_status = false;

	cvbsc_power_on(cvbsc);

	cvbsc_auto_detected_plugin(cvbsc);

	cvbsc->ctrl = &owl_cvbs_ctrl;
	owl_ctrl_set_drvdata(&owl_cvbs_ctrl, cvbsc);

	ret = owl_ctrl_register(&owl_cvbs_ctrl);
	if (ret < 0)
		goto err_ctrl_register;
	return 0;
err_ctrl_register:
	return ret;
}
