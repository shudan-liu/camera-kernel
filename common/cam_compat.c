// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/dma-mapping.h>
#include <linux/dma-buf.h>
#include <linux/of_address.h>
#include <linux/slab.h>

#include "cam_compat.h"
#include "cam_debug_util.h"
#include "cam_cpas_api.h"
#include "camera_main.h"

int cam_reserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length)
{
	int rc = 0;
	struct device_node *of_node;
	struct device_node *mem_node;
	struct resource     res;

	of_node = (icp_fw->fw_dev)->of_node;
	mem_node = of_parse_phandle(of_node, "memory-region", 0);
	if (!mem_node) {
		rc = -ENOMEM;
		CAM_ERR(CAM_SMMU, "FW memory carveout not found");
		goto end;
	}
	rc = of_address_to_resource(mem_node, 0, &res);
	of_node_put(mem_node);
	if (rc < 0) {
		CAM_ERR(CAM_SMMU, "Unable to get start of FW mem carveout");
		goto end;
	}
	icp_fw->fw_hdl = res.start;
	icp_fw->fw_kva = ioremap_wc(icp_fw->fw_hdl, fw_length);
	if (!icp_fw->fw_kva) {
		CAM_ERR(CAM_SMMU, "Failed to map the FW.");
		rc = -ENOMEM;
		goto end;
	}
	memset_io(icp_fw->fw_kva, 0, fw_length);

end:
	return rc;
}

void cam_unreserve_icp_fw(struct cam_fw_alloc_info *icp_fw, size_t fw_length)
{
	iounmap(icp_fw->fw_kva);
}

#ifdef CONFIG_SPECTRA_SECURE
int cam_ife_notify_safe_lut_scm(bool safe_trigger)
{
	const uint32_t smmu_se_ife = 0;
	uint32_t camera_hw_version, rc = 0;

	rc = cam_cpas_get_cpas_hw_version(&camera_hw_version);
	if (!rc && qcom_scm_smmu_notify_secure_lut(smmu_se_ife, safe_trigger)) {
		switch (camera_hw_version) {
		case CAM_CPAS_TITAN_170_V100:
		case CAM_CPAS_TITAN_170_V110:
		case CAM_CPAS_TITAN_175_V100:
			CAM_ERR(CAM_ISP, "scm call to enable safe failed");
			rc = -EINVAL;
			break;
		default:
			break;
		}
	}

	return rc;
}

int cam_csiphy_notify_secure_mode(struct csiphy_device *csiphy_dev,
	bool protect, int32_t offset)
{
	int rc = 0;

	if (offset >= CSIPHY_MAX_INSTANCES_PER_PHY) {
		CAM_ERR(CAM_CSIPHY, "Invalid CSIPHY offset");
		rc = -EINVAL;
	} else if (qcom_scm_camera_protect_phy_lanes(protect,
			csiphy_dev->csiphy_info[offset]
				.csiphy_cpas_cp_reg_mask)) {
		CAM_ERR(CAM_CSIPHY, "SCM call to hypervisor failed");
		rc = -EINVAL;
	}

	return rc;
}

void cam_cpastop_scm_write(struct cam_cpas_hw_errata_wa *errata_wa)
{
	int reg_val;

	qcom_scm_io_readl(errata_wa->data.reg_info.offset, &reg_val);
	reg_val |= errata_wa->data.reg_info.value;
	qcom_scm_io_writel(errata_wa->data.reg_info.offset, reg_val);
}
#endif /* CONFIG_SPECTRA_SECURE */

static int camera_platform_compare_dev(struct device *dev, const void *data)
{
	return platform_bus_type.match(dev, (struct device_driver *) data);
}

void cam_free_clear(const void * ptr)
{
	kfree_sensitive(ptr);
}

/* Callback to compare device from match list before adding as component */
static inline int camera_component_compare_dev(struct device *dev, void *data)
{
	return dev == data;
}

/* Add component matches to list for master of aggregate driver */
int camera_component_match_add_drivers(struct device *master_dev,
	struct component_match **match_list)
{
	int i, rc = 0;
	struct platform_device *pdev = NULL;
	struct device *start_dev = NULL, *match_dev = NULL;

