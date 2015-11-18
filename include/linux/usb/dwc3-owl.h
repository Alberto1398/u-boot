/*
 * Actions OWL SoCs dwc3 driver
 *
 * Copyright (c) 2015 Actions Semiconductor Co., Ltd.
 * tangshaoqing <tangshaoqing@actions-semi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DWC3_OWL_H_
#define __DWC3_OWL_H_

struct dwc3_port_info {
	void __iomem *sps_pg_ctl;
	void __iomem *sps_pg_ack;
	void __iomem *sps_ldo_ctl;

	void __iomem *usbpll;
	void __iomem *cmu_assistpll;
	void __iomem *devrst;

	void __iomem *usbecs;
};


struct dwc3_owl {
	struct dwc3_port_info port_info;
};

#endif /* __DWC3_OWL_H_ */
