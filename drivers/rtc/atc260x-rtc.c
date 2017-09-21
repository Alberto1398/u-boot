#include <power/atc260x/atc260x_core.h>
#include <dm.h>
#include <i2c.h>
#include <rtc.h>

#ifdef CONFIG_DM_PMIC_ATC2609A
#include <power/atc260x/actions_reg_atc2609a.h>
#endif

#ifdef CONFIG_DM_PMIC_ATC2603C
#include <power/atc260x/actions_reg_atc2603c.h>
#endif

/* mktime & rtc_time_to_tm : stolen from kernel source ;) */

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
static unsigned long
_mktime(const unsigned int year0, const unsigned int mon0,
	const unsigned int day, const unsigned int hour,
	const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int)(mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}
	debug("%d %d %d %d %d %d", year, mon, day, hour, min, sec);
	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
		)*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

static const unsigned char _rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static inline unsigned int _is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

/*
 * The number of days in the month.
 */
static int _rtc_month_days(unsigned int month, unsigned int year)
{
	return _rtc_days_in_month[month] + (_is_leap_year(year) && month == 1);
}

/*
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */
static void _rtc_time_to_tm(unsigned long time, struct atc260x_rtc_time *tm)
{
	unsigned int month, year;
	int days;

	days = time / 86400;
	time -= (unsigned int)days * 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	/* tm->tm_wday = (days + 4) % 7; */

	year = 1970 + days / 365;
	days -= (year - 1970) * 365 + LEAPS_THRU_END_OF(year - 1)
	    - LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + _is_leap_year(year);
	}
	tm->year = year;
	/* tm->tm_yday = days + 1; */

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - _rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->mon = month + 1;
	tm->dom = days + 1;

	tm->hour = time / 3600;
	time -= tm->hour * 3600;
	tm->min = time / 60;
	tm->sec = time - tm->min * 60;

	/* tm->tm_isdst = 0; */
}

#ifdef CONFIG_DM_PMIC_ATC2609A
static const u16 sc_atc260x_regtbl_rtc_ms = ATC2609A_RTC_MS;
static const u16 sc_atc260x_regtbl_rtc_h = ATC2609A_RTC_H;
static const u16 sc_atc260x_regtbl_rtc_ymd = ATC2609A_RTC_YMD;
static const u16 sc_atc260x_regtbl_rtc_dc = ATC2609A_RTC_DC;
static const u16 sc_atc260x_regtbl_rtc_msalm = ATC2609A_RTC_MSALM;
static const u16 sc_atc260x_regtbl_rtc_halm = ATC2609A_RTC_HALM;
static const u16 sc_atc260x_regtbl_rtc_ymdalm = ATC2609A_RTC_YMDALM;
#endif

#ifdef CONFIG_DM_PMIC_ATC2603C
static const u16 sc_atc260x_regtbl_rtc_ms = ATC2603C_RTC_MS;
static const u16 sc_atc260x_regtbl_rtc_h = ATC2603C_RTC_H;
static const u16 sc_atc260x_regtbl_rtc_ymd = ATC2603C_RTC_YMD;
static const u16 sc_atc260x_regtbl_rtc_dc = ATC2603C_RTC_DC;
static const u16 sc_atc260x_regtbl_rtc_msalm = ATC2603C_RTC_MSALM;
static const u16 sc_atc260x_regtbl_rtc_halm = ATC2603C_RTC_HALM;
static const u16 sc_atc260x_regtbl_rtc_ymdalm = ATC2603C_RTC_YMDALM;
#endif

static void _atc260x_rtc_unpack_reg(struct atc260x_rtc_time *rtc_tm,
				    unsigned int raw_rtc_ms,
				    unsigned int raw_rtc_h,
				    unsigned int raw_rtc_ymd,
				    unsigned int raw_rtc_cen)
{
	unsigned int rtc_ms, rtc_h, rtc_ymd, rtc_cen;
	rtc_ms = raw_rtc_ms & 0xfffU;
	rtc_h = raw_rtc_h & 0x1fU;
	rtc_ymd = raw_rtc_ymd;
	rtc_cen = raw_rtc_cen & 0x7fU;

	rtc_tm->year = rtc_cen * 100 + ((rtc_ymd >> 9) & 0x7fU);
	rtc_tm->mon = (rtc_ymd >> 5) & 0xfU;
	rtc_tm->dom = rtc_ymd & 0x1fU;
	rtc_tm->hour = rtc_h;
	rtc_tm->min = (rtc_ms >> 6) & 0x3fU;
	rtc_tm->sec = rtc_ms & 0x3fU;
}

