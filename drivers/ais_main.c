// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/module.h>
#include <linux/build_bug.h>

#include "cam_req_mgr_dev.h"
#include "cam_sync_api.h"
#include "cam_smmu_api.h"
#include "cam_cpas_hw_intf.h"
#include "cam_cdm_intf_api.h"

#include "cam_ife_csid_dev.h"
#include "cam_vfe.h"
#include "cam_isp_dev.h"
#include "ais_ife_dev.h"
#include "ais_ife_csid_dev.h"
#include "ais_ife_csid_lite17x.h"
#include "ais_vfe17x.h"

#include "cam_res_mgr_api.h"
#include "cam_cci_dev.h"
#include "cam_sensor_dev.h"
#include "cam_actuator_dev.h"
#include "cam_csiphy_dev.h"
#include "cam_eeprom_dev.h"
#include "cam_ois_dev.h"
#include "cam_flash_dev.h"

#include "a5_core.h"
#include "ipe_core.h"
#include "bps_core.h"
#include "cam_icp_subdev.h"

#include "jpeg_dma_core.h"
#include "jpeg_enc_core.h"
#include "cam_jpeg_dev.h"

#include "cam_fd_hw_intf.h"
#include "cam_fd_dev.h"

#include "cam_lrme_hw_intf.h"
#include "cam_lrme_dev.h"

#include "cam_debug_util.h"

#include "v4l2loopback.h"

#include "ais_main.h"

struct camera_submodule_component {
	int (*init)(void);
	void (*exit)(void);
};

struct camera_submodule {
	char *name;
	uint num_component;
	const struct camera_submodule_component *component;
};

static const struct camera_submodule_component camera_base[] = {
#ifdef CONFIG_MSM_AIS
	{&cam_req_mgr_init, &cam_req_mgr_exit},
	{&cam_sync_init, &cam_sync_exit},
	{&cam_smmu_init_module, &cam_smmu_exit_module},
	{&cam_cpas_dev_init_module, &cam_cpas_dev_exit_module},
	{&cam_cdm_intf_init_module, &cam_cdm_intf_exit_module},
	{&cam_hw_cdm_init_module, &cam_hw_cdm_exit_module},
#endif
};

