/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2014-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_COMPAT_H_
#define _CAM_COMPAT_H_

#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/component.h>
#include <linux/list_sort.h>
#include "cam_csiphy_dev.h"
#include "cam_cpastop_hw.h"
#include "cam_smmu_api.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)

#define VFL_TYPE_VIDEO VFL_TYPE_GRABBER

#endif

#include <linux/iommu.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
MODULE_IMPORT_NS(DMA_BUF);
#endif

struct cam_fw_alloc_info {
	struct device *fw_dev;
	void          *fw_kva;
	uint64_t       fw_hdl;
};

int cam_reserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length);
void cam_unreserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length);
int camera_component_match_add_drivers(struct device *master_dev,
	struct component_match **match_list);
void cam_check_iommu_faults(struct iommu_domain *domain,
	struct cam_smmu_pf_info *pf_info);
void cam_free_clear(const void *);
int cam_compat_util_get_dmabuf_va(struct dma_buf *dmabuf, uintptr_t *vaddr);
void cam_compat_util_put_dmabuf_va(struct dma_buf *dmabuf, void *vaddr);
void cam_smmu_util_iommu_custom(struct device *dev,
	dma_addr_t discard_start, size_t discard_length);
static inline int cam_get_ddr_type(void)
{
	WARN_ONCE(1, "of_fdt_get_ddrtype not implemented");
	/* Return value of of_fdt_get_ddrtype */
	return DDR_TYPE_LPDDR5;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
int cam_req_mgr_ordered_list_cmp(void *priv,
	const struct list_head *head_1, const struct list_head *head_2);
#else
int cam_req_mgr_ordered_list_cmp(void *priv,
	struct list_head *head_1, struct list_head *head_2);
#endif

int cam_compat_util_get_irq(struct cam_hw_soc_info *soc_info);

long cam_dma_buf_set_name(struct dma_buf *dmabuf, const char *name);

#ifdef CONFIG_SPECTRA_SECURE
void cam_cpastop_scm_write(struct cam_cpas_hw_errata_wa *errata_wa);
int cam_ife_notify_safe_lut_scm(bool safe_trigger);
int cam_csiphy_notify_secure_mode(struct csiphy_device *csiphy_dev,
	bool protect, int32_t offset);
#endif /* CONFIG_SPECTRA_SECURE */
int cam_actuator_driver_i2c_remove_common(struct i2c_client *client);
int cam_eeprom_i2c_driver_remove_common(struct i2c_client *client);
int cam_flash_i2c_driver_remove_common(struct i2c_client *client);
int cam_ois_i2c_driver_remove_common(struct i2c_client *client);
int cam_sensor_driver_i2c_remove_common(struct i2c_client *client);

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_actuator_driver_i2c_remove(struct i2c_client *client);
#else
static int32_t cam_actuator_driver_i2c_remove(struct i2c_client *client);
#endif

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_eeprom_i2c_driver_remove(struct i2c_client *client);
#else
static int32_t cam_eeprom_i2c_driver_remove(struct i2c_client *client);
#endif

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_flash_i2c_driver_remove(struct i2c_client *client);
#else
static int32_t cam_flash_i2c_driver_remove(struct i2c_client *client);
#endif

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_ois_i2c_driver_remove(struct i2c_client *client);
#else
static int32_t cam_ois_i2c_driver_remove(struct i2c_client *client);
#endif

#if KERNEL_VERSION(6, 1, 0) <= LINUX_VERSION_CODE
void cam_sensor_driver_i2c_remove(struct i2c_client *client);
#else
static int32_t cam_sensor_driver_i2c_remove(struct i2c_client *client);
#endif

#endif /* _CAM_COMPAT_H_ */
