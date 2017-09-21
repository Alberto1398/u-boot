/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009 Actions Semi Inc.
*/
/*****************************************************************************/

#ifndef __CVBS_H__
#define __CVBS_H__

/*--------------Register Address---------------------------------------*/
#define     TVOUT_BASE                                     0xE02B0000
#define     TVOUT_EN                                       (TVOUT_BASE+0x0000)
#define     TVOUT_OCR                                      (TVOUT_BASE+0x0004)
#define     TVOUT_STA                                      (TVOUT_BASE+0x0008)
#define     TVOUT_CCR                                      (TVOUT_BASE+0x000C)
#define     TVOUT_BCR                                      (TVOUT_BASE+0x0010)
#define     TVOUT_CSCR                                     (TVOUT_BASE+0x0014)
#define     TVOUT_PRL                                      (TVOUT_BASE+0x0018)
#define     CVBS_MSR                                       (TVOUT_BASE+0x0020)
#define     CVBS_AL_SEPO                                   (TVOUT_BASE+0x0024)
#define     CVBS_AL_SEPE                                   (TVOUT_BASE+0x0028)
#define     CVBS_AD_SEP                                    (TVOUT_BASE+0x002C)
#define     CVBS_HUECR                                     (TVOUT_BASE+0x0030)
#define     CVBS_SCPCR                                     (TVOUT_BASE+0x0034)
#define     CVBS_SCFCR                                     (TVOUT_BASE+0x0038)
#define     CVBS_CBACR                                     (TVOUT_BASE+0x003C)
#define     CVBS_SACR                                      (TVOUT_BASE+0x0040)
#define     TVOUT_DCR                                      (TVOUT_BASE+0x0070)
#define     TVOUT_DDCR                                     (TVOUT_BASE+0x0074)
#define     TVOUT_DCORCTL                                  (TVOUT_BASE+0x0078)
#define     TVOUT_DRCR                                     (TVOUT_BASE+0x007C)

/*****************************************************************************/
/* TVOUT_EN */
/* bit[31:1]  Reserved */
#define TVOUT_EN_CVBS_EN				(0x1 << 0)

/* TVOUT_OCR */
/* bit[2:0],bit[6:5],bit[22:13],bit[31:29]  Reserved */
#define TVOUT_OCR_PI_IRQEN				(0x1 << 12)
#define TVOUT_OCR_INACEN				(0x1 << 10)
#define TVOUT_OCR_PO_ADEN				(0x1 << 9)
#define TVOUT_OCR_PI_ADEN				(0x1 << 8)
#define TVOUT_OCR_INREN					(0x1 << 7)
#define TVOUT_OCR_DACOUT				(0x1 << 4)
#define TVOUT_OCR_DAC3					(0x1 << 3)

/* TVOUT_STA */
/* bit[2:0],bit[6:4]  Reserved */
#define TVOUT_STA_DAC3ILS				(0x1 << 3)

#define CVBS_MSR_CVCKS					(0x1 << 4)
#define CVBS_MSR_CVBS_NTSC_M				(0x0 << 0)
#define CVBS_MSR_CVBS_NTSC_J				(0x1 << 0)
#define CVBS_MSR_CVBS_PAL_D				(0x4 << 0)

/* CVBS_AL_SEPO */
/* bit[31:26]  Reserved */
#define CVBS_AL_SEPO_ALEP_MASK				(0x3FF << 16)
#define CVBS_AL_SEPO_ALEP(x)				(((x) & 0x3FF) << 16)
/* bit[15:10]  Reserved */
#define CVBS_AL_SEPO_ALSP_MASK				(0x3FF << 0)
#define CVBS_AL_SEPO_ALSP(x)				(((x) & 0x3FF) << 0)

/* CVBS_AL_SEPE */
/* bit[31:26]  Reserved */
#define CVBS_AL_SEPE_ALEPEF_MASK			(0x3FF << 16)
#define CVBS_AL_SEPE_ALEPEF(x)				(((x) & 0x3FF) << 16)
/* bit[15:10]  Reserved */
#define CVBS_AL_SEPE_ALSPEF_MASK			(0x3FF << 0)
#define CVBS_AL_SEPE_ALSPEF(x)				(((x) & 0x3FF) << 0)

/* CVBS_AD_SEP */
/* bit[31:26]  Reserved */
#define CVBS_AD_SEP_ADEP_MASK				(0x3FF << 16)
#define CVBS_AD_SEP_ADEP(x)				(((x) & 0x3FF) << 16)
/* bit[15:10]  Reserved */
#define CVBS_AD_SEP_ADSP_MASK				(0x3FF << 0)
#define CVBS_AD_SEP_ADSP(x)				(((x) & 0x3FF) << 0)

#define CMU_TVOUTPLL_PLL1EN				(1 << 11)
#define CMU_TVOUTPLL_CVBS_PLL1FSS(x)			(((x) & 0x7)  << 8)
#define CMU_TVOUTPLL_TK0SS				(1 << 6)

#endif
