/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _CAM_COMPAT_H_
#define _CAM_COMPAT_H_

#include <linux/version.h>
#include <linux/platform_device.h>


#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE

#include <linux/msm_ion.h>
#include <linux/ion.h>
#include <linux/qcom_scm.h>

#else

#include <linux/msm_ion.h>
#include <linux/ion_kernel.h>
#include <soc/qcom/scm.h>

#endif


struct cam_fw_alloc_info {
	struct device *fw_dev;
	void __iomem  *fw_kva;
	uint64_t       fw_hdl;
};

int cam_reserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length);
void cam_unreserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length);


#endif /* _CAM_COMPAT_H_ */
