// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/of_device.h>
#include "ais_ife_csid_core.h"
#include "ais_ife_csid_dev.h"
#include "ais_ife_csid_hw_intf.h"
#include "cam_debug_util.h"
#include "ais_main.h"

static struct cam_hw_intf *ais_ife_csid_hw_list[AIS_IFE_CSID_HW_RES_MAX] = {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static char csid_dev_name[8];

static int ais_ife_csid_dev_component_bind(struct device *dev,
	struct device *master_dev, void *data)
{

	struct cam_hw_intf             *csid_hw_intf;
	struct cam_hw_info             *csid_hw_info;
	struct ais_ife_csid_hw         *csid_dev = NULL;
	const struct of_device_id      *match_dev = NULL;
	struct ais_ife_csid_hw_info    *csid_hw_data = NULL;
	uint32_t                        csid_dev_idx;
	int                             rc = 0;
	struct platform_device *pdev = to_platform_device(dev);

	CAM_DBG(CAM_ISP, "CSID component bind called");

	csid_hw_intf = kzalloc(sizeof(*csid_hw_intf), GFP_KERNEL);
	if (!csid_hw_intf) {
		rc = -ENOMEM;
		goto err;
	}

	csid_hw_info = kzalloc(sizeof(struct cam_hw_info), GFP_KERNEL);
	if (!csid_hw_info) {
		rc = -ENOMEM;
		goto free_hw_intf;
	}

	csid_dev = kzalloc(sizeof(struct ais_ife_csid_hw), GFP_KERNEL);
	if (!csid_dev) {
		rc = -ENOMEM;
		goto free_hw_info;
	}

	/* get ife csid hw index */
	of_property_read_u32(pdev->dev.of_node, "cell-index", &csid_dev_idx);
	/* get ife csid hw information */
	match_dev = of_match_device(pdev->dev.driver->of_match_table,
		&pdev->dev);
	if (!match_dev) {
		CAM_ERR(CAM_ISP, "No matching table for the IFE CSID HW!");
		rc = -EINVAL;
		goto free_dev;
	}

	CAM_DBG(CAM_ISP, "CSID%d component bind called", csid_dev_idx);

	memset(csid_dev_name, 0, sizeof(csid_dev_name));
	snprintf(csid_dev_name, sizeof(csid_dev_name),
		"csid%1u", csid_dev_idx);

	csid_hw_intf->hw_idx = csid_dev_idx;
	csid_hw_intf->hw_type = AIS_ISP_HW_TYPE_IFE_CSID;
	csid_hw_intf->hw_priv = csid_hw_info;

	csid_hw_info->core_info = csid_dev;
	csid_hw_info->soc_info.pdev = pdev;
	csid_hw_info->soc_info.dev = &pdev->dev;
	csid_hw_info->soc_info.dev_name = csid_dev_name;
	csid_hw_info->soc_info.index = csid_dev_idx;

	csid_hw_data = (struct ais_ife_csid_hw_info  *)match_dev->data;
	/* need to setup the pdev before call the ife hw probe init */
	csid_dev->csid_info = csid_hw_data;

	rc = ais_ife_csid_hw_probe_init(csid_hw_intf, csid_dev_idx);
	if (rc)
		goto free_dev;

	platform_set_drvdata(pdev, csid_dev);
	CAM_DBG(CAM_ISP, "CSID:%d component bound successful",
		csid_hw_intf->hw_idx);

	if (csid_hw_intf->hw_idx < AIS_IFE_CSID_HW_RES_MAX)
		ais_ife_csid_hw_list[csid_hw_intf->hw_idx] = csid_hw_intf;
	else
		goto free_dev;

	return 0;

free_dev:
	kfree(csid_dev);
free_hw_info:
	kfree(csid_hw_info);
free_hw_intf:
	kfree(csid_hw_intf);
err:
	return rc;
}

static void ais_ife_csid_dev_component_unbind(struct device *dev,
	struct device *master_dev, void *data)
{
	struct ais_ife_csid_hw         *csid_dev = NULL;
	struct cam_hw_intf             *csid_hw_intf;
	struct cam_hw_info             *csid_hw_info;
	struct platform_device *pdev = to_platform_device(dev);

	csid_dev = (struct ais_ife_csid_hw *)platform_get_drvdata(pdev);
	csid_hw_intf = csid_dev->hw_intf;
	csid_hw_info = csid_dev->hw_info;

	CAM_DBG(CAM_ISP, "CSID:%d remove",
		csid_dev->hw_intf->hw_idx);

	ais_ife_csid_hw_deinit(csid_dev);

	/*release the csid device memory */
	kfree(csid_dev);
	kfree(csid_hw_info);
	kfree(csid_hw_intf);
}

int ais_ife_csid_hw_init(struct cam_hw_intf **ife_csid_hw,
		struct ais_isp_hw_init_args *init)
{
	int rc = 0;

	if (ais_ife_csid_hw_list[init->hw_idx]) {
		struct cam_hw_info *csid_hw_info = NULL;
		struct ais_ife_csid_hw *core_info = NULL;

		csid_hw_info = ais_ife_csid_hw_list[init->hw_idx]->hw_priv;
		core_info = (struct ais_ife_csid_hw *)csid_hw_info->core_info;

		core_info->event_cb = init->event_cb;
		core_info->event_cb_priv = init->event_cb_priv;
		*ife_csid_hw = ais_ife_csid_hw_list[init->hw_idx];
	} else {
		*ife_csid_hw = NULL;
		rc = -1;
	}

	return rc;
}

static const struct component_ops ais_ife_csid_dev_component_ops = {
	.bind = ais_ife_csid_dev_component_bind,
	.unbind = ais_ife_csid_dev_component_unbind,
};

int ais_ife_csid_probe(struct platform_device *pdev)
{
	int rc = 0;

	CAM_DBG(CAM_ISP, "Adding CSID dev component");
	rc = component_add(&pdev->dev, &ais_ife_csid_dev_component_ops);
	if (rc)
		CAM_ERR(CAM_ISP, "failed to add component rc: %d", rc);

	return rc;
}

int ais_ife_csid_remove(struct platform_device *pdev)
{
	component_del(&pdev->dev, &ais_ife_csid_dev_component_ops);
	return 0;
}
