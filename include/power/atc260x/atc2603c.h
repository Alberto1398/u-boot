#ifndef __ATC2603C_H__
#define __ATC2603C_H__

#define ATC2603C_LDO_DRIVER	"atc2603c_ldo"
#define ATC2603C_DCDC_DRIVER	"atc2603c_DCDC"
/* #define OWL_PMU_DBG */
#ifdef OWL_PMU_DBG
#define pmu_dbg(fmt, args...)	printf(fmt, ##args)
#define pmu_warn(fmt, args...)	printf(fmt, ##args)
#else
#define pmu_dbg(fmt, args...)
#define pmu_warn(fmt, args...) printf(fmt, ##args)
#endif

#endif
