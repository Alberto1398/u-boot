#ifndef __ATC260X_CORE_H__
#define __ATC260X_CORE_H__

#include <power/atc260x/owl_atc260x.h>
#include <common.h>

struct atc260x_pmu_initdata {
	char dcdc_en_bm;
	char dcdc_cfg[7];
	unsigned short ldo_en_bm;
	char ldo_cfg[14];
};

/* ATC260X device */
struct atc260x_dev {
	struct owl_i2c_dev *i2c_client;
	void *spi_client;

	char dev_id;
	char ic_type;		/* see ATC260X_ICTYPE_2603A ... */
	char ic_ver;		/* see ATC260X_ICVER_A ... */
	char bus_num;		/* SPI / I2C bus number, used by reg direct access. */
	char bus_addr;		/* device's bus address, only for I2C, 7bit, r/w bit excluded */
	char s2_resume;

	int (*reg_read) (struct atc260x_dev *atc260x, uint reg);
	int (*reg_write) (struct atc260x_dev *atc260x, uint reg, u16 val);

	unsigned long s2_resume_addr;

	unsigned long auxadc_data[16 / sizeof(ulong)];

	unsigned int _obj_type_id;
	struct udevice *udev;
};

#define ATC260x_PARENT_OBJ_TYPE_ID 0x72f80927U
/* for debug */
#ifdef OWL_PMU_CORE_DEBUG

#define dev_err(DEV, FMT, ARGS...)  printf("PMU_ERROR:" FMT, ## ARGS)
#define dev_warn(DEV, FMT, ARGS...) printf("PMU_WARN:" FMT, ## ARGS)
#define dev_info(DEV, FMT, ARGS...) printf("PMU_INFO:" FMT, ## ARGS)
#define dev_dbg(DEV, FMT, ARGS...)  do {} while (0)

#define pr_err(FMT, ARGS...)        printf("PMU:" FMT, ## ARGS)
#define pr_warn(FMT, ARGS...)       printf("PMU:" FMT, ## ARGS)
#define pr_info(FMT, ARGS...)       printf("PMU:" FMT, ## ARGS)
#define pr_dbg(FMT, ARGS...)        do {} while (0)

#else /* no debug */

#define dev_err(DEV, FMT, ARGS...)  printf(FMT, ## ARGS)
#define dev_warn(DEV, FMT, ARGS...) do {} while (0)
#define dev_info(DEV, FMT, ARGS...) do {} while (0)
#define dev_dbg(DEV, FMT, ARGS...)  do {} while (0)
#define pr_err(FMT, ARGS...)        printf(FMT, ## ARGS)
#define pr_warn(FMT, ARGS...)       do {} while (0)
#define pr_info(FMT, ARGS...)       do {} while (0)
#define pr_dbg(FMT, ARGS...)        do {} while (0)

#endif

extern int atc260x_i2c_if_init(struct atc260x_dev *atc260x);

#endif /* __ATC260X_CORE_H__ */
