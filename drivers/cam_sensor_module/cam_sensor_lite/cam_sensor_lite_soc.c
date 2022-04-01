// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */


#include "cam_sensor_lite_dev.h"
#include "cam_sensor_lite_core.h"
#include "cam_sensor_lite_soc.h"

int32_t cam_sensor_lite_parse_dt_info(struct platform_device *pdev,
	struct sensor_lite_device *sensor_lite_dev)
{
	int32_t   rc = 0;
	struct cam_hw_soc_info   *soc_info;

	soc_info = &sensor_lite_dev->soc_info;

	rc = cam_soc_util_get_dt_properties(soc_info);
	if (rc < 0) {
		CAM_ERR(CAM_SENSOR_LITE, "parsing common soc dt(rc %d)", rc);
		return  rc;
	}

	return rc;
}
