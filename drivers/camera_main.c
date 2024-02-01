// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/module.h>
#include <linux/build_bug.h>

#include "cam_req_mgr_dev.h"
#include "cam_sync_api.h"
#include "cam_smmu_api.h"
#include "cam_cpas_hw_intf.h"
#include "cam_cdm_intf_api.h"
#include "cam_rpmsg.h"

#include "cam_ife_csid_dev.h"
#include "cam_ife_virt_csid_mod.h"
#include "cam_vfe.h"
#include "cam_sfe_dev.h"
#include "cam_isp_dev.h"

#include "cam_res_mgr_api.h"
#include "cam_cci_dev.h"
#include "cam_sensor_dev.h"
#include "cam_actuator_dev.h"
#include "cam_csiphy_dev.h"
#include "cam_eeprom_dev.h"
#include "cam_ois_dev.h"
#include "cam_tpg_dev.h"
#include "cam_sensor_lite_dev.h"
#include "cam_csiphy_remote_dev.h"
#include "cam_flash_dev.h"

#include "a5_core.h"
#include "lx7_dev.h"
#include "ipe_core.h"
#include "bps_core.h"
#include "cam_icp_subdev.h"

#include "jpeg_dma_core.h"
#include "jpeg_enc_core.h"
#include "cam_jpeg_dev.h"

#include "cre_core.h"
#include "cam_cre_dev.h"

#include "cam_fd_hw_intf.h"
#include "cam_fd_dev.h"

#include "cam_lrme_hw_intf.h"
#include "cam_lrme_dev.h"

#include "cam_custom_dev.h"
#include "cam_custom_csid_dev.h"
#include "cam_custom_sub_mod_dev.h"

#include "cam_debug_util.h"

#include "ope_dev_intf.h"
#include "cre_dev_intf.h"

#include "cam_tfe_dev.h"
#include "cam_tfe_csid.h"
#include "cam_csid_ppi100.h"
#include "camera_main.h"

#include "cam_generated_h"

#ifdef CONFIG_V4L2_LOOPBACK
#include "v4l2loopback.h"
#endif

const char camera_banner[] = "Camera-Banner: (" CAMERA_COMPILE_BY "@"
	CAMERA_COMPILE_HOST ") (" CAMERA_COMPILE_TIME ")";

#ifdef CONFIG_CAM_PRESIL
extern int cam_presil_framework_dev_init_from_main(void);
extern void cam_presil_framework_dev_exit_from_main(void);
#endif

struct camera_submodule_component {
	int (*init)(void);
	void (*exit)(void);
};

struct camera_submodule {
	char *name;
	uint num_component;
	const struct camera_submodule_component *component;
};

#ifdef CAMERA_BUILD_FOR_AUTO
static const struct camera_submodule_component camera_base[] = {
#ifdef CONFIG_SPECTRA_CORE
	{&cam_req_mgr_init, &cam_req_mgr_exit},
	{&cam_rpmsg_init, &cam_rpmsg_exit},
	{&cam_sync_init, &cam_sync_exit},
	{&cam_smmu_init_module, &cam_smmu_exit_module},
	{&cam_cpas_dev_init_module, &cam_cpas_dev_exit_module},
	{&cam_cdm_intf_init_module, &cam_cdm_intf_exit_module},
	{&cam_hw_cdm_init_module, &cam_hw_cdm_exit_module},
#endif
};

static const struct camera_submodule_component camera_sensor[] = {
#ifdef CONFIG_SPECTRA_SENSOR
	{&cam_res_mgr_init, &cam_res_mgr_exit},
	{&cam_cci_init_module, &cam_cci_exit_module},
	{&cam_csiphy_init_module, &cam_csiphy_exit_module},
	{&cam_sensor_driver_init, &cam_sensor_driver_exit},
#endif
};