static void _atc260x_rtc_pack_reg(struct atc260x_rtc_time *rtc_tm,
				  unsigned short *raw_rtc_ms,
				  unsigned short *raw_rtc_h,
				  unsigned short *raw_rtc_ymd,
				  unsigned short *raw_rtc_cen)
{
	*raw_rtc_ms = (rtc_tm->min << 6) | rtc_tm->sec;
	*raw_rtc_h = rtc_tm->hour;
	*raw_rtc_ymd =
	    ((rtc_tm->year % 100U) << 9) | (rtc_tm->mon << 5) | rtc_tm->dom;
	if (raw_rtc_cen)
		*raw_rtc_cen = rtc_tm->year / 100U;
}

static int _atc260x_rtc_set_alarm_raw(unsigned short rtc_msalm,
				      unsigned short rtc_halm,
				      unsigned short rtc_ymdalm)
{
	unsigned int reg_rtc_msalm, reg_rtc_halm, reg_rtc_ymdalm;
	int ret1, ret2, ret3;

	reg_rtc_msalm = sc_atc260x_regtbl_rtc_msalm;
	reg_rtc_halm = sc_atc260x_regtbl_rtc_halm;
	reg_rtc_ymdalm = sc_atc260x_regtbl_rtc_ymdalm;

	ret1 = atc260x_reg_write(reg_rtc_msalm, rtc_msalm);
	ret2 = atc260x_reg_write(reg_rtc_halm, rtc_halm);
	ret3 = atc260x_reg_write(reg_rtc_ymdalm, rtc_ymdalm);
	if (ret1 || ret2 || ret3) {
		pr_err("rtc write IO err\n");
		return -1;
	}
	return 0;
}

int atc260x_rtc_get_time(struct atc260x_rtc_time *rtc_tm)
{
	unsigned int reg_rtc_ms, reg_rtc_h, reg_rtc_ymd, reg_rtc_dc;
	int ret1, ret2, ret3, ret4;

	reg_rtc_ms = sc_atc260x_regtbl_rtc_ms;
	reg_rtc_h = sc_atc260x_regtbl_rtc_h;
	reg_rtc_ymd = sc_atc260x_regtbl_rtc_ymd;
	reg_rtc_dc = sc_atc260x_regtbl_rtc_dc;

	ret1 = atc260x_reg_read(reg_rtc_ms);
	ret2 = atc260x_reg_read(reg_rtc_h);
	ret3 = atc260x_reg_read(reg_rtc_ymd);
	ret4 = atc260x_reg_read(reg_rtc_dc);
	if (ret1 < 0 || ret2 < 0 || ret3 < 0 || ret4 < 0) {
		pr_err("rtc read IO err\n");
		return -1;
	}
	_atc260x_rtc_unpack_reg(rtc_tm, ret1, ret2, ret3, ret4);
	return 0;
}

int atc260x_rtc_get_alarm_time(struct atc260x_rtc_time *rtc_tm)
{
	unsigned int reg_rtc_msalm, reg_rtc_halm, reg_rtc_ymdalm, reg_rtc_dc;
	int ret1, ret2, ret3, ret4;

	reg_rtc_msalm = sc_atc260x_regtbl_rtc_msalm;
	reg_rtc_halm = sc_atc260x_regtbl_rtc_halm;
	reg_rtc_ymdalm = sc_atc260x_regtbl_rtc_ymdalm;
	reg_rtc_dc = sc_atc260x_regtbl_rtc_dc;

	ret1 = atc260x_reg_read(reg_rtc_msalm);
	ret2 = atc260x_reg_read(reg_rtc_halm);
	ret3 = atc260x_reg_read(reg_rtc_ymdalm);
	ret4 = atc260x_reg_read(reg_rtc_dc);
	if (ret1 < 0 || ret2 < 0 || ret3 < 0 || ret4 < 0) {
		pr_err("rtc read IO err\n");
		return -1;
	}
	_atc260x_rtc_unpack_reg(rtc_tm, ret1, ret2, ret3, ret4);
	return 0;
}

int atc260x_rtc_setup_alarm(unsigned int seconds, unsigned int safe_margin,
			    struct atc260x_rtc_alarm_save *p_old_alarm)
{
	struct atc260x_rtc_time rtc_tm;
	struct atc260x_rtc_time rtc_tm_alm;
	ulong rtc_times, rtc_alm_times;
	unsigned short rtc_msalm, rtc_halm, rtc_ymdalm;
	int ret;