	if (!master_dev || !match_list) {
		CAM_ERR(CAM_UTIL, "Invalid parameters for component match add");
		rc = -EINVAL;
		goto end;
	}

	for (i = 0; i < ARRAY_SIZE(cam_component_drivers); i++) {
		struct device_driver const *drv =
			&cam_component_drivers[i]->driver;
		const void *drv_ptr = (const void *)drv;
		start_dev = NULL;
		while ((match_dev = bus_find_device(&platform_bus_type,
			start_dev, drv_ptr, &camera_platform_compare_dev))) {
			put_device(start_dev);
			pdev = to_platform_device(match_dev);
			CAM_DBG(CAM_UTIL, "Adding matched component:%s", pdev->name);
			component_match_add(master_dev, match_list,
				camera_component_compare_dev, match_dev);
			start_dev = match_dev;
		}
		put_device(start_dev);
	}

end:
	return rc;
}

struct iommu_fault_ids {
	int bid;
	int pid;
	int mid;
};
static int iommu_get_fault_ids(struct iommu_domain *domain, struct iommu_fault_ids *fault_ids)
{
	WARN_ONCE(1, "iommu_get_fault_ids unimplemented");
	return 0;
}
void cam_check_iommu_faults(struct iommu_domain *domain,
	struct cam_smmu_pf_info *pf_info)
{
	struct iommu_fault_ids fault_ids = {0, 0, 0};

	if (iommu_get_fault_ids(domain, &fault_ids))
		CAM_ERR(CAM_SMMU, "Error: Can not get smmu fault ids");

	CAM_ERR(CAM_SMMU, "smmu fault ids bid:%d pid:%d mid:%d",
		fault_ids.bid, fault_ids.pid, fault_ids.mid);

	pf_info->bid = fault_ids.bid;
	pf_info->pid = fault_ids.pid;
	pf_info->mid = fault_ids.mid;
}

static int inline cam_subdev_list_cmp(struct cam_subdev *entry_1, struct cam_subdev *entry_2)
{
	if (entry_1->close_seq_prior > entry_2->close_seq_prior)
		return 1;
	else if (entry_1->close_seq_prior < entry_2->close_seq_prior)
		return -1;
	else
		return 0;
}

int cam_compat_util_get_dmabuf_va(struct dma_buf *dmabuf, uintptr_t *vaddr)
{
	struct iosys_map mapping;
	int error_code = dma_buf_vmap(dmabuf, &mapping);

	if (error_code) {
		*vaddr = 0;
	} else {
		*vaddr = (mapping.is_iomem) ?
			(uintptr_t)mapping.vaddr_iomem : (uintptr_t)mapping.vaddr;
		CAM_DBG(CAM_MEM,
			"dmabuf=%p, *vaddr=%p, is_iomem=%d, vaddr_iomem=%p, vaddr=%p",
			dmabuf, *vaddr, mapping.is_iomem, mapping.vaddr_iomem, mapping.vaddr);
	}

	return error_code;
}

void cam_compat_util_put_dmabuf_va(struct dma_buf *dmabuf, void *vaddr)
{
	struct iosys_map mapping = IOSYS_MAP_INIT_VADDR(vaddr);

	dma_buf_vunmap(dmabuf, &mapping);
}

void cam_smmu_util_iommu_custom(struct device *dev,
	dma_addr_t discard_start, size_t discard_length)
{
	return;
}

int cam_req_mgr_ordered_list_cmp(void *priv,
	const struct list_head *head_1, const struct list_head *head_2)
{
	return cam_subdev_list_cmp(list_entry(head_1, struct cam_subdev, list),
		list_entry(head_2, struct cam_subdev, list));
}

long cam_dma_buf_set_name(struct dma_buf *dmabuf, const char *name)
{
	return 0;
}

int cam_compat_util_get_irq(struct cam_hw_soc_info *soc_info)
{
	int rc = 0;

	soc_info->irq_num = platform_get_irq(soc_info->pdev, 0);
	if (soc_info->irq_num < 0) {
		rc = soc_info->irq_num;
		return rc;
	}

	return rc;
}

