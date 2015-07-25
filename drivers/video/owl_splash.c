#include <common.h>
#include <malloc.h>
#include <version.h>
#include <nand.h>
#include <mmc.h>
#include <fat.h>
#include <power/owlxx_power.h>
#include <linux/compiler.h>
#include <video.h>
#include <lcd.h>

#include <libfdt_env.h>
#include <fdtdec.h>
#include <fdt.h>
#include <libfdt.h>

#include <owlxx_dss.h>

DECLARE_GLOBAL_DATA_PTR;

extern int bat_low_flag;

#define BOOTLOGO_SEC_PATH	"boot_logo_1.bmp.gz"

/*support dual logo*/
extern int ReadExtensionType(int type, unsigned char * buf_data, int size);
void adfu_printf(char *fmt, ...)
{

}
static int dual_logo_sel(void)
{
	int ret = 0;
	int rnum, val;
	unsigned char buf[4];
	val = dual_logo_cfgval_get()& 0x0f;
	if ( val == 0 )
			return 0;
	
	const char *ifname = get_defif();
	if ( strcmp(ifname, "nand") == 0 ) {
	#if defined(CONFIG_OWLXX_NAND)
		rnum = ReadExtensionType(5, buf, 4);
		printf("bf:ret=%d, %c\n", rnum, buf[0]);
		if ( rnum >= 1 && buf[0] == '1' )
			ret =1;
	#endif

	}else {

	}
	return ret;
}
static  char * boot_sel_logo(void)
{

	if ( dual_logo_sel() )
		return BOOTLOGO_SEC_PATH;
	else
		return CONFIG_SYS_VIDEO_LOGO_NAME;
}

static  char *splash_image_select(void)
{
#if 0
	int bat_v;

	power_plug_status = atc260x_get_charge_plugin_status();

	if (bat_exist) {
		if (((0 == OWLXX_PMU_POWER_SYTLE) && (power_plug_status == USB_PC)) || 
			(OWLXX_PMU_POWER_SYTLE && (power_plug_status & USB_PC))) {
			bat_v = get_battery_voltage();
			if (bat_v < POWER_SERIOUS_LOW_LEVEL) {
				printf("~~CONFIG_SYS_BATTERY_LOW_NAME when usb-pc plug\n");
				bat_low_flag = 1;
			}
		} else if (0 == power_plug_status) {
			bat_cap = get_battery_cap();
			if (bat_cap < 0 || bat_cap > 100) {
				bat_v = get_battery_voltage();
				printf("bat_v: %d\n", bat_v);
				if (bat_v < POWER_BAT_LOW_LEVEL)
					bat_low_flag = 1;
				else
					bat_low_flag = 0;
			} else {
				if (bat_cap < OWLXX_PMU_LOW_BATTY_CAP)
					bat_low_flag = 1;
				else
					bat_low_flag = 0;
			}

			if (bat_low_flag)
				printf("~CONFIG_SYS_BATTERY_LOW_NAME when no plug\n");
		}

		if (bat_low_flag) {
			splash_image_type = BOOT_IMAGE_BATTERY_LOW;
			return CONFIG_SYS_BATTERY_LOW_NAME;
		}
	}

	if (owlxx_in_recovery() == 1) {
		printf("~~CONFIG_SYS_RECOVERY_LOGO_NAME\n");
		splash_image_type = BOOT_IMAGE_RECOVERY;
		return CONFIG_SYS_RECOVERY_LOGO_NAME;
	}

	if (!((1 == charger_boot_mode) && (owlxx_in_recovery() == 0)) || (0 == bat_exist)) {
		printf("~~CONFIG_SYS_VIDEO_LOGO_NAME\n");
		splash_image_type = BOOT_IMAGE_NORMAL;
		return CONFIG_SYS_VIDEO_LOGO_NAME;
	}	

	splash_image_type = BOOT_IMAGE_NO_PICTURE;
	printf("~~NO picture\n");
	return NULL;
#endif

	if(owlxx_in_recovery() ) {
		splash_image_type = BOOT_IMAGE_RECOVERY;
		return CONFIG_SYS_RECOVERY_LOGO_NAME;
	} else if(gd->flags & GD_FLG_CHARGER ) {	/*charger logo  by cxj*/
		return CONFIG_SYS_CHARGER_LOGO_NAME;
	} else if((gd->flags & 0x80000) == GD_FLG_LOWPOWER) {
		return CONFIG_SYS_BATTERY_LOW_NAME;   /*by cxj @2014/10/06:show low power picture*/
	} else {	
		return boot_sel_logo();
	}

}


