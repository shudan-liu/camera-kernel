/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _CAM_COMPAT_H_
#define _CAM_COMPAT_H_

#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/component.h>


#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE

#include <linux/msm_ion.h>
#include <linux/ion.h>
#include <linux/qcom_scm.h>
#include <linux/dma-buf.h>
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
int cam_compat_util_get_dmabuf_va(struct dma_buf *dmabuf, uintptr_t *vaddr);
void cam_compat_util_put_dmabuf_va(struct dma_buf *dmabuf, void *vaddr);
int camera_component_match_add_drivers(struct device *master_dev,
	struct component_match **match_list);
void cam_free_clear(const void *ptr);

#endif /* _CAM_COMPAT_H_ */
