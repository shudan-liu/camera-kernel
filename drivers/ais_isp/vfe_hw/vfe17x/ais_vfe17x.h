/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _AIS_VFE17X_H_
#define _AIS_VFE17X_H_

/**
 * @brief : API to register VFE HW to platform framework.
 * @return struct platform_device pointer on success, or ERR_PTR() on error.
 */
int ais_vfe_init_module(void);

/**
 * @brief : API to remove VFE HW from platform framework.
 */
void ais_vfe_exit_module(void);

#endif /* _AIS_VFE17X_H_ */
