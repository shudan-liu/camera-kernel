/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include "cam_sensor_dev.h"

#define DP_BDG_SENSOR_ID 0x1901
#define DP_BDG_FW_VERSION 0x000001
#define DP_BDG_DP_CONNECTED    0x01
#define DP_BDG_DP_DISCONNECTED 0x00
#define DP_SENSOR_NAME "lt7911uxc"

/**
 * @s_ctrl: Sensor ctrl structure
 *
 * This API set lt7911 camera struct for irq handle.
 */
int cam_dp_bdg_set_cam_ctrl(struct cam_sensor_ctrl_t *s_ctrl);

/**
 * This API unset lt7911 camera struct for irq handle.
 */
void cam_dp_bdg_unset_cam_ctrl(void);

int cam_dp_bdg_get_src_resolution(bool *signal_stable,
		int *width, int *height, int *id);

/**
 * @brief : API to register DP dev to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int dp_bdg_irq_handler_init(void);

/**
 * @brief : API to remove DP dev from platform framework.
 */
void dp_bdg_irq_handler_exit(void);