static const struct camera_submodule_component camera_isp[] = {
#ifdef CONFIG_SPECTRA_ISP
	{&ais_ife_csid17x_init_module, &ais_ife_csid17x_exit_module},
	{&ais_ife_csid_lite_init_module, &ais_ife_csid_lite_exit_module},
	{&ais_vfe_init_module, &ais_vfe_exit_module},
	{&ais_ife_dev_init_module, &ais_ife_dev_exit_module},
	{&cam_ife_csid17x_init_module, &cam_ife_csid17x_exit_module},
	{&cam_ife_csid_lite_init_module, &cam_ife_csid_lite_exit_module},
	{&cam_vfe_init_module, &cam_vfe_exit_module},
	{&cam_isp_dev_init_module, &cam_isp_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_sensor[] = {
#ifdef CONFIG_SPECTRA_SENSOR
	{&cam_res_mgr_init, &cam_res_mgr_exit},
	{&cam_cci_init_module, &cam_cci_exit_module},
	{&cam_csiphy_init_module, &cam_csiphy_exit_module},
#ifdef CONFIG_MSM_AIS_ACTUATOR
	{&cam_actuator_driver_init, &cam_actuator_driver_exit},
#endif
	{&cam_sensor_driver_init, &cam_sensor_driver_exit},
#ifdef CONFIG_MSM_AIS_EEPROM
	{&cam_eeprom_driver_init, &cam_eeprom_driver_exit},
#endif
#ifdef CONFIG_MSM_AIS_OIS
	{&cam_ois_driver_init, &cam_ois_driver_exit},
#endif
#ifdef CONFIG_MSM_AIS_FLASH
	{&cam_flash_init_module, &cam_flash_exit_module},
#endif
#endif
};

static const struct camera_submodule_component camera_icp[] = {
#ifdef CONFIG_SPECTRA_ICP
	{&cam_a5_init_module, &cam_a5_exit_module},
	{&cam_ipe_init_module, &cam_ipe_exit_module},
	{&cam_bps_init_module, &cam_bps_exit_module},
	{&cam_icp_init_module, &cam_icp_exit_module},
#endif
};

static const struct camera_submodule_component camera_jpeg[] = {
#ifdef CONFIG_SPECTRA_JPEG
	{&cam_jpeg_enc_init_module, &cam_jpeg_enc_exit_module},
	{&cam_jpeg_dma_init_module, &cam_jpeg_dma_exit_module},
	{&cam_jpeg_dev_init_module, &cam_jpeg_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_fd[] = {
#ifdef CONFIG_SPECTRA_FD
	{&cam_fd_hw_init_module, &cam_fd_hw_exit_module},
	{&cam_fd_dev_init_module, &cam_fd_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_lrme[] = {
#ifdef CONFIG_SPECTRA_LRME
	{&cam_lrme_hw_init_module, &cam_lrme_hw_exit_module},
	{&cam_lrme_dev_init_module, &cam_lrme_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_v4l2loopback[] = {
#ifdef CONFIG_V4L2_LOOPBACK_V2
	{&v4l2loopback_init_module, &v4l2loopback_cleanup_module},
#endif
};

static const struct camera_submodule submodule_table[] = {
	{
		.name = "Camera BASE",
		.num_component = ARRAY_SIZE(camera_base),
		.component = camera_base,
	},
	{
		.name = "Camera ISP",
		.num_component = ARRAY_SIZE(camera_isp),
		.component = camera_isp,
	},
	{
		.name = "Camera SENSOR",
		.num_component = ARRAY_SIZE(camera_sensor),
		.component = camera_sensor
	},
	{
		.name = "Camera ICP",
		.num_component = ARRAY_SIZE(camera_icp),
		.component = camera_icp,
	},
	{
		.name = "Camera JPEG",
		.num_component = ARRAY_SIZE(camera_jpeg),
		.component = camera_jpeg,
	},
	{
		.name = "Camera FD",
		.num_component = ARRAY_SIZE(camera_fd),
		.component = camera_fd,
	},
	{
		.name = "Camera LRME",
		.num_component = ARRAY_SIZE(camera_lrme),
		.component = camera_lrme,
	},
	{
		.name = "Camera V4L2loopback_v2",
		.num_component = ARRAY_SIZE(camera_v4l2loopback),
		.component = camera_v4l2loopback,
	}
};

static int camera_verify_submodules(void)
{
	int rc = 0;
	int i, j, num_components;

	for (i = 0; i < ARRAY_SIZE(submodule_table); i++) {
		num_components = submodule_table[i].num_component;
		for (j = 0; j < num_components; j++) {
			if (!submodule_table[i].component[j].init ||
				!submodule_table[i].component[j].exit) {
				CAM_ERR(CAM_UTIL,
					"%s module has init = %ps, exit = %ps",
					submodule_table[i].name,
					submodule_table[i].component[j].init,
					submodule_table[i].component[j].exit);
				rc = -EINVAL;
				goto end;
			}
		}
	}

end:
	return rc;
}

static void __camera_exit(int i, int j)
{
	uint num_exits;

	/* Exit from current submodule */
	for (j -= 1; j >= 0; j--)
		submodule_table[i].component[j].exit();

	/* Exit remaining submodules */
	for (i -= 1; i >= 0; i--) {
		num_exits = submodule_table[i].num_component;
		for (j = num_exits - 1; j >= 0; j--)
			submodule_table[i].component[j].exit();
	}
}

static int camera_init(void)
{
	int rc;
	uint i, j, num_inits;

	rc = camera_verify_submodules();
	if (rc)
		goto end_init;

	/* For Probing all available submodules */
	for (i = 0; i < ARRAY_SIZE(submodule_table); i++) {
		num_inits = submodule_table[i].num_component;
		CAM_DBG(CAM_UTIL, "Number of %s components: %u",
			submodule_table[i].name, num_inits);
		for (j = 0; j < num_inits; j++) {
			rc = submodule_table[i].component[j].init();
			if (rc) {
				CAM_ERR(CAM_UTIL,
					"%s module failure at %ps rc = %d",
					submodule_table[i].name,
					submodule_table[i].component[j].init,
					rc);
				__camera_exit(i, j);
				goto end_init;
			}
		}
	}

	CAM_DBG(CAM_UTIL, "AIS camera driver initcalls done");

end_init:
	return rc;
}

static void camera_exit(void)
{
	__camera_exit(ARRAY_SIZE(submodule_table), 0);

	CAM_INFO(CAM_UTIL, "AIS camera driver exited!");
}

module_init(camera_init);
module_exit(camera_exit);

MODULE_DESCRIPTION("AIS camera driver");
MODULE_LICENSE("GPL v2");