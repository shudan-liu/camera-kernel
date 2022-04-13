/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_LRME_DEV_H_
#define _CAM_LRME_DEV_H_

/**
 * @brief : API to register LRME dev to platform framework.
 * @return struct platform_device pointer on success, or ERR_PTR() on error.
 */
int cam_lrme_dev_init_module(void);

/**
 * @brief : API to remove LRME dev from platform framework.
 */
void cam_lrme_dev_exit_module(void);

#endif /* _CAM_LRME_DEV_H_ */
