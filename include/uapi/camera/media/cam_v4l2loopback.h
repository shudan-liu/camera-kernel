/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef _UAPI_CAM_V4L2_H
#define _UAPI_CAM_V4L2_H

#include <linux/types.h>

#define CAM_V4L2_DRV_MAX_VERSION    6
#define CAM_V4L2_DRV_MIN_VERSION    0
#define CAM_V4L2_DRV_BUGFIX_VERSION 0

#define MAX_CAM_V4L2_PAYLOAD_SIZE     256
#define MAX_CAM_V4L2_PARAM_EVNET_SIZE 62

#define MAX_CAM_BUFFERS_NUM  20
#define MAX_BATCH_NUM 4

enum CAM_V4L2_CLIENT_ID {
	CAM_V4L2_CLIENT_OUTPUT = 1,
	CAM_V4L2_CLIENT_CAPTURE = 2,
};

enum CAM_V4L2_CAPTURE_NOTIFY_CMD {
	CAM_V4L2_CAPTURE_PARAM_EVENT   = 0, /* notify the capture side the param event */
	CAM_V4L2_CAPTURE_EVENT_INPUT_SIGNAL,/* nofity the capture side the input */
	CAM_V4L2_CAPTURE_EVENT_ERROR,       /* nofity the capture side the error */
	CAM_V4L2_CAPTURE_EVENT_FRAME_DROP,  /* nofity the capture side frame drop*/
};

enum CAM_V4L2_PARAM_CODE {
	CAM_V4L2_PARAM_EVENT_CB = 0x1,   /* Event callback function. */
	CAM_V4L2_PARAM_EVENT_MASK,       /* Mask of events */
	CAM_V4L2_PARAM_COLOR_FMT,        /* Output color format. */
	CAM_V4L2_PARAM_RESOLUTION,       /* Input dev resolution. */
	CAM_V4L2_PARAM_BRIGHTNESS,
	CAM_V4L2_PARAM_CONTRAST,
	CAM_V4L2_PARAM_MIRROR_H,         /* Horizontal mirror. */
	CAM_V4L2_PARAM_MIRROR_V,         /* Vertical mirror. */
	CAM_V4L2_PARAM_FRAME_RATE,
	CAM_V4L2_PARAM_VID_STD,          /* Video standard */
	CAM_V4L2_PARAM_CURRENT_VID_STD,  /* Video standard */
	CAM_V4L2_PARAM_STATUS,           /* Video lock status */
	CAM_V4L2_PARAM_LATENCY_MAX,      /* Max buffer latency in frame
					                  * done Q
					                  */
	CAM_V4L2_PARAM_LATENCY_REDUCE_RATE,  /* Number of buffers to drop
					                     * when max latency reached
					                     */
	CAM_V4L2_PARAM_PRIVATE_DATA,
	CAM_V4L2_PARAM_INJECTION_START,
	CAM_V4L2_PARAM_EXPOSURE,         /* exposure setting */
	CAM_V4L2_PARAM_HUE,              /* hue setting */
	CAM_V4L2_PARAM_SATURATION,       /* saturation setting */
	CAM_V4L2_PARAM_HDR_EXPOSURE,
	CAM_V4L2_PARAM_GAMMA,            /* gamma setting */
	CAM_V4L2_PARAM_OPMODE,           /* operation mode */
	CAM_V4L2_PARAM_ISP_CTRLS,        /* ISP controls */
	CAM_V4L2_PARAM_VENDOR,           /* vendor param */
	CAM_V4L2_PARAM_INPUT_MODE,       /* Input device mode. */
	CAM_V4L2_PARAM_MASTER,           /* Set the client as master */
	CAM_V4L2_PARAM_EVENT_CHANGE_SUBSCRIBE,   /* Event subscription */
	CAM_V4L2_PARAM_EVENT_CHANGE_UNSUBSCRIBE, /* Event unsubscribe */
	CAM_V4L2_PARAM_RECOVERY,             /* Should recovery mechanism be active or not. */
	CAM_V4L2_PARAM_BATCH_MODE,
	CAM_V4L2_PARAM_ISP_USECASE,          /* ISP usecase */
	CAM_V4L2_PARAM_NUM,                  /* total number of valid parameters. */

	CAM_V4L2_PARAM_MAX = 0x7FFFFFFF
};

enum CAM_V4L2_CAPTURE_PRIVATE_CMD {
	CAM_V4L2_CAPTURE_PRIV_SET_PARAM,
	CAM_V4L2_CAPTURE_PRIV_GET_PARAM,
};

struct cam_v4l2_control_t {
	__u32 type;
	__u32 size;
	__u64 payload;
	__u8  cmd;
	union {
		__u8  param_type;
		__u8  ctrl_ret;  /* 0: succeed; 1: fail */
	};
};

struct cam_v4l2_buffer_ext_t {
	__u32 batch_num;
	__u32 reversed;
	__u64 timestamp[MAX_BATCH_NUM];

};

#endif /* _UAPI_CAM_V4L2_H */