	ret = atc260x_rtc_get_time(&rtc_tm);
	ret |= atc260x_rtc_get_alarm_time(&rtc_tm_alm);
	if (ret) {
		return ret;
	}

	rtc_times = _mktime(rtc_tm.year, rtc_tm.mon, rtc_tm.dom,
			    rtc_tm.hour, rtc_tm.min, rtc_tm.sec);
	rtc_alm_times = _mktime(rtc_tm_alm.year, rtc_tm_alm.mon, rtc_tm_alm.dom,
				rtc_tm_alm.hour, rtc_tm_alm.min,
				rtc_tm_alm.sec);

	/* save old alarm if needed. */
	if (p_old_alarm) {
		if (rtc_times < rtc_alm_times) {
			/* Alarm already activated. */
			if ((rtc_alm_times - rtc_times) <= safe_margin) {
				pr_warn
				    ("%s() old alarm will be postponed (+ ~%us)\n",
				     __func__, safe_margin);
				rtc_alm_times = rtc_times + safe_margin;
			}
			/* save old alarm */
			_rtc_time_to_tm(rtc_alm_times, &rtc_tm_alm);
			_atc260x_rtc_pack_reg(&rtc_tm_alm,
					      &p_old_alarm->msalm,
					      &p_old_alarm->halm,
					      &p_old_alarm->ymdalm, NULL);
			pr_info("%s() saved old alarm %u-%u-%u %u:%u:%u, "
				"msalm=0x%x halm=0x%x ymdalm=0x%x\n", __func__,
				rtc_tm_alm.year, rtc_tm_alm.mon, rtc_tm_alm.dom,
				rtc_tm_alm.hour, rtc_tm_alm.min, rtc_tm_alm.sec,
				p_old_alarm->msalm, p_old_alarm->halm,
				p_old_alarm->ymdalm);
		} else {
			pr_info("%s() no need to save old alarm\n", __func__);
			memset(p_old_alarm, 0, sizeof(*p_old_alarm));
		}
	}

	/* set new alarm */
	rtc_alm_times = rtc_times + seconds;
	_rtc_time_to_tm(rtc_alm_times, &rtc_tm_alm);
	_atc260x_rtc_pack_reg(&rtc_tm_alm, &rtc_msalm, &rtc_halm, &rtc_ymdalm,
			      NULL);
	ret = _atc260x_rtc_set_alarm_raw(rtc_msalm, rtc_halm, rtc_ymdalm);
	if (ret) {
		pr_err("%s() write IO err\n", __func__);
		return -1;
	}
	pr_info("%s() new alarm set to %u-%u-%u %u:%u:%u, "
		"msalm=0x%x halm=0x%x ymdalm=0x%x\n",
		__func__,
		rtc_tm_alm.year, rtc_tm_alm.mon, rtc_tm_alm.dom,
		rtc_tm_alm.hour, rtc_tm_alm.min, rtc_tm_alm.sec,
		rtc_msalm, rtc_halm, rtc_ymdalm);

	return 0;
}

int atc260x_rtc_restore_alarm(struct atc260x_rtc_alarm_save *p_old_alarm)
{
	int ret =
	    _atc260x_rtc_set_alarm_raw(p_old_alarm->msalm, p_old_alarm->halm,
				       p_old_alarm->ymdalm);
	if (ret) {
		pr_err("%s() write IO err\n", __func__);
	}
	return ret;
}

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.*/
int atc260x_rtc_tm_to_time(unsigned long *time)
{
	struct atc260x_rtc_time rtc_tm;
	int ret;

	ret = atc260x_rtc_get_time(&rtc_tm);
	if (ret)
		return ret;

	debug("%d %d %d %d %d %d", rtc_tm.year, rtc_tm.mon, rtc_tm.dom,
	       rtc_tm.hour, rtc_tm.min, rtc_tm.sec);
	*time =
	    _mktime(rtc_tm.year, rtc_tm.mon, rtc_tm.dom, rtc_tm.hour,
		    rtc_tm.min, rtc_tm.sec);

	return 0;
}