static const struct camera_submodule_component camera_isp[] = {
#ifdef CONFIG_SPECTRA_ISP
	{&cam_ife_csid_lite_init_module, &cam_ife_csid_lite_exit_module},
	{&cam_vfe_init_module, &cam_vfe_exit_module},
	{&cam_isp_dev_init_module, &cam_isp_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_v4l2loopback[] = {
#ifdef CONFIG_V4L2_LOOPBACK
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
		.name = "Camera SENSOR",
		.num_component = ARRAY_SIZE(camera_sensor),
		.component = camera_sensor
	},
	{
		.name = "Camera ISP",
		.num_component = ARRAY_SIZE(camera_isp),
		.component = camera_isp,
	},
	{
		.name = "CAM V4L2LOOPBACK driver",
		.num_component = ARRAY_SIZE(camera_v4l2loopback),
		.component = camera_v4l2loopback,
	}
};

#else
static const struct camera_submodule_component camera_base[] = {
#ifdef CONFIG_SPECTRA_CORE
	{&cam_req_mgr_init, &cam_req_mgr_exit},
	{&cam_rpmsg_init, &cam_rpmsg_exit},
	{&cam_sync_init, &cam_sync_exit},
	{&cam_smmu_init_module, &cam_smmu_exit_module},
	{&cam_cpas_dev_init_module, &cam_cpas_dev_exit_module},
	{&cam_cdm_intf_init_module, &cam_cdm_intf_exit_module},
	{&cam_hw_cdm_init_module, &cam_hw_cdm_exit_module},
#endif
};

static const struct camera_submodule_component camera_tfe[] = {
#ifdef CONFIG_SPECTRA_TFE
	{&cam_csid_ppi100_init_module, &cam_csid_ppi100_exit_module},
	{&cam_tfe_init_module, &cam_tfe_exit_module},
	{&cam_tfe_csid_init_module, &cam_tfe_csid_exit_module},
#endif
};

static const struct camera_submodule_component camera_isp[] = {
#ifdef CONFIG_SPECTRA_ISP
	{&cam_ife_csid_init_module, &cam_ife_csid_exit_module},
	{&cam_ife_csid_lite_init_module, &cam_ife_csid_lite_exit_module},
	{&cam_ife_virt_csid_init_module, &cam_ife_virt_csid_exit_module},
	{&cam_vfe_init_module, &cam_vfe_exit_module},
	{&cam_virt_vfe_init_module, &cam_virt_vfe_exit_module},
	{&cam_sfe_init_module, &cam_sfe_exit_module},
	{&cam_isp_dev_init_module, &cam_isp_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_sensor[] = {
#ifdef CONFIG_SPECTRA_SENSOR
	{&cam_res_mgr_init, &cam_res_mgr_exit},
	{&cam_cci_init_module, &cam_cci_exit_module},
	{&cam_csiphy_init_module, &cam_csiphy_exit_module},
	{&cam_tpg_init_module, &cam_tpg_exit_module},
	{&cam_actuator_driver_init, &cam_actuator_driver_exit},
	{&cam_sensor_driver_init, &cam_sensor_driver_exit},
	{&cam_eeprom_driver_init, &cam_eeprom_driver_exit},
	{&cam_ois_driver_init, &cam_ois_driver_exit},
	{&cam_flash_init_module, &cam_flash_exit_module},
	{&cam_sensor_lite_init_module, &cam_sensor_lite_exit_module},
	{&cam_csiphy_remote_init_module, &cam_csiphy_remote_exit_module},
#endif
};

static const struct camera_submodule_component camera_icp[] = {
#ifdef CONFIG_SPECTRA_ICP
	{&cam_a5_init_module, &cam_a5_exit_module},
	{&cam_lx7_init_module, &cam_lx7_exit_module},
	{&cam_ipe_init_module, &cam_ipe_exit_module},
	{&cam_bps_init_module, &cam_bps_exit_module},
	{&cam_icp_init_module, &cam_icp_exit_module},
#endif
};

static const struct camera_submodule_component camera_ope[] = {
#ifdef CONFIG_SPECTRA_OPE
	{&cam_ope_init_module, &cam_ope_exit_module},
	{&cam_ope_subdev_init_module, &cam_ope_subdev_exit_module},
#endif
};

static const struct camera_submodule_component camera_cre[] = {
#ifdef CONFIG_SPECTRA_CRE
	{&cam_cre_init_module, &cam_cre_exit_module},
	{&cam_cre_subdev_init_module, &cam_cre_subdev_exit_module},
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

static const struct camera_submodule_component camera_custom[] = {
#ifdef CONFIG_SPECTRA_CUSTOM
	{&cam_custom_hw_sub_module_init, &cam_custom_hw_sub_module_exit},
	{&cam_custom_csid_driver_init, &cam_custom_csid_driver_exit},
	{&cam_custom_dev_init_module, &cam_custom_dev_exit_module},
#endif
};

static const struct camera_submodule_component camera_presil[] = {
#ifdef CONFIG_CAM_PRESIL
	{&cam_presil_framework_dev_init_from_main, &cam_presil_framework_dev_exit_from_main},
#endif
};

static const struct camera_submodule_component camera_v4l2loopback[] = {
#ifdef CONFIG_V4L2_LOOPBACK
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
		.name = "Camera TFE",
		.num_component = ARRAY_SIZE(camera_tfe),
		.component = camera_tfe,
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
		.name = "Camera OPE",
		.num_component = ARRAY_SIZE(camera_ope),
		.component = camera_ope,
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
		.name = "Camera CRE",
		.num_component = ARRAY_SIZE(camera_cre),
		.component = camera_cre,
	},
	{
		.name = "Camera CUSTOM",
		.num_component = ARRAY_SIZE(camera_custom),
		.component = camera_custom,
	},
	{
		.name = "Camera Presil",
		.num_component = ARRAY_SIZE(camera_presil),
		.component = camera_presil,
	},
	{
		.name = "CAM V4L2LOOPBACK driver",
		.num_component = ARRAY_SIZE(camera_v4l2loopback),
		.component = camera_v4l2loopback,
	}
};
#endif

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

	CAM_INFO(CAM_UTIL, "%s", camera_banner);
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

	CAM_DBG(CAM_UTIL, "Camera initcalls done");

end_init:
	return rc;
}

static void camera_exit(void)
{
	__camera_exit(ARRAY_SIZE(submodule_table), 0);

	CAM_INFO(CAM_UTIL, "Spectra camera driver exited!");
}

module_init(camera_init);
module_exit(camera_exit);

MODULE_DESCRIPTION("Spectra camera driver");
MODULE_LICENSE("GPL v2");
