/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#ifndef __SONOFF_IHOST_H
#define __SONOFF_IHOST_H

#define ROCKCHIP_DEVICE_SETTINGS \
		"stdout=serial\0" \
		"stderr=serial\0"

#include <configs/rv1126_common.h>

#undef BOOT_TARGET_DEVICES

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1)

#endif /* __NEURAL_COMPUTE_MODULE_2_H */
