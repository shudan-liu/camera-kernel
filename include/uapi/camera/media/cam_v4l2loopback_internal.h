/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef _UAPI_CAM_V4L2_INTERNAL_H
#define _UAPI_CAM_V4L2_INTERNAL_H


enum CAM_V4L2_OUTPUT_NOTIFY_CMD {
	CAM_V4L2_OUTPUT_OPEN_INPUT   = 0,  /* notify the output side qcarcam_open */
	CAM_V4L2_OUTPUT_CLOSE_INPUT,       /* notify the output side qcarcam_close */
	CAM_V4L2_OUTPUT_START_INPUT,       /* notify the output side qcarcam_start */
	CAM_V4L2_OUTPUT_STOP_INPUT,        /* notify the output side qcarcam_stop */
	CAM_V4L2_OUTPUT_GET_PARAM,         /* notify the output side get the param */
	CAM_V4L2_OUTPUT_SET_PARAM,         /* notify the output side set the param */
	CAM_V4L2_OUTPUT_ALLOC_BUFS,        /* notify the output side alloc the bufs */
	CAM_V4L2_OUTPUT_BUF_READY,         /* notify the output side buf ready */
};

enum CAM_V4L2_OUTPUT_PRIVATE_CMD {
	CAM_V4L2_OUTPUT_PRIV_SET_PARAM_RET,
	CAM_V4L2_OUTPUT_PRIV_SET_PARAM_EVENT,
	CAM_V4L2_OUTPUT_PRIV_SET_PARAM2,
	CAM_V4L2_OUTPUT_PRIV_GET_PARAM,
	CAM_V4L2_OUTPUT_PRIV_OPEN_RET,
	CAM_V4L2_OUTPUT_PRIV_CLOSE_RET,
	CAM_V4L2_OUTPUT_PRIV_START_RET,
	CAM_V4L2_OUTPUT_PRIV_STOP_RET,
	CAM_V4L2_OUTPUT_PRIV_SET_BUFS,
	CAM_V4L2_OUTPUT_PRIV_SET_INPUT_SIGNAL_EVENT,
	CAM_V4L2_OUTPUT_PRIV_SET_ERROR_EVENT,
	CAM_V4L2_OUTPUT_PRIV_SET_FRAME_DROP_EVENT,
};

struct cam_v4l2_buffers_t {
	__u32 nbufs;
	__s32 fds[MAX_CAM_BUFFERS_NUM];
};

#endif /* _UAPI_CAM_V4L2_INTERNAL_H */
