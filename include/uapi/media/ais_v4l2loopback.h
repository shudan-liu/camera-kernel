#ifndef _UAPI_AIS_V4L2_H
#define _UAPI_AIS_V4L2_H

#include <linux/types.h>

#define AIS_V4L2_DRV_MAX_VERSION    2
#define AIS_V4L2_DRV_MIN_VERSION    1
#define AIS_V4L2_DRV_BUGFIX_VERSION 0

#define MAX_AIS_V4L2_PAYLOAD_SIZE     256
#define MAX_AIS_V4L2_PARAM_EVNET_SIZE 62

#define MAX_AIS_BUFFERS_NUM  20

enum AIS_V4L2_CLIENT_ID {
	AIS_V4L2_CLIENT_OUTPUT = 1,
	AIS_V4L2_CLIENT_CAPTURE = 2,
};

enum AIS_V4L2_NOTIFY_CMD {
	AIS_V4L2_OPEN_INPUT   = 0,  /* notify the output side qcarcam_open */
	AIS_V4L2_CLOSE_INPUT,       /* notify the output side qcarcam_close */
	AIS_V4L2_START_INPUT,       /* notify the output side qcarcam_start */
	AIS_V4L2_STOP_INPUT,        /* notify the output side qcarcam_stop */
	AIS_V4L2_GET_PARAM,         /* notify the output side get the param */
	AIS_V4L2_SET_PARAM,         /* notify the output side set the param */
	AIS_V4L2_PARAM_EVENT,       /* notify the capture side the param event */
	AIS_V4L2_ALLOC_BUFS,        /* notify the output side alloc the bufs */
	AIS_V4L2_OUTPUT_BUF_READY,  /* notify the output side buf ready */
};

enum AIS_V4L2_PARAM_CODE {
	AIS_V4L2_PARAM_EVENT_CB = 0x1,   /* Event callback function. */
	AIS_V4L2_PARAM_EVENT_MASK,       /* Mask of events */
	AIS_V4L2_PARAM_COLOR_FMT,        /* Output color format. */
	AIS_V4L2_PARAM_RESOLUTION,       /* Input dev resolution. */
	AIS_V4L2_PARAM_BRIGHTNESS,
	AIS_V4L2_PARAM_CONTRAST,
	AIS_V4L2_PARAM_MIRROR_H,         /* Horizontal mirror. */
	AIS_V4L2_PARAM_MIRROR_V,         /* Vertical mirror. */
	AIS_V4L2_PARAM_FRAME_RATE,
	AIS_V4L2_PARAM_VID_STD,          /* Video standard */
	AIS_V4L2_PARAM_CURRENT_VID_STD,  /* Video standard */
	AIS_V4L2_PARAM_STATUS,           /* Video lock status */
	AIS_V4L2_PARAM_LATENCY_MAX,      /* Max buffer latency in frame
					  * done Q
					  */
	AIS_V4L2_PARAM_LATENCY_REDUCE_RATE,  /* Number of buffers to drop
					      * when max latency reached
					      */
	AIS_V4L2_PARAM_PRIVATE_DATA,
	AIS_V4L2_PARAM_INJECTION_START,
	AIS_V4L2_PARAM_EXPOSURE,         /* exposure setting */
	AIS_V4L2_PARAM_HUE,              /* hue setting */
	AIS_V4L2_PARAM_SATURATION,       /* saturation setting */
	AIS_V4L2_PARAM_HDR_EXPOSURE,
	AIS_V4L2_PARAM_GAMMA,            /* gamma setting */
	AIS_V4L2_PARAM_OPMODE,           /* operation mode */
	AIS_V4L2_PARAM_ISP_CTRLS,        /* ISP controls */
	AIS_V4L2_PARAM_VENDOR,           /* vendor param */
	AIS_V4L2_PARAM_INPUT_MODE,       /* Input device mode. */
	AIS_V4L2_PARAM_MASTER,           /* Set the client as master */
	AIS_V4L2_PARAM_EVENT_CHANGE_SUBSCRIBE,   /* Event subscription */
	AIS_V4L2_PARAM_EVENT_CHANGE_UNSUBSCRIBE, /* Event unsubscribe */
	AIS_V4L2_PARAM_NUM,              /* total number of valid parameters. */

	AIS_V4L2_PARAM_MAX = 0x7FFFFFFF
};

enum AIS_V4L2_OUTPUT_PRIVATE_CMD {
	AIS_V4L2_OUTPUT_PRIV_SET_PARAM_RET,
	AIS_V4L2_OUTPUT_PRIV_SET_PARAM_EVENT,
	AIS_V4L2_OUTPUT_PRIV_SET_PARAM2,
	AIS_V4L2_OUTPUT_PRIV_GET_PARAM,
	AIS_V4L2_OUTPUT_PRIV_OPEN_RET,
	AIS_V4L2_OUTPUT_PRIV_CLOSE_RET,
	AIS_V4L2_OUTPUT_PRIV_START_RET,
	AIS_V4L2_OUTPUT_PRIV_STOP_RET,
	AIS_V4L2_OUTPUT_PRIV_SET_BUFS,
};

enum AIS_V4L2_CAPTURE_PRIVATE_CMD {
	AIS_V4L2_CAPTURE_PRIV_SET_PARAM,
	AIS_V4L2_CAPTURE_PRIV_GET_PARAM,
};

struct ais_v4l2_control_t {
	__u32 type;
	__u32 size;
	__u64 payload;
	__u8  cmd;
	union {
		__u8  param_type;
		__u8  ctrl_ret;  /* 0: succeed; 1: fail */
	};
};

struct ais_v4l2_buffers_t {
	__u32 nbufs;
	__s32 fds[MAX_AIS_BUFFERS_NUM];
};

#endif /* _UAPI_AIS_V4L2_H */
