/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _V4L2LOOPBACK_V2_H_
#define _V4L2LOOPBACK_V2_H_

/**
 * @brief : API to register v4l2loopback driver to platform framework.
 * @return struct platform_device pointer on success, or ERR_PTR() on error.
 */
int v4l2loopback_init_module(void);

/**
 * @brief : API to remove v4l2loopback from platform framework.
 */
void v4l2loopback_cleanup_module(void);

#endif /* _V4L2LOOPBACK_V2_H_ */