static int _atc260x_rtc_set_time_raw(unsigned short raw_rtc_ms,
				     unsigned short raw_rtc_h,
				     unsigned short raw_rtc_ymd,
				     unsigned short raw_rtc_cen)
{
	unsigned int reg_rtc_ms, reg_rtc_h, reg_rtc_ymd, reg_rtc_cen;
	int ret1, ret2, ret3, ret4;

	reg_rtc_ms = sc_atc260x_regtbl_rtc_ms;
	reg_rtc_h = sc_atc260x_regtbl_rtc_h;
	reg_rtc_ymd = sc_atc260x_regtbl_rtc_ymd;
	reg_rtc_cen = sc_atc260x_regtbl_rtc_dc;

	ret1 = atc260x_reg_write(reg_rtc_ms, raw_rtc_ms);
	ret2 = atc260x_reg_write(reg_rtc_h, raw_rtc_h);
	ret3 = atc260x_reg_write(reg_rtc_ymd, raw_rtc_ymd);
	ret4 = atc260x_reg_write(reg_rtc_cen, raw_rtc_cen);
	if (ret1 || ret2 || ret3 || ret4) {
		pr_err("rtc write IO err\n");
		return -1;
	}
	return 0;
}

/* Converts  seconds since 1970-01-01 00:00:00. to atc260x_rtc_tm and then write to reg*/
int atc260x_time_to_rtc_tm(unsigned long time)
{
	struct atc260x_rtc_time rtc_tm;
	unsigned short raw_rtc_ms, raw_rtc_h, raw_rtc_ymd, raw_rtc_cen;
	int ret;
	_rtc_time_to_tm(time, &rtc_tm);

	debug("%d %d %d %d %d %d\n", rtc_tm.year, rtc_tm.mon, rtc_tm.dom,
	       rtc_tm.hour, rtc_tm.min, rtc_tm.sec);

	_atc260x_rtc_pack_reg(&rtc_tm, &raw_rtc_ms, &raw_rtc_h, &raw_rtc_ymd,
			      &raw_rtc_cen);

	ret =
	    _atc260x_rtc_set_time_raw(raw_rtc_ms, raw_rtc_h, raw_rtc_ymd,
				      raw_rtc_cen);

	if (ret) {
		pr_err("%s() write IO err\n", __func__);
		return -1;
	}
	return 0;
}

static int atc2609a_rtc_get(struct udevice *dev, struct rtc_time *time)
{
	struct atc260x_rtc_time rtc_tm;
	atc260x_rtc_get_time(&rtc_tm);
	time->tm_sec = rtc_tm.sec;
	time->tm_min = rtc_tm.min;
	time->tm_hour = rtc_tm.hour;
	time->tm_mday = rtc_tm.dom;
	time->tm_mon = rtc_tm.mon;
	time->tm_year = rtc_tm.year;
	return 0;
}

#if 1
static int atc2609a_rtc_set(struct udevice *dev, const struct rtc_time *time)
{
	struct atc260x_rtc_time rtc_tm;
	unsigned short raw_rtc_ms, raw_rtc_h, raw_rtc_ymd, raw_rtc_cen;
	int ret;
	rtc_tm.year = time->tm_year;
	rtc_tm.mon = time->tm_mon;
	rtc_tm.dom = time->tm_mday;
	rtc_tm.hour = time->tm_hour;
	rtc_tm.min = time->tm_min;
	rtc_tm.sec = time->tm_sec;

	_atc260x_rtc_pack_reg(&rtc_tm, &raw_rtc_ms, &raw_rtc_h, &raw_rtc_ymd,
			      &raw_rtc_cen);

	ret =
	    _atc260x_rtc_set_time_raw(raw_rtc_ms, raw_rtc_h, raw_rtc_ymd,
				      raw_rtc_cen);

	if (ret) {
		pr_err("%s() write IO err\n", __func__);
		return -1;
	}
	return 0;
}
#endif

static const struct rtc_ops atc2609a_rtc_ops = {
	.get = atc2609a_rtc_get,
	.set = atc2609a_rtc_set,
};

static int atc2609a_rtc_probe(struct udevice *dev)
{
	/*printf("atc260x_rtc_probe\n"); */
	return 0;
}

static const struct udevice_id atc2609a_rtc_ids[] = {
	{.compatible = "actions,atc2609a-rtc"},
	{.compatible = "actions,atc2603c-rtc"},
	{}
};

U_BOOT_DRIVER(atc2609a_rtc) = {
		.name = "atc2609a_rtc",
		.id = UCLASS_RTC,
		.probe = atc2609a_rtc_probe,
		.of_match = atc2609a_rtc_ids,
		.ops = &atc2609a_rtc_ops,
};