static int splash_image_load(void)
{
	int ret;
	char *filename;
	void *splash_image_addr;
	char splash_image_char[16];

	splash_image_addr = memalign(128, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if(splash_image_addr == NULL) {
		printk("Malloc size for splash image failed!\n");
		return -1;
	}

	filename = splash_image_select();
	if (!filename) {
		printk("No splash image loaded\n");
		return -1;
	}

	ret = file_fat_read(filename, splash_image_addr, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if(ret < 0) {
		printk("Fail to load splash image\n");
		free(splash_image_addr);
		return -1;
	}
	sprintf(splash_image_char, "%x", (unsigned int) splash_image_addr);
	setenv("splashimage", splash_image_char);

}

static int lcd_gamma_load(void)
{
	int ret;
	char gamma_char[16];
	struct gamma_info *gamma_info;

	gamma_info = &kinfo->gamma;
	ret = file_fat_read(CONFIG_SYS_GAMMA_NAME, gamma_info->gamma_table,
			CONFIG_SYS_GAMMA_SIZE);
	if(ret < 0) {
		printk("Fail to load gamma\n");
		gamma_info->is_valid = 0;
		return -1;
	}

	gamma_info->is_valid = 1;
	sprintf(gamma_char, "%x", (unsigned int)gamma_info);
	setenv("lcdgamma", gamma_char);
}

int splash_image_init(void)
{
	int ret, dev;
	block_dev_desc_t *dev_desc;
	const char *ifname = get_defif();

	dev = FAT_MISC_DEV;
	if ( owlxx_in_recovery() == 1 )
		dev = FAT_RECOVERY_DEV;

#ifdef CONFIG_MMC
	if (strcmp(ifname, "mmc") == 0) {
		struct mmc *mmc = find_mmc_device(dev);

		if (!mmc) {
			printf("no mmc device at slot %x\n", dev);
			return -1;
		}

		mmc->has_init = 0;
		mmc_init(mmc);
	}
#endif /* CONFIG_MMC */

	dev_desc = get_dev(ifname, dev);
	if (dev_desc == NULL) {
		printf("Failed to find %s:%d\n", ifname, dev);
		return -1;
	}
	printf("bootlogo: if=%s, dev=%d\n",ifname, dev);

	ret = fat_register_device(dev_desc, 0);
	if (ret) {
		printf("Failed to register %s:\n",ifname);
		return -1;
	}

	splash_image_load();

	lcd_gamma_load();

	return 0;
}

int recovery_image_init(void)
{
	int ret;
	void *splash_image_addr;
	block_dev_desc_t *dev_desc;
	char splash_image_char[8];
	char *filename;
	const char *ifname = get_defif();

	printf("recovery_image_init 1 \n");

#ifdef CONFIG_MMC
	if (strcmp(ifname, "mmc") == 0) {
		struct mmc *mmc = find_mmc_device(FAT_RECOVERY_DEV);

		if (!mmc) {
			printf("no mmc device at slot %x\n", FAT_RECOVERY_DEV);
			return -1;
		}

		mmc->has_init = 0;
		mmc_init(mmc);
	}
#endif /* CONFIG_MMC */

	dev_desc = get_dev(ifname, FAT_RECOVERY_DEV);
	if (dev_desc == NULL) {
		printf("Failed to find %s:%d\n",
			ifname, FAT_RECOVERY_DEV);
		return -1;
	}

	ret = fat_register_device(dev_desc, 0);
	if (ret) {
		printf("Failed to register %s:\n",	ifname);
		return -1;
	}

	splash_image_addr = malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if(splash_image_addr == NULL) {
		printk("Malloc size for splash image failed!\n");
		return -1;
	}

	ret = file_fat_read(CONFIG_SYS_RECOVERY_LOGO_NAME, splash_image_addr, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if(ret < 0) {
		printk("Fail to load splash image\n");
		free(splash_image_addr);
		return -1;
	}

	video_display_bitmap((unsigned long)splash_image_addr,0x7FFF,0x7FFF);
	
	return 0;
}


#define CHARGE_ANIMATION_REP		2
#define CHARGE_ANIMATION_NUM_FRAMES		6

int start_charge_frame_select(int bat_cap)
{
	int cap_interval;
	int index = 0;

	cap_interval = 100 / (CHARGE_ANIMATION_NUM_FRAMES - 1);
	index = bat_cap / cap_interval;	

	return index;
}

int charge_animation_display(int bat_cap)
{
	int ret;
	void *splash_image_addr;
	block_dev_desc_t *dev_desc;
	char filename[30];
	int start_index = 0;
	int rep_times = 0;
	int i;
	const char *ifname = get_defif();

	printf("%s 1 \n", __func__);

#ifdef CONFIG_MMC
	if (strcmp(ifname, "mmc") == 0) {
		struct mmc *mmc = find_mmc_device(FAT_MISC_DEV);

		if (!mmc) {
			printf("no mmc device at slot %x\n", FAT_MISC_DEV);
			return -1;
		}

		mmc->has_init = 0;
		mmc_init(mmc);
	}
#endif /* CONFIG_MMC */

	dev_desc = get_dev(ifname, FAT_MISC_DEV);
	if (dev_desc == NULL) {
		printf("Failed to find %s:%d\n",
			ifname, FAT_MISC_DEV);
		return -1;
	}

	ret = fat_register_device(dev_desc, 0);
	if (ret) {
		printf("Failed to register %s:\n",	ifname);
		return -1;
	}

	splash_image_addr = malloc(CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
	if(splash_image_addr == NULL) {
		printf("Malloc size for splash image failed!\n");
		return -1;
	}

	start_index = start_charge_frame_select(bat_cap);


	video_clear();
	owlxx_dss_enable();

	for (rep_times = 0; rep_times < CHARGE_ANIMATION_REP; rep_times++) {
		for (i = start_index; i < CHARGE_ANIMATION_NUM_FRAMES; i++) {
			snprintf(filename, sizeof(filename), "%d_%s", i, CONFIG_SYS_CHARGER_FRAME_NAME);

			printf("%s : read file %s\n", __func__, filename);

			ret = file_fat_read(filename, splash_image_addr, CONFIG_SYS_VIDEO_LOGO_MAX_SIZE);
			if(ret < 0) {
				printf("Fail to load charge frame %d\n", i);
				free(splash_image_addr);
				return -1;
			}

			printf("%s : ready to display charger frame %d\n", __func__, i);
			video_display_bitmap((unsigned long)splash_image_addr,0x7FFF,0x7FFF);
			printf("%s : display charger frame %d OK\n", __func__, i);

			mdelay(600);
		}
	}

	free(splash_image_addr);
	return 0;
}
