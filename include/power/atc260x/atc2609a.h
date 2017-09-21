#ifndef __ATC2609A_H__
#define __ATC2609A_H__

#define ATC2609A_LDO_DRIVER	"atc2609a_ldo"
#define ATC2609A_DCDC_DRIVER	"atc2609a_DCDC"
/* #define OWL_PMU_DBG */
#ifdef OWL_PMU_DBG
#define pmu_dbg(fmt, args...)	printf(fmt, ##args)
#define pmu_warn(fmt, args...)	printf(fmt, ##args)
#else
#define pmu_dbg(fmt, args...)
#define pmu_warn(fmt, args...) printf(fmt, ##args)
#endif

#endif
