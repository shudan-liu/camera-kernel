// SPDX-License-Identifier: GPL-2.0-only
/* -*- c-file-style: "linux" -*-*/
/*
 * v4l2loopback.c  --  video4linux2 loopback driver
 *
 * Copyright (C) 2005-2009 Vasily Levin (vasaka@gmail.com)
 * Copyright (C) 2010-2019 IOhannes m zmoelnig (zmoelnig@iem.at)
 * Copyright (C) 2011 Stefan Diewald (stefan.diewald@mytum.de)
 * Copyright (C) 2012 Anton Novikov (random.plant@gmail.com)
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved
 * Copyright (c) 2021-2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/time64.h>
#include <linux/module.h>
#include <linux/videodev2.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/eventpoll.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-common.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <linux/dma-buf.h>
#include <linux/ion.h>
#include <linux/msm_ion.h>

#include <media/ais_v4l2loopback.h>
#include <media/cam_defs.h>


#include <linux/init.h>
#include <linux/kprobes.h>
#include <asm/traps.h>

#include "cam_common_util.h"
#include "cam_debug_util.h"

# define HAVE__V4L2_DEVICE
# include <media/v4l2-device.h>
# define HAVE__V4L2_CTRLS
# include <media/v4l2-ctrls.h>


#if defined(timer_setup) && defined(from_timer)
#define HAVE_TIMER_SETUP
#endif

#define V4L2LOOPBACK_VERSION_CODE KERNEL_VERSION(0, 12, 0)

MODULE_DESCRIPTION("V4L2 loopback video device");
MODULE_LICENSE("GPL v2");

/*
 * helpers
 */
#define STRINGIFY(s) #s
#define STRINGIFY2(s) STRINGIFY(s)

#define MARK()                                                          \
	do { if (debug > 1) {                                                  \
		pr_err("%s:%d[%s]\n", __FILE__, __LINE__, __func__);       \
	} } while (0)

#define AIS_V4L2LOOPBACK_EVENT_MAX  30

/*
 * compatibility hacks
 */

#ifndef HAVE__V4L2_CTRLS
struct v4l2_ctrl_handler {
	int error;
};
struct v4l2_ctrl_config {
	void *ops;
	u32 id;
	const char *name;
	int type;
	s32 min;
	s32 max;
	u32 step;
	s32 def;
};
int v4l2_ctrl_handler_init(struct v4l2_ctrl_handler *hdl,
			   unsigned int nr_of_controls_hint)
{
	hdl->error = 0;
	return 0;
}
void v4l2_ctrl_handler_free(struct v4l2_ctrl_handler *hdl)
{
}
void *v4l2_ctrl_new_custom(struct v4l2_ctrl_handler *hdl,
			  const struct v4l2_ctrl_config *conf,
			  void *priv)
{
	return NULL;
}
#endif /* HAVE__V4L2_CTRLS */


#ifndef HAVE__V4L2_DEVICE
/* dummy v4l2_device struct/functions */
#define V4L2_DEVICE_NAME_SIZE (20 + 16)
struct v4l2_device {
	char name[V4L2_DEVICE_NAME_SIZE];
	struct v4l2_ctrl_handler *ctrl_handler;
};
static inline int v4l2_device_register(void *dev, void *v4l2_dev)
{
	return 0;
}
static inline void v4l2_device_unregister(struct v4l2_device *v4l2_dev)
{
}
#endif /*  HAVE__V4L2_DEVICE */

# define v4l2l_vzalloc vzalloc


/* module constants
 *  can be overridden during he build process using something like
 *      make KCPPFLAGS="-DMAX_DEVICES=100"
 */


/* maximum number of v4l2loopback devices that can be created */
#ifndef MAX_DEVICES
# define MAX_DEVICES 16
#endif

/* when a producer is considered to have gone stale */
#ifndef MAX_TIMEOUT
# define MAX_TIMEOUT (100 * 1000) /* in msecs */
#endif

/* Timeout value in msec */
#define GPARAM_TIMEOUT 2000
#define SPARAM_TIMEOUT 2000
#define OPEN_TIMEOUT 4000
#define CLOSE_TIMEOUT 4000
#define START_TIMEOUT 4000
#define STOP_TIMEOUT 4000
#define ALLOCBUFS_TIMEOUT 6000

/* max buffers that can be mapped, actually they
 * are all mapped to max_buffers buffers
 */
#ifndef MAX_BUFFERS
# define MAX_BUFFERS MAX_AIS_BUFFERS_NUM
#endif

/* module parameters */
static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "debugging level (higher values == more verbose)");

static int max_buffers = MAX_AIS_BUFFERS_NUM;
module_param(max_buffers, int, 0444);
MODULE_PARM_DESC(max_buffers, "how many buffers should be allocated");

/* how many times a device can be opened
 * the per-module default value can be overridden on a per-device basis using
 * the /sys/devices interface
 *
 * note that max_openers should be at least 2 in order to get a working system:
 *   one opener for the producer and one opener for the consumer
 *   however, we leave that to the user
 */
static int max_openers = 10;
module_param(max_openers, int, 0644);
MODULE_PARM_DESC(max_openers, "how many users can open loopback device");


static int devices = -1;
module_param(devices, int, 0644);
MODULE_PARM_DESC(devices, "how many devices should be created");


static int video_nr[MAX_DEVICES] = { [0 ... (MAX_DEVICES - 1)] = -1 };
module_param_array(video_nr, int, NULL, 0444);
MODULE_PARM_DESC(video_nr,
		"video device numbers (-1=auto, 0=/dev/video0, etc.)");

static char *card_label[MAX_DEVICES];
module_param_array(card_label, charp, NULL, 0000);
MODULE_PARM_DESC(card_label, "card labels for every device");

static bool exclusive_caps[MAX_DEVICES] = { [0 ... (MAX_DEVICES - 1)] = 0 };
module_param_array(exclusive_caps, bool, NULL, 0444);
/* FIXXME: wording */
MODULE_PARM_DESC(exclusive_caps,
		"whether to announce OUTPUT/CAPTURE capabilities exclusively or not");


/* format specifications */
#define V4L2LOOPBACK_SIZE_MIN_WIDTH   48
#define V4L2LOOPBACK_SIZE_MIN_HEIGHT  32
#define V4L2LOOPBACK_SIZE_MAX_WIDTH   8192
#define V4L2LOOPBACK_SIZE_MAX_HEIGHT  8192

#define V4L2LOOPBACK_SIZE_DEFAULT_WIDTH   640
#define V4L2LOOPBACK_SIZE_DEFAULT_HEIGHT  480

static int max_width = V4L2LOOPBACK_SIZE_MAX_WIDTH;
module_param(max_width, int, 0444);
MODULE_PARM_DESC(max_width, "maximum frame width");
static int max_height = V4L2LOOPBACK_SIZE_MAX_HEIGHT;
module_param(max_height, int, 0444);
MODULE_PARM_DESC(max_height, "maximum frame height");


/* control IDs */
#ifndef HAVE__V4L2_CTRLS
#define V4L2LOOPBACK_CID_BASE (V4L2_CID_PRIVATE_BASE)
#else
#define V4L2LOOPBACK_CID_BASE (V4L2_CID_USER_BASE | 0xf000)
#endif

static int v4l2loopback_s_ctrl(struct v4l2_ctrl *ctrl);
static int v4l2loopback_g_ctrl(struct v4l2_ctrl *ctrl);
static int v4l2loopback_datasize_try_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops v4l2loopback_ctrl_ops = {
	.s_ctrl = v4l2loopback_s_ctrl,
	.try_ctrl = v4l2loopback_datasize_try_ctrl,
	.g_volatile_ctrl = v4l2loopback_g_ctrl,
};

static const struct v4l2_ctrl_config v4l2loopback_ctrl_buffernum = {
	.ops = &v4l2loopback_ctrl_ops,
	.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE,
	.name = "buffer_num",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.min = 4,
	.max = 20,
	.step = 1,
	.def = 4,   // def >= min
};

enum V4L2_LOOPBACK_IO_MODE {
	V4L2L_IO_MODE_MMAP   = 0,
	V4L2L_IO_MODE_USERPTR = 1,
	V4L2L_IO_MODE_DMABUF = 2,
};

/* The initial state is V4L2L_READY_FOR_OUTPUT, when ais_v4l2_proxy opens
 * device, the state becomes V4L2L_READY_FOR_CAPTURE; when v4l2 app opens
 * the device, the state becomes V4L2L_OPENED, can't be opened by app again.
 * When v4l2 app close the device, the state becomes V4L2L_READY_FOR_CAPTURE
 * again. when proxy close the device, the state becomes V4L2L_READY_FOR_OUTPUT
 * again.
 * Todo: when the app/pxoy is closed by kill -9, the device is not closed
 * correctly. The state will be corrupted. For this situation, need reset
 * the state. Need find a way to reset the state to initial state. eg. when
 * a new proxy opens the device, kick off the other proxy/app and reset the
 * state.
 */
enum V4L2_LOOPBACK_STATE {
	V4L2L_READY_FOR_OUTPUT = 0x1,
	V4L2L_READY_FOR_CAPTURE = 0x2,
	V4L2L_OPENED = 0x22,
};

enum V4L2_LOOPBACK_BUF_STATE {
	V4L2L_BUF_READY_Q   = 0,
	V4L2L_BUF_DONE_Q    = 1,
	V4L2L_BUF_USER_ACQUIRED  = 2,
	V4L2L_BUF_PROXY_ACQUIRED = 3,
	V4L2L_BUF_PENDING = 4,
};


/* module structures */
struct v4l2loopback_private {
	int devicenr;
};

/* TODO(vasaka) use typenames which are common to kernel, but first find out if
 * it is needed
 */
/* struct keeping state and settings of loopback device */

struct v4l2l_buffer {
	struct v4l2_buffer buffer;
	struct list_head list_head;

	int use_count;
	uintptr_t kvaddr;
	enum V4L2_LOOPBACK_BUF_STATE state;
	struct ais_v4l2_buffer_ext_t ext;
};

struct v4l2_loopback_device {
	struct v4l2_device v4l2_dev;
	struct v4l2_ctrl_handler ctrl_handler;
	struct video_device *vdev;
	struct v4l2_loopback_opener *main_opener; /*proxy first opener*/

	/* qcarcam open ret*/
	int qcarcam_ctrl_ret;
	struct completion open_complete;
	struct completion close_complete;
	struct v4l2_fh *open_ret_fh;

	enum V4L2_LOOPBACK_STATE state;
	atomic_t open_count;

	int max_openers;
	int stream_status;

	struct mutex dev_mutex;

	int announce_all_caps;/* set to false, if device caps (OUTPUT/CAPTURE)
			       * should only be announced if the resp. "ready"
			       * flag is set; default=TRUE
			       */
};

/* types of opener shows what opener wants to do with loopback */
enum v4l2_loopback_opener_type {
	V4L2L_UNNEGOTIATED = 0,
	V4L2L_READER = 1,
	V4L2L_WRITER = 2,
};

struct v4l2_streamdata {
	/* pixel and stream format */
	struct v4l2_pix_format pix_format;
	struct v4l2_captureparm capture_param;

	/* streamon/streamoff ctrl*/
	struct completion ctrl_complete;
	int qcarcam_ctrl_ret;

	/* param for vendor extension&other qcarcam API*/
	u8 qcarcam_code;
	u8 qcarcam_param[MAX_AIS_V4L2_PAYLOAD_SIZE];
	long qcarcam_param_size;
	struct completion sparam_complete;
	struct completion gparam_complete;
	int qcarcam_sparam_ret;

	struct v4l2_crop frame_crop;

	enum V4L2_LOOPBACK_IO_MODE io_mode;

	struct dma_buf *dmabufs[MAX_BUFFERS];
	struct completion allocbufs_complete;
	int allocbufs_ret;
	int buffers_number;  /* should not be big, 4 is a good choice */
	struct v4l2l_buffer buffers[MAX_BUFFERS]; /* inner driver buffers */
	int used_buffers; /* number of the actually used buffers */
	unsigned int use_buf_width;
	u8 is_streaming;

	struct list_head outbufs_list; /* buffers in output DQBUF order */
	struct list_head capbufs_list; /* buffers in capture DQBUF order */
	struct mutex outbufs_mutex;
	struct mutex capbufs_mutex;

	long buffer_size;

	wait_queue_head_t read_event;
	spinlock_t lock;
	struct mutex buf_mutex;
};

/* struct keeping state and type of opener */
struct v4l2_loopback_opener {
	enum v4l2_loopback_opener_type type;
	struct v4l2_fh fh;
	struct v4l2_loopback_opener *connected_opener;

	struct v4l2_streamdata *data;
};

#define fh_to_opener(ptr) container_of((ptr), struct v4l2_loopback_opener, fh)
/* this is heavily inspired by the bttv driver found in the linux kernel */
struct v4l2l_format {
	char *name;
	int  fourcc;          /* video4linux 2      */
	int  depth;           /* bit/pixel          */
	int  flags;
};
/* set the v4l2l_format.flags to PLANAR for non-packed formats */
#define FORMAT_FLAGS_PLANAR       0x01
#define FORMAT_FLAGS_COMPRESSED   0x02

#ifndef V4L2_PIX_FMT_VP9
#define V4L2_PIX_FMT_VP9  v4l2_fourcc('V', 'P', '9', '0')
#endif
#ifndef V4L2_PIX_FMT_HEVC
#define V4L2_PIX_FMT_HEVC  v4l2_fourcc('H', 'E', 'V', 'C')
#endif

static const struct v4l2l_format formats[] = {
#include "v4l2loopback_formats.h"
};

static const unsigned int FORMATS = ARRAY_SIZE(formats);


static char *fourcc2str(unsigned int fourcc, char buf[4])
{
	buf[0] = (fourcc >>  0) & 0xFF;
	buf[1] = (fourcc >>  8) & 0xFF;
	buf[2] = (fourcc >> 16) & 0xFF;
	buf[3] = (fourcc >> 24) & 0xFF;

	return buf;
}

static void send_v4l2_event(struct v4l2_loopback_opener *opener, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;

	if (opener)
		v4l2_event_queue_fh(&opener->fh, &event);
	CAM_DBG(CAM_V4L2, "send v4l2 event for ais_v4l2loopback :%d",
		cmd);
}

static void send_v4l2_event_ex(struct v4l2_loopback_opener *opener, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd, u8 data0)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;
	event.u.data[0] = data0;

	if (opener)
		v4l2_event_queue_fh(&opener->fh, &event);
	CAM_DBG(CAM_V4L2, "send v4l2 event for ais_v4l2loopback :%d",
		cmd);
}

static void send_v4l2_event_ex1(struct v4l2_loopback_opener *opener, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd, u8 data0, u8 data1)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;
	event.u.data[0] = data0;
	event.u.data[1] = data1;

	if (opener)
		v4l2_event_queue_fh(&opener->fh, &event);
	CAM_DBG(CAM_V4L2, "send v4l2 event for ais_v4l2loopback :%d",
		cmd);
}


static void send_v4l2_event_ex2(struct v4l2_loopback_opener *opener, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd, u8 data0, u8 size, __u64 payload)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;
	event.u.data[0] = data0;
	event.u.data[1] = size;

	if (size == 0) {
		if (opener) {
			v4l2_event_queue_fh(&opener->fh, &event);
			CAM_DBG(CAM_V4L2, "send v4l2 event for ais_v4l2loopback :%d",
				cmd);
		}
	} else if (size <= MAX_AIS_V4L2_PARAM_EVNET_SIZE) {
		if (copy_from_user(&event.u.data[2],
					u64_to_user_ptr(payload),
					size)) {
			CAM_ERR(CAM_V4L2, "fail to copy from user when send v4l2");
		} else {
			if (opener) {
				v4l2_event_queue_fh(&opener->fh, &event);
				CAM_DBG(CAM_V4L2, "send v4l2 event for ais_v4l2loopback :%d",
					cmd);
			}
		}
	} else {
		if (opener)
			v4l2_event_queue_fh(&opener->fh, &event);
	}
}

static const struct v4l2l_format *format_by_fourcc(int fourcc)
{
	unsigned int i;

	for (i = 0; i < FORMATS; i++) {
		if (formats[i].fourcc == fourcc)
			return formats + i;
	}

	CAM_ERR(CAM_V4L2, "unsupported format '%c%c%c%c'",
			(fourcc >>  0) & 0xFF,
			(fourcc >>  8) & 0xFF,
			(fourcc >> 16) & 0xFF,
			(fourcc >> 24) & 0xFF);
	return NULL;
}

static void pix_format_set_size(struct v4l2_pix_format *f,
		const struct v4l2l_format *fmt,
		unsigned int width, unsigned int height)
{
	f->width = width;
	f->height = height;

	if (fmt->flags & FORMAT_FLAGS_PLANAR) {
		if (f->bytesperline == 0)
			f->bytesperline = width; /* Y plane */
		if (f->sizeimage == 0)
			f->sizeimage = (width * height * fmt->depth) >> 3;
	} else if (fmt->flags & FORMAT_FLAGS_COMPRESSED) {
		/* doesn't make sense for compressed formats */
		f->bytesperline = 0;
		if (f->sizeimage == 0)
			f->sizeimage = (width * height * fmt->depth) >> 3;
	} else {
		if (f->bytesperline == 0)
			f->bytesperline = (width * fmt->depth) >> 3;
		if (f->sizeimage == 0)
			f->sizeimage = height * f->bytesperline;
	}
}

static struct v4l2_loopback_device *v4l2loopback_cd2dev(struct device *cd);

/* device attributes */
/* available via sysfs: /sys/devices/virtual/video4linux/video* */

static ssize_t attr_show_stream_status(struct device *cd,
		struct device_attribute *attr, char *buf)
{
	struct v4l2_loopback_device *dev = v4l2loopback_cd2dev(cd);

	if (dev == NULL) {
		CAM_ERR(CAM_V4L2, "\ndev value is null");
		return -EINVAL;
	}

	return scnprintf(buf, sizeof(dev->stream_status), "%d",
			dev->stream_status);
}

static ssize_t attr_store_stream_status(struct device *cd,
		struct device_attribute *attr, const char *buf, size_t len)
{
	struct v4l2_loopback_device *dev = NULL;
	unsigned long curr = 0;

	if (kstrtoul(buf, 0, &curr))
		return -EINVAL;

	dev = v4l2loopback_cd2dev(cd);

	if (dev == NULL) {
		CAM_ERR(CAM_V4L2, "\ndev value is null");
		return -EINVAL;
	}

	if (dev->stream_status == curr)
		return len;

	dev->stream_status = (int)curr;

	return len;
}


static DEVICE_ATTR(stream_status, 0644, attr_show_stream_status,
		attr_store_stream_status);

static void v4l2loopback_remove_sysfs(struct video_device *vdev)
{
#define V4L2_SYSFS_DESTROY(x) device_remove_file(&vdev->dev, &dev_attr_##x)

	if (vdev) {
		do {

			V4L2_SYSFS_DESTROY(stream_status);
			/* ... */
		} while (0);
	}
}

static void v4l2loopback_create_sysfs(struct video_device *vdev)
{
	int res = 0;

#define V4L2_SYSFS_CREATE(x) \
	do { \
		res = device_create_file(&vdev->dev, &dev_attr_##x); \
		if (res < 0) \
			break; \
	} while (0) \


	if (!vdev)
		return;
	do {

		V4L2_SYSFS_CREATE(stream_status);
		/* ... */
	} while (0);

	if (res >= 0)
		return;
	dev_err(&vdev->dev, "error: %d\n", res);
}

/* global module data */
static struct v4l2_loopback_device *devs[MAX_DEVICES];

static struct v4l2_loopback_device *v4l2loopback_cd2dev(struct device *cd)
{
	struct video_device *loopdev = to_video_device(cd);
	struct v4l2loopback_private *ptr =
		(struct v4l2loopback_private *)video_get_drvdata(loopdev);
	int nr = ptr->devicenr;

	if (nr < 0 || nr >= devices) {
		CAM_ERR(CAM_V4L2, "v4l2-loopback: illegal device %d", nr);
		return NULL;
	}
	return devs[nr];
}

static struct v4l2_loopback_device *v4l2loopback_getdevice(struct file *f)
{
	struct video_device *loopdev = video_devdata(f);
	struct v4l2loopback_private *ptr =
		(struct v4l2loopback_private *)video_get_drvdata(loopdev);
	int nr = ptr->devicenr;

	if (nr < 0 || nr >= devices) {
		CAM_ERR(CAM_V4L2, "v4l2-loopback: illegal device %d", nr);
		return NULL;
	}
	return devs[nr];
}

/* forward declarations */
static void init_buffers(struct v4l2_streamdata *data);
static int allocate_dma_buffers(struct v4l2_loopback_opener *opener, struct v4l2_streamdata *data);

static int free_buffers(struct v4l2_streamdata *data);
static int init_stream_data(struct v4l2_streamdata *data);
static void free_stream_data(struct v4l2_streamdata *data);

static const struct v4l2_file_operations v4l2_loopback_fops;
static const struct v4l2_ioctl_ops v4l2_loopback_ioctl_ops;

/* Queue helpers */
/* next functions sets buffer flags and adjusts counters accordingly */
static inline void set_done(struct v4l2l_buffer *buffer)
{
	buffer->buffer.flags &= ~V4L2_BUF_FLAG_QUEUED;
	buffer->buffer.flags |= V4L2_BUF_FLAG_DONE;
}

static inline void set_queued(struct v4l2l_buffer *buffer)
{
	buffer->buffer.flags &= ~V4L2_BUF_FLAG_DONE;
	buffer->buffer.flags |= V4L2_BUF_FLAG_QUEUED;
}

static inline void unset_flags(struct v4l2l_buffer *buffer)
{
	buffer->buffer.flags &= ~V4L2_BUF_FLAG_QUEUED;
	buffer->buffer.flags &= ~V4L2_BUF_FLAG_DONE;
}

static void vidioc_fill_name(char *buf, int len, int nr)
{
	if (card_label[nr] != NULL)
		snprintf(buf, len, "%s", card_label[nr]);
	else
		snprintf(buf, len, "Dummy video device (0x%04X)", nr);

	CAM_DBG(CAM_V4L2, "name is %s", buf);
}

/* V4L2 ioctl caps and params calls
 * returns device capabilities
 * called on VIDIOC_QUERYCAP
 */
static int vidioc_querycap(struct file *file, void *priv,
		struct v4l2_capability *cap)
{
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);
	int devnr;
	__u32 capabilities;

	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}
	devnr = ((struct v4l2loopback_private *)
			video_get_drvdata(dev->vdev))->devicenr;

	CAM_DBG(CAM_V4L2, "device_caps 0x%x, capabilities 0x%x",
		cap->device_caps, cap->capabilities);

	capabilities = V4L2_CAP_STREAMING | V4L2_CAP_READWRITE;

	strlcpy(cap->driver, "v4l2 loopback", sizeof(cap->driver));
	vidioc_fill_name(cap->card, sizeof(cap->card), devnr);
	snprintf(cap->bus_info, sizeof(cap->bus_info),
			"platform:v4l2loopback-%03d", devnr);

	/* since 3.1.0, the v4l2-core system is supposed to set the version */
	cap->version = V4L2LOOPBACK_VERSION_CODE;

#ifdef V4L2_CAP_VIDEO_M2M
	capabilities |= V4L2_CAP_VIDEO_M2M;
#endif /* V4L2_CAP_VIDEO_M2M */

	if (dev->announce_all_caps) {
		capabilities |= V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_OUTPUT;
	} else {
		if (dev->state & V4L2L_READY_FOR_CAPTURE)
			capabilities |= V4L2_CAP_VIDEO_CAPTURE;

		if (dev->state & V4L2L_READY_FOR_OUTPUT)
			capabilities |= V4L2_CAP_VIDEO_OUTPUT;
	}

	cap->device_caps = cap->capabilities = capabilities;
	/* >=linux-4.7.0 */
	dev->vdev->device_caps = cap->device_caps;
	/* >=linux-3.3.0 */
	cap->capabilities |= V4L2_CAP_DEVICE_CAPS;

	memset(cap->reserved, 0, sizeof(cap->reserved));
	CAM_DBG(CAM_V4L2, "device_caps 0x%x, capabilities 0x%x",
		cap->device_caps, cap->capabilities);

	return 0;
}

static int vidioc_enum_framesizes(struct file *file, void *fh,
		struct v4l2_frmsizeenum *argp)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener = fh_to_opener(fh);
	struct v4l2_streamdata *data;

	/* LATER: what does the index really  mean?
	 * if it's about enumerating formats, we can safely ignore it
	 * (CHECK)
	 */

	/* there can be only one... */
	if (argp->index)
		return -EINVAL;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}
	data = opener->data;

	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		/* format has already been negotiated
		 * cannot change during runtime
		 */
		argp->type = V4L2_FRMSIZE_TYPE_DISCRETE;

		argp->discrete.width = data->pix_format.width;
		argp->discrete.height = data->pix_format.height;
	} else {
		/* if the format has not been negotiated yet, we accept anything
		 */
		argp->type = V4L2_FRMSIZE_TYPE_CONTINUOUS;

		argp->stepwise.min_width = V4L2LOOPBACK_SIZE_MIN_WIDTH;
		argp->stepwise.min_height = V4L2LOOPBACK_SIZE_MIN_HEIGHT;

		argp->stepwise.max_width = max_width;
		argp->stepwise.max_height = max_height;

		argp->stepwise.step_width = 1;
		argp->stepwise.step_height = 1;
	}
	return 0;
}

/* ------------------ CAPTURE ----------------------- */

/* returns device formats
 * called on VIDIOC_ENUM_FMT, with v4l2_buf_type
 * set to V4L2_BUF_TYPE_VIDEO_CAPTURE
 */
static int    vidioc_enum_fmt_cap(struct file *file, void *fh,
		struct v4l2_fmtdesc *f)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}
	data = opener->data;

	if (f->index)
		return -EINVAL;

	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		const __u32 format = data->pix_format.pixelformat;

		snprintf(f->description, sizeof(f->description),
				"[%c%c%c%c]",
				(format >>  0) & 0xFF,
				(format >>  8) & 0xFF,
				(format >> 16) & 0xFF,
				(format >> 24) & 0xFF);

		f->pixelformat = data->pix_format.pixelformat;
	} else {
		return -EINVAL;
	}
	f->flags = 0;
	MARK();
	return 0;
}

/* returns current video format fmt
 * called on VIDIOC_G_FMT, with v4l2_buf_type
 * set to V4L2_BUF_TYPE_VIDEO_CAPTURE
 */
static int vidioc_g_fmt_cap(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}
	data = opener->data;

	CAM_DBG(CAM_V4L2, "%s opener: %p data: %p",
		opener, data);

	if (!(dev->state & V4L2L_READY_FOR_CAPTURE))
		return -EINVAL;

	fmt->fmt.pix = data->pix_format;
	MARK();
	return 0;
}

/* checks if it is OK to change to format fmt;
 * actual check is done by inner_try_fmt_cap
 * just checking that pixelformat is OK and set other parameters, app should
 * obey this decision
 * called on VIDIOC_TRY_FMT, with v4l2_buf_type
 * set to V4L2_BUF_TYPE_VIDEO_CAPTURE
 */
static int vidioc_try_fmt_cap(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;
	char buf[5];

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}
	data = opener->data;

	CAM_DBG(CAM_V4L2, "opener: %p data: %p",
		opener, data);

	if (!(dev->state & V4L2L_READY_FOR_CAPTURE)) {
		CAM_ERR(CAM_V4L2, "setting fmt_cap not possible yet");
		return -EBUSY;
	}

	if (fmt->fmt.pix.pixelformat != data->pix_format.pixelformat) {
		CAM_ERR(CAM_V4L2, "can't support set format now");
		return -EINVAL;
	}

	fmt->fmt.pix = data->pix_format;

	buf[4] = 0;
	CAM_DBG(CAM_V4L2, "capFOURCC=%s", fourcc2str(data->pix_format.pixelformat,
				buf));
	return 0;
}

/* sets new output format, if possible
 * actually format is set  by input and we even do not check it, just return
 * current one, but it is possible to set subregions of input TODO(vasaka)
 * called on VIDIOC_S_FMT, with v4l2_buf_type set to V4L2_BUF_TYPE_VIDEO_CAPTURE
 */
static int vidioc_s_fmt_cap(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	return vidioc_try_fmt_cap(file, priv, fmt);
}


/* ------------------ OUTPUT ----------------------- */

/* returns device formats;
 * LATER: allow all formats
 * called on VIDIOC_ENUM_FMT, with v4l2_buf_type
 * set to V4L2_BUF_TYPE_VIDEO_OUTPUT
 */
static int vidioc_enum_fmt_out(struct file *file, void *fh,
		struct v4l2_fmtdesc *f)
{
	struct v4l2_loopback_device *dev;
	const struct v4l2l_format *fmt;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;


	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}
	data = opener->data;

	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		const __u32 format = data->pix_format.pixelformat;

		/* format has been fixed by the writer,
		 * so only one single format is supported
		 */
		if (f->index)
			return -EINVAL;

		fmt = format_by_fourcc(format);
		if (fmt == NULL)
			return -EINVAL;

		f->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		/* f->flags = ??; */
		snprintf(f->description, sizeof(f->description), "%s",
				fmt->name);

		f->pixelformat = data->pix_format.pixelformat;
	} else {
		__u32 format;

		/* fill in a dummy format */
		/* coverity[unsigned_compare] */
		if (f->index < 0 || f->index >= FORMATS)
			return -EINVAL;

		fmt = &formats[f->index];

		f->pixelformat = fmt->fourcc;
		format = f->pixelformat;

		snprintf(f->description, sizeof(f->description), "%s",
				fmt->name);

	}
	f->flags = 0;

	return 0;
}

/* returns current video format fmt
 * NOTE: this is called from the producer
 * so if format has not been negotiated yet,
 * it should return ALL of available formats,
 * called on VIDIOC_G_FMT, with v4l2_buf_type set to V4L2_BUF_TYPE_VIDEO_OUTPUT
 */
static int vidioc_g_fmt_out(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}
	data = opener->data;

	CAM_DBG(CAM_V4L2, "opener: %p data: %p",
		opener, data);

	/*
	 * LATER: this should return the currently valid format
	 * gstreamer doesn't like it, if this returns -EINVAL, as it
	 * then concludes that there is _no_ valid format
	 * CHECK whether this assumption is wrong,
	 * or whether we have to always provide a valid format
	 */

	fmt->fmt.pix = data->pix_format;
	return 0;
}

/* checks if it is OK to change to format fmt;
 * if format is negotiated do not change it
 * called on VIDIOC_TRY_FMT with v4l2_buf_type set to V4L2_BUF_TYPE_VIDEO_OUTPUT
 */
static int vidioc_try_fmt_out(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;
	__u32 w = fmt->fmt.pix.width;
	__u32 h = fmt->fmt.pix.height;
	__u32 pixfmt = fmt->fmt.pix.pixelformat;
	const struct v4l2l_format *format = format_by_fourcc(pixfmt);
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}
	data = opener->data;

	CAM_DBG(CAM_V4L2, "opener: %p data: %p",
		opener, data);

	/* TODO(vasaka) loopback does not care about formats writer want to set,
	 * maybe it is a good idea to restrict format somehow
	 */

	if (w > max_width)
		w = max_width;
	if (h > max_height)
		h = max_height;

	CAM_DBG(CAM_V4L2, "trying image %dx%d", w, h);

	if (w < 1)
		w = V4L2LOOPBACK_SIZE_DEFAULT_WIDTH;

	if (h < 1)
		h = V4L2LOOPBACK_SIZE_DEFAULT_HEIGHT;

	if (format == NULL)
		format = &formats[0];

	pix_format_set_size(&fmt->fmt.pix, format, w, h);

	fmt->fmt.pix.pixelformat = format->fourcc;
	fmt->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

	if (fmt->fmt.pix.field == V4L2_FIELD_ANY)
		fmt->fmt.pix.field = V4L2_FIELD_NONE;

	/* FIXXME: try_fmt should never modify the device-state */
	data->pix_format = fmt->fmt.pix;

	return 0;
}

/* sets new output format, if possible;
 * allocate data here because we do not know if it will be streaming or
 * read/write IO
 * called on VIDIOC_S_FMT with v4l2_buf_type set to V4L2_BUF_TYPE_VIDEO_OUTPUT
 */
static int vidioc_s_fmt_out(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;
	char buf[5];
	int ret = 0;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}
	data = opener->data;

	CAM_DBG(CAM_V4L2, "opener: %p data: %p",
		opener, data);

	ret = vidioc_try_fmt_out(file, priv, fmt);

	CAM_DBG(CAM_V4L2, "s_fmt_out(%d) %d...%d %d %d", ret,
		(dev->state & V4L2L_READY_FOR_CAPTURE),
		fmt->fmt.pix.width, fmt->fmt.pix.height,
		data->pix_format.sizeimage);

	buf[4] = 0;
	CAM_DBG(CAM_V4L2, "outFOURCC=%s",
		fourcc2str(data->pix_format.pixelformat, buf));

	if (ret < 0)
		return ret;

	data->buffer_size = PAGE_ALIGN(data->pix_format.sizeimage);
	fmt->fmt.pix.sizeimage = data->buffer_size;
	CAM_DBG(CAM_V4L2, "buffer size %u", data->buffer_size);

	return ret;
}

/*#define V4L2L_OVERLAY*/
#ifdef V4L2L_OVERLAY
/* ------------------ OVERLAY ----------------------- */
/* currently unsupported */
/* GSTreamer's v4l2sink is buggy, as it requires the overlay to work
 * while it should only require it, if overlay is requested
 * once the gstreamer element is fixed, remove the overlay dummies
 */
#warning OVERLAY dummies
static int vidioc_g_fmt_overlay(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	return 0;
}

static int vidioc_s_fmt_overlay(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	return 0;
}
#endif /* V4L2L_OVERLAY */


/* ------------------ PARAMs ----------------------- */

/* get some data flow parameters, only capability, fps and readbuffers has
 * effect on this driver
 * called on VIDIOC_G_PARM
 */
static int vidioc_g_parm(struct file *file, void *priv,
		struct v4l2_streamparm *parm)
{
	/* do not care about type of opener, hope this enums would always be
	 * compatible
	 */
	struct v4l2_loopback_device *dev;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
		parm->type !=  V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	return 0;
}

/* get some data flow parameters, only capability, fps and readbuffers has
 * effect on this driver
 * called on VIDIOC_S_PARM
 */
static int vidioc_s_parm(struct file *file, void *priv,
		struct v4l2_streamparm *parm)
{
	struct v4l2_loopback_device *dev;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE &&
		parm->type !=  V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	return 0;
}

/* ------------------ Ctrl ----------------------- */

static int v4l2loopback_s_ctrl(struct v4l2_ctrl *ctrl)
{
	return 0;
}

int v4l2loopback_g_ctrl(struct v4l2_ctrl *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = 4;
		CAM_INFO(CAM_V4L2, "min_buffers is 4");
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int v4l2loopback_datasize_try_ctrl(struct v4l2_ctrl *ctrl)
{
	return -EINVAL;
}

#ifdef V4L2LOOPBACK_WITH_OUTPUT

/* returns set of device outputs, in our case there is only one
 * called on VIDIOC_ENUMOUTPUT
 */
static int vidioc_enum_output(struct file *file, void *fh,
		struct v4l2_output *outp)
{
	__u32 index = outp->index;
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);

	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	MARK();
	if (!dev->announce_all_caps && dev->state != V4L2L_READY_FOR_OUTPUT) {
		CAM_ERR(CAM_V4L2, "is enotify");
		return -ENOTTY;
	}

	if (index != 0)
		return -EINVAL;

	/* clear all data (including the reserved fields) */
	memset(outp, 0, sizeof(*outp));

	outp->index = index;
	strlcpy(outp->name, "loopback in", sizeof(outp->name));
	outp->type = V4L2_OUTPUT_TYPE_ANALOG;
	outp->audioset = 0;
	outp->modulator = 0;
#ifdef V4L2LOOPBACK_WITH_STD
	outp->std = V4L2_STD_ALL;
# ifdef V4L2_OUT_CAP_STD
	outp->capabilities |= V4L2_OUT_CAP_STD;
# endif /*  V4L2_OUT_CAP_STD */
#endif /* V4L2LOOPBACK_WITH_STD */

	return 0;
}


/* which output is currently active,
 * called on VIDIOC_G_OUTPUT
 */
static int vidioc_g_output(struct file *file, void *fh, unsigned int *i)
{
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);

	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	if (!dev->announce_all_caps && dev->state != V4L2L_READY_FOR_OUTPUT) {
		CAM_ERR(CAM_V4L2, "is enotify");
		return -ENOTTY;
	}

	if (i)
		*i = 0;

	return 0;
}

/* set output, can make sense if we have more than one video src,
 * called on VIDIOC_S_OUTPUT
 */
static int vidioc_s_output(struct file *file, void *fh, unsigned int i)
{
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);

	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	if (!dev->announce_all_caps && dev->state != V4L2L_READY_FOR_OUTPUT) {
		CAM_ERR(CAM_V4L2, "is enotify");
		return -ENOTTY;
	}

	if (i)
		return -EINVAL;

	return 0;
}
#endif

/* --------------- V4L2 ioctl buffer related calls ----------------- */

/* negotiate buffer type
 * only mmap streaming supported
 * called on VIDIOC_REQBUFS
 */
static int vidioc_reqbufs(struct file *file, void *fh,
		struct v4l2_requestbuffers *b)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(fh);
	data = opener->data;

	/* release buffer when request buffer 0*/
	if (b->count == 0) {
		free_buffers(data);
		if (opener->connected_opener)
			send_v4l2_event_ex(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_ALLOC_BUFS, 0);
		data->used_buffers = 0;
		return 0;
	}

	init_buffers(data);
	switch (b->memory) {
	case V4L2_MEMORY_USERPTR:
	case V4L2_MEMORY_MMAP:
		/* do nothing here, buffers are always allocated */
		if (b->count < 1 || data->buffers_number < 1)
			return 0;

		if (b->count > data->buffers_number)
			b->count = data->buffers_number;

		data->used_buffers = b->count;
		data->use_buf_width = b->reserved[0];

		CAM_DBG(CAM_V4L2, "use_buf_width %d", data->use_buf_width);

		return 0;
	default:
		return -EINVAL;
	}
}

/* returns buffer asked for;
 * give app as many buffers as it wants, if it less than MAX,
 * but map them in our inner buffers
 * called on VIDIOC_QUERYBUF
 */
static int vidioc_querybuf(struct file *file, void *fh, struct v4l2_buffer *b)
{
	enum v4l2_buf_type type;
	int index;

	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	MARK();

	type = b->type;
	index = b->index;

	opener = fh_to_opener(fh);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;

	if ((b->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) &&
			(b->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)) {
		return -EINVAL;
	}
	if (b->index > max_buffers)
		return -EINVAL;


	*b = data->buffers[b->index % data->used_buffers].buffer;
	b->type = type;
	b->index = index;

	CAM_DBG(CAM_V4L2, "buffer type: %d (of %d with size=%ld)",
			b->memory, data->buffers_number, data->buffer_size);
	CAM_DBG(CAM_V4L2, "buffer %d %u", b->index, b->length);

	/*  Hopefully fix 'DQBUF return bad index if queue bigger
	 *  then 2 for capture'
	 *  https://github.com/umlaeute/v4l2loopback/issues/60
	 */
	b->flags &= ~V4L2_BUF_FLAG_DONE;
	b->flags |= V4L2_BUF_FLAG_QUEUED;

	return 0;
}

static int vidioc_expbuf(struct file *file, void *fh,
			struct v4l2_exportbuffer *e)
{
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;
	int fd = 0;
	int rc;
	struct dma_buf *dmabuf = NULL;

	MARK();

	opener = fh_to_opener(fh);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;

	if (e->index == -1U) {
		CAM_INFO(CAM_V4L2, "support DMABUF");
		return 0;
	}

	if (e->index >= data->used_buffers) {
		CAM_WARN(CAM_V4L2, "invalid index %d", e->index);
		return -EINVAL;
	}

	switch (e->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE: {
		CAM_DBG(CAM_V4L2, "export buffer %d", e->index);
		if (data->dmabufs[0] == NULL ||
			data->dmabufs[e->index] == NULL) {
			data->io_mode = V4L2L_IO_MODE_DMABUF;

			rc = allocate_dma_buffers(opener, data);
			if (rc) {
				CAM_ERR(CAM_V4L2, "fail to allocate dma buffers");
				return -EINVAL;
			}
		}

		fd = dma_buf_fd(data->dmabufs[e->index], O_CLOEXEC);
		if (fd < 0) {
			CAM_ERR(CAM_V4L2, "dma get fd fail, *fd=%d", fd);
			return -EINVAL;
		}

		/*
		 * increment the ref count so that ref count becomes 2 here
		 * when we close fd, refcount becomes 1 and when we do
		 * dmap_put_buf, ref count becomes 0 and memory will be freed.
		 */
		dmabuf = dma_buf_get(fd);
		if (dmabuf == NULL) {
			CAM_ERR(CAM_V4L2, "dma_buf_get failed, fd=%d", fd);
			return -EINVAL;
		}

		break;
	}
	case V4L2_BUF_TYPE_VIDEO_OUTPUT: {
		break;
	}

	}

	CAM_DBG(CAM_V4L2, "exit");

	e->fd = fd;
	return 0;
}

static int can_read(struct v4l2_streamdata *data,
		struct v4l2_loopback_opener *opener)
{
	int ret;

	mutex_lock(&data->capbufs_mutex);
	ret = !list_empty(&data->capbufs_list);
	mutex_unlock(&data->capbufs_mutex);

	return ret;
}


static int set_bufstate(struct v4l2_streamdata *data,
	struct v4l2l_buffer *b, enum V4L2_LOOPBACK_BUF_STATE state)
{
	int ret = 0;

	mutex_lock(&data->buf_mutex);

	switch (b->state) {
	case V4L2L_BUF_PROXY_ACQUIRED:
		if (state == V4L2L_BUF_DONE_Q)
			b->state = state;
		else
			ret = -1;
		break;
	case V4L2L_BUF_DONE_Q:
		if (state == V4L2L_BUF_USER_ACQUIRED)
			b->state = state;
		else
			ret = -1;
		break;
	case V4L2L_BUF_READY_Q:
		if (state == V4L2L_BUF_PROXY_ACQUIRED)
			b->state = state;
		else
			ret = -1;
		break;
	case V4L2L_BUF_USER_ACQUIRED:
		if (state == V4L2L_BUF_READY_Q)
			b->state = state;
		else
			ret = -1;
		break;
	case V4L2L_BUF_PENDING:
		ret = -1;
		break;
	}

	if (ret == -1) {
		CAM_ERR(CAM_V4L2, "[data %p] fail to set buffer state %u current state %u",
			data, state, b->state);
	}

	mutex_unlock(&data->buf_mutex);

	return ret;
}

static void set_allbufs_state(struct v4l2_streamdata *data,
	enum V4L2_LOOPBACK_BUF_STATE state)
{
	int i = 0;

	mutex_lock(&data->buf_mutex);
	for (i = 0; i < data->used_buffers; ++i)
		data->buffers[i].state = state;

	mutex_unlock(&data->buf_mutex);
}

/* put buffer to queue
 * called on VIDIOC_QBUF
 */
static int vidioc_qbuf(struct file *file,
		void *private_data, struct v4l2_buffer *buf)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;
	struct v4l2l_buffer *b;
	int index;
	int rc;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;
	if (!data) {
		CAM_ERR(CAM_V4L2, "data is null");
		return -EINVAL;
	}

	if (buf->index > max_buffers)
		return -EINVAL;

	index = buf->index % data->used_buffers;
	b = &data->buffers[index];

	switch (buf->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		CAM_DBG(CAM_V4L2, "[dev %s] capture QBUF index: %d", dev->vdev->name, index);
		rc = set_bufstate(data, b, V4L2L_BUF_READY_Q);
		if (rc < 0)
			CAM_ERR(CAM_V4L2, "[dev %s] capture QBUF index: %d fail",
				dev->vdev->name, b->buffer.index);
		else {
			/* since when app VIDIOC_REQBUFS, proxy side will allocate buf and export,
			 * so before start, no need qbuf the outbufs_list for proxy to dquf.
			 */
			if (data->is_streaming) {
				mutex_lock(&data->outbufs_mutex);
				list_add_tail(&b->list_head, &data->outbufs_list);
				mutex_unlock(&data->outbufs_mutex);
				send_v4l2_event(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
					AIS_V4L2_OUTPUT_BUF_READY);
			}
		}
		return rc;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT: {
		__u64 payload = 0;

		CAM_DBG(CAM_V4L2, "[dev %s] output QBUF index: %d",
				dev->vdev->name, index);
		if (buf->timestamp.tv_sec == 0 && buf->timestamp.tv_usec == 0) {
			struct timespec64 ts;

			cam_common_util_get_curr_timestamp(&ts);
			b->buffer.timestamp.tv_sec = ts.tv_sec;
			b->buffer.timestamp.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
		} else
			b->buffer.timestamp = buf->timestamp;

		b->buffer.sequence = buf->sequence;
		rc = set_bufstate(data, b, V4L2L_BUF_DONE_Q);
		if (rc < 0)
			CAM_ERR(CAM_V4L2, "[dev %s] output QBUF index: %d fail",
				dev->vdev->name, b->buffer.index);
		else {
			mutex_lock(&data->capbufs_mutex);
			list_add_tail(&b->list_head, &data->capbufs_list);
			mutex_unlock(&data->capbufs_mutex);

			// copy from user to b->ext
			payload = *((__u64 *)&buf->reserved2);
			if (payload != 0) {
				if (copy_from_user(&(b->ext),
					u64_to_user_ptr(payload),
					sizeof(b->ext))) {
					rc = -EFAULT;
					CAM_ERR(CAM_V4L2, "copy_from_user fail on qbuf");
				}
			}

			wake_up_all(&data->read_event);
		}
		return rc;
	}
	default:
		CAM_ERR(CAM_V4L2, "unsupported buf type %d", buf->type);
		return -EINVAL;
	}
}

/* put buffer to dequeue
 * called on VIDIOC_DQBUF
 */
static int vidioc_dqbuf(struct file *file,
		void *private_data, struct v4l2_buffer *buf)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;
	struct v4l2l_buffer *b;
	int rc = 0;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;
	if (!data) {
		CAM_ERR(CAM_V4L2, "data is null");
		return -EINVAL;
	}

	switch (buf->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		mutex_lock(&data->capbufs_mutex);
		if (!list_empty(&data->capbufs_list)) {
			b = list_first_entry(&data->capbufs_list,
						struct v4l2l_buffer, list_head);

			rc = set_bufstate(data, b, V4L2L_BUF_USER_ACQUIRED);
			if (rc < 0) {
				mutex_unlock(&data->capbufs_mutex);
				CAM_ERR(CAM_V4L2, "[dev %s] capture DQBUF index: %d fail",
					dev->vdev->name, b->buffer.index);
			} else {
				__u64 payload = 0;

				list_del_init(&b->list_head);
				mutex_unlock(&data->capbufs_mutex);

				CAM_DBG(CAM_V4L2, "[dev %s] capture DQBUF index: %d",
					dev->vdev->name, b->buffer.index);
				// copy to user *buf.reversed2 from b->ext
				payload = *((__u64 *)&buf->reserved2);
				if (payload != 0) {
					if (copy_to_user(u64_to_user_ptr(payload),
						&b->ext,
						sizeof(b->ext))) {
						rc = -EFAULT;
						CAM_ERR(CAM_V4L2, "copy_to_user fail on dqbuf");
					}
					b->ext.batch_num = 0;
				}

				*buf = b->buffer;
			}
		} else {
			mutex_unlock(&data->capbufs_mutex);

			CAM_WARN(CAM_V4L2, "[dev %s] capture list is empty", dev->vdev->name);
			if (file->f_flags & O_NONBLOCK)
				return -EAGAIN;

			wait_event_interruptible(data->read_event, can_read(data, opener));
		}

		return rc;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		mutex_lock(&data->outbufs_mutex);
		if (!list_empty(&data->outbufs_list)) {
			b = list_first_entry(&data->outbufs_list,
					struct v4l2l_buffer, list_head);

			rc = set_bufstate(data, b, V4L2L_BUF_PROXY_ACQUIRED);
			if (rc < 0) {
				mutex_unlock(&data->outbufs_mutex);
				CAM_ERR(CAM_V4L2, "[dev %s] output DQBUF index: %d fail",
					dev->vdev->name, b->buffer.index);
			} else {
				list_del_init(&b->list_head);
				CAM_DBG(CAM_V4L2, "[dev %s] output DQBUF index: %d",
					dev->vdev->name, b->buffer.index);
				mutex_unlock(&data->outbufs_mutex);

				*buf = b->buffer;
				buf->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			}
		} else {
			mutex_unlock(&data->outbufs_mutex);
			CAM_WARN(CAM_V4L2, "[dev %s] output list is empty", dev->vdev->name);
			// for proxy, must use NONBLOCK method
			return -EAGAIN;
		}
		return rc;
	default:
		CAM_ERR(CAM_V4L2, "[dev %s] unsupported buf type %d",
			dev->vdev->name, buf->type);
		return -EINVAL;
	}
}

/* ------------- STREAMING ------------------- */

/* start streaming
 * called on VIDIOC_STREAMON
 */
static int vidioc_streamon(struct file *file,
		void *private_data, enum v4l2_buf_type type)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;
	struct v4l2l_buffer *pos, *n;
	int rc = 0;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;

	CAM_INFO(CAM_V4L2, "opener: %p data: %p", opener, data);

	switch (type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		rc = 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE: {
		if (data->is_streaming) {
			CAM_WARN(CAM_V4L2, "data: %p already is streamoned, return", data);
			rc = -EINVAL;
			break;
		}
		CAM_INFO(CAM_V4L2, "data: %p streamon", data);
		set_allbufs_state(data, V4L2L_BUF_PROXY_ACQUIRED);
		mutex_lock(&data->outbufs_mutex);
		list_for_each_entry_safe(pos, n,
			&data->outbufs_list, list_head) {
			list_del_init(&pos->list_head);
		}
		INIT_LIST_HEAD(&data->outbufs_list);
		mutex_unlock(&data->outbufs_mutex);

		mutex_lock(&data->capbufs_mutex);
		list_for_each_entry_safe(pos, n,
			&data->capbufs_list, list_head) {
			list_del_init(&pos->list_head);
		}
		INIT_LIST_HEAD(&data->capbufs_list);
		mutex_unlock(&data->capbufs_mutex);

		send_v4l2_event(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_START_INPUT);
		rc = wait_for_completion_timeout(&data->ctrl_complete,
					msecs_to_jiffies(START_TIMEOUT));
		if (rc) {
			rc = data->qcarcam_ctrl_ret;
			data->is_streaming = 1;
			if (data->qcarcam_ctrl_ret == 0)
				dev->stream_status = 1;
		} else {
			CAM_ERR(CAM_V4L2, "streamon fail, timeout %d", rc);
			rc = -ETIMEDOUT;
		}
		break;
	}
	default:
		rc = -EINVAL;
	}

	return rc;
}

/* stop streaming
 * called on VIDIOC_STREAMOFF
 */
static int vidioc_streamoff(struct file *file,
		void *private_data, enum v4l2_buf_type type)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;
	struct v4l2l_buffer *pos, *n;

	int rc = 0;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	opener = fh_to_opener(file->private_data);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;
	if (!data) {
		CAM_ERR(CAM_V4L2, "data is null");
		return -EINVAL;
	}

	CAM_INFO(CAM_V4L2, "opener: %p data: %p", opener, data);

	data->is_streaming = 0;

	switch (type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		rc = 0;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		CAM_INFO(CAM_V4L2, "streamoff");
		set_allbufs_state(data, V4L2L_BUF_PENDING);

		if (opener->connected_opener) {
			send_v4l2_event(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_STOP_INPUT);
			rc = wait_for_completion_timeout(&data->ctrl_complete,
						msecs_to_jiffies(STOP_TIMEOUT));
			if (rc) {
				rc = data->qcarcam_ctrl_ret;
				if (data->qcarcam_ctrl_ret == 0)
					dev->stream_status = 0;
				CAM_INFO(CAM_V4L2, "clear list when streamoff");
				mutex_lock(&data->outbufs_mutex);
				list_for_each_entry_safe(pos, n,
					&data->outbufs_list, list_head) {
					list_del_init(&pos->list_head);
				}
				INIT_LIST_HEAD(&data->outbufs_list);
				mutex_unlock(&data->outbufs_mutex);

				mutex_lock(&data->capbufs_mutex);
				list_for_each_entry_safe(pos, n,
					&data->capbufs_list, list_head) {
					list_del_init(&pos->list_head);
				}
				INIT_LIST_HEAD(&data->capbufs_list);
				mutex_unlock(&data->capbufs_mutex);
			} else {
				CAM_ERR(CAM_V4L2, "streamoff fail, timeout %d", rc);
				rc = -ETIMEDOUT;
			}
		}

		break;
	default:
		rc = -EINVAL;
	}

	return rc;
}

static int cam_subscribe_event(struct v4l2_fh *fh,
	const struct v4l2_event_subscription *sub)
{
	return v4l2_event_subscribe(fh, sub, AIS_V4L2LOOPBACK_EVENT_MAX, NULL);
}

static int cam_unsubscribe_event(struct v4l2_fh *fh,
	const struct v4l2_event_subscription *sub)
{
	return v4l2_event_unsubscribe(fh, sub);
}

static int process_capture_set_param(struct ais_v4l2_control_t *kcmd,
	struct v4l2_loopback_opener *opener)
{
	int rc = 0;

	if (kcmd->size > MAX_AIS_V4L2_PAYLOAD_SIZE) {
		rc = -EINVAL;
	} else if (u64_to_user_ptr(kcmd->payload) == NULL) {
		rc = -EINVAL;
		CAM_ERR(CAM_V4L2, "payload is NULL on set param");
	} else if (!opener->data) {
		CAM_ERR(CAM_V4L2, "data is null");
		return -EINVAL;
	} else if (copy_from_user(opener->data->qcarcam_param,
			u64_to_user_ptr(kcmd->payload),
			kcmd->size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "copy_from_user fail on set param");
	} else {
		CAM_DBG(CAM_V4L2, "s_param %u", kcmd->param_type);

		opener->data->qcarcam_param_size = kcmd->size;
		opener->data->qcarcam_code = kcmd->param_type;

		send_v4l2_event_ex(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_SET_PARAM, kcmd->param_type);

		CAM_DBG(CAM_V4L2, "s_param %u debug1", kcmd->param_type);

		/* wait for the signal */
		rc = wait_for_completion_timeout(&opener->data->sparam_complete,
				msecs_to_jiffies(SPARAM_TIMEOUT));

		CAM_DBG(CAM_V4L2, "s_param %u debug2", kcmd->param_type);
		if (rc) {
			rc = opener->data->qcarcam_sparam_ret;
		} else {
			CAM_ERR(CAM_V4L2, "s_param fail, timeout %d", rc);
			rc = -ETIMEDOUT;
		}
	}
	return rc;
}

static int process_capture_get_param(struct ais_v4l2_control_t *kcmd,
	struct v4l2_loopback_opener *opener)
{
	int rc = 0;

	if (kcmd->size > MAX_AIS_V4L2_PAYLOAD_SIZE) {
		rc = -EINVAL;
	} else if (u64_to_user_ptr(kcmd->payload) == NULL) {
		rc = -EINVAL;
		CAM_ERR(CAM_V4L2, "payload is NULL on set param");
	} else {
		CAM_DBG(CAM_V4L2, "get_param %u", kcmd->param_type);

		/* send the payload to userspace by v4l2_event
		 * when payload size is large, can't send by v4l2
		 */
		if (!opener->data) {
			CAM_ERR(CAM_V4L2, "data is null");
			return -EINVAL;
		}

		if (kcmd->size > MAX_AIS_V4L2_PARAM_EVNET_SIZE &&
			copy_from_user(opener->data->qcarcam_param,
			u64_to_user_ptr(kcmd->payload),
			kcmd->size)) {
			rc = -EFAULT;
			CAM_ERR(CAM_V4L2, "copy_from_user fail on get param");
			return rc;
		}

		opener->data->qcarcam_param_size = kcmd->size;
		opener->data->qcarcam_code = kcmd->param_type;

		/* send get_param to output end */
		send_v4l2_event_ex2(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_GET_PARAM, kcmd->param_type, kcmd->size, kcmd->payload);

		/* wait for the signal */
		rc = wait_for_completion_timeout(&opener->data->gparam_complete,
			msecs_to_jiffies(GPARAM_TIMEOUT));
		if (rc) {
			rc = 0;
		} else {
			CAM_ERR(CAM_V4L2, "g_param fail, timeout %d", rc);
			rc = -ETIMEDOUT;
			return rc;
		}

		if (opener->data->qcarcam_param_size == 0) {
			/* g_param fail */
			rc = -EINVAL;
			CAM_ERR(CAM_V4L2, "get param size failed");
		} else if (copy_to_user(u64_to_user_ptr(kcmd->payload),
					opener->data->qcarcam_param,
					opener->data->qcarcam_param_size)) {
			/* copy the param from kernel */
			rc = -EFAULT;
			CAM_ERR(CAM_V4L2, "copy_from_user fail on get param");
		} else {
			kcmd->size = opener->data->qcarcam_param_size;
			kcmd->param_type = opener->data->qcarcam_code;
			CAM_DBG(CAM_V4L2, "succeed to get param");
		}
	}
	return rc;
}

static int process_capture_cmd(struct v4l2_loopback_device *dev,
	struct ais_v4l2_control_t *kcmd, void *fh)
{
	int rc = 0;
	struct v4l2_loopback_opener *opener;

	opener = fh_to_opener(fh);
	if (!opener || !opener->connected_opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	CAM_DBG(CAM_V4L2, "opener: %p cmd %d",
		opener, kcmd->cmd);

	switch (kcmd->cmd) {
	case AIS_V4L2_CAPTURE_PRIV_SET_PARAM: {
		rc = process_capture_set_param(kcmd, opener);
		break;
	}
	case AIS_V4L2_CAPTURE_PRIV_GET_PARAM: {
		rc = process_capture_get_param(kcmd, opener);
		break;
	}
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int process_set_param(struct ais_v4l2_control_t *kcmd, struct v4l2_loopback_opener *opener)
{
	struct v4l2_event event;
	int rc = 0;

	if (kcmd->size > MAX_AIS_V4L2_PARAM_EVNET_SIZE) {
		rc = -EINVAL;
		CAM_ERR(CAM_V4L2, "fail to set param event, size=%u",
			kcmd->size);
	} else if (copy_from_user(&event.u.data[2],
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail to copy from user on set param event");
	} else {
		event.type = AIS_V4L2_CLIENT_CAPTURE;
		event.id = AIS_V4L2_PARAM_EVENT;

		event.u.data[0] = kcmd->param_type;
		event.u.data[1] = kcmd->size;

		if (!opener->connected_opener) {
			CAM_ERR(CAM_V4L2, "connected_opener is null");
			return -EINVAL;
		}
		v4l2_event_queue_fh(&(opener->connected_opener->fh), &event);
		CAM_DBG(CAM_V4L2, "send AIS_V4L2_PARAM_EVENT :%d %d",
				event.u.data[2], event.u.data[3]);
	}
	return rc;
}

static int process_get_param(struct ais_v4l2_control_t *kcmd, struct v4l2_streamdata *data)
{
	int rc = 0;

	/* copy the param from kernel */
	CAM_DBG(CAM_V4L2, "AIS_V4L2_OUTPUT_PRIV_GET_PARAM from opener");
	if (copy_to_user(u64_to_user_ptr(kcmd->payload),
					data->qcarcam_param,
					data->qcarcam_param_size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail to AIS_V4L2_OUTPUT_PRIV_GET_PARAM");
	} else {
		kcmd->size = data->qcarcam_param_size;
		kcmd->param_type = data->qcarcam_code;
	}
	return rc;
}
static int process_set_param2(struct ais_v4l2_control_t *kcmd, struct v4l2_streamdata *data)
{
	int rc = 0;

	if (kcmd->size > MAX_AIS_V4L2_PAYLOAD_SIZE) {
		rc = -EINVAL;
	} else if (copy_from_user(data->qcarcam_param,
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail to AIS_V4L2_OUTPUT_PRIV_SET_PARAM2");
	} else {
		data->qcarcam_param_size = kcmd->size;
		data->qcarcam_code = kcmd->param_type;
		/* signal capture_g_param_cond */
		complete(&data->gparam_complete);
	}
	return rc;
}

static int process_set_bufs(struct ais_v4l2_control_t *kcmd, struct v4l2_streamdata *data)
{
	struct ais_v4l2_buffers_t bufs;
	int rc = 0;
	struct dma_buf *dmabuf = NULL;
	uint32_t i;

	if (kcmd->size != sizeof(bufs)) {
		rc = -EINVAL;
		CAM_ERR(CAM_V4L2, "kcmd->size != sizeof(bufs)");
	} else if (copy_from_user(&bufs,
				u64_to_user_ptr(kcmd->payload),
				sizeof(bufs.nbufs))) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail get count for AIS_V4L2_OUTPUT_PRIV_SET_BUFS");
	} else {
		if (bufs.nbufs > MAX_AIS_BUFFERS_NUM) {
			rc = -EFAULT;
			pr_err("invalid buffer count for AIS_V4L2_OUTPUT_PRIV_SET_BUFS\n");
		} else if (copy_from_user(&bufs,
			u64_to_user_ptr(kcmd->payload),
			sizeof(bufs.nbufs) + sizeof(int) * bufs.nbufs)) {
			rc = -EFAULT;
			CAM_ERR(CAM_V4L2, "fail to get fds AIS_V4L2_OUTPUT_PRIV_SET_BUFS");
		} else {
			CAM_INFO(CAM_V4L2, "v4l2 setbufs %u begin", bufs.nbufs);

			for (i = 0; i < bufs.nbufs; ++i) {
				dmabuf = dma_buf_get(bufs.fds[i]);
				if (dmabuf == NULL) {
					CAM_ERR(CAM_V4L2, "dma_buf_get failed, fd=%d",
						bufs.fds[i]);
					return -EINVAL;
				}
				data->dmabufs[i] = dmabuf;
			}

			data->allocbufs_ret = kcmd->ctrl_ret;
			complete(&data->allocbufs_complete);

			CAM_INFO(CAM_V4L2, "v4l2 setbufs %u end", bufs.nbufs);
		}
	}
	return rc;
}

static int process_set_input_event(struct ais_v4l2_control_t *kcmd,
	struct v4l2_loopback_opener *opener)
{
	struct v4l2_event event;
	int rc = 0;

	if (copy_from_user(&event.u.data[2],
			u64_to_user_ptr(kcmd->payload),
			kcmd->size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail to copy from user on input signal");
	} else {
		event.type = AIS_V4L2_CLIENT_CAPTURE;
		event.id = AIS_V4L2_EVENT_INPUT_SIGNAL;

		event.u.data[0] = kcmd->param_type;
		event.u.data[1] = kcmd->size;
		if (opener->connected_opener)
			v4l2_event_queue_fh(&(opener->connected_opener->fh), &event);
		CAM_DBG(CAM_V4L2, "send AIS_V4L2_EVENT_INPUT_SIGNAL : %d",
			event.u.data[2]);
	}
	return rc;
}

static int process_set_error_event(struct ais_v4l2_control_t *kcmd,
	struct v4l2_loopback_opener *opener)
{
	struct v4l2_event event;
	int rc = 0;

	if (copy_from_user(&event.u.data[2],
			u64_to_user_ptr(kcmd->payload),
			kcmd->size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail to copy from user on input signal");
	} else {
		event.type = AIS_V4L2_CLIENT_CAPTURE;
		event.id = AIS_V4L2_EVENT_ERROR;

		event.u.data[0] = kcmd->param_type;
		event.u.data[1] = kcmd->size;
		if (opener->connected_opener)
			v4l2_event_queue_fh(&(opener->connected_opener->fh), &event);
		CAM_DBG(CAM_V4L2, "send AIS_V4L2_EVENT_ERROR :%d",
			event.u.data[2]);
	}
	return rc;
}

static int process_set_frame_drop_event(struct ais_v4l2_control_t *kcmd,
	struct v4l2_loopback_opener *opener)
{
	struct v4l2_event event;
	int rc = 0;

	if (copy_from_user(&event.u.data[2],
			u64_to_user_ptr(kcmd->payload),
			kcmd->size)) {
		rc = -EFAULT;
		CAM_ERR(CAM_V4L2, "fail to copy from user on drop event\n");
	} else {
		event.type = AIS_V4L2_CLIENT_CAPTURE;
		event.id = AIS_V4L2_EVENT_FRAME_DROP;
		event.u.data[0] = kcmd->param_type;
		event.u.data[1] = kcmd->size;
		if (opener->connected_opener)
			v4l2_event_queue_fh(&(opener->connected_opener->fh), &event);
		CAM_DBG(CAM_V4L2, "send AIS_V4L2_EVENT_FRAME_DROP :%d\n",
			event.u.data[2]);
	}
	return rc;
}

static int process_output_cmd(struct v4l2_loopback_device *dev,
	struct ais_v4l2_control_t *kcmd, void *fh)
{
	int rc = 0;
	struct v4l2_loopback_opener *opener;
	struct v4l2_streamdata *data;

	opener = fh_to_opener(fh);
	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;

	CAM_DBG(CAM_V4L2, "opener: %p data: %p cmd %d",
		opener, data, kcmd->cmd);

	switch (kcmd->cmd) {
	case AIS_V4L2_OUTPUT_PRIV_SET_PARAM_EVENT: {
		rc = process_set_param(kcmd, opener);
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_PARAM_RET:
		/* signal capture_s_param_cond */
		data->qcarcam_sparam_ret = kcmd->param_type;
		complete(&data->sparam_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_SET_PARAM2:
		rc = process_set_param2(kcmd, data);
		break;
	case AIS_V4L2_OUTPUT_PRIV_GET_PARAM:
		rc = process_get_param(kcmd, data);
		break;
	case AIS_V4L2_OUTPUT_PRIV_OPEN_RET:
		dev->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		dev->open_ret_fh = fh;
		complete(&dev->open_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_CLOSE_RET:
		dev->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		complete(&dev->close_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_START_RET:
		data->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		complete(&data->ctrl_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_STOP_RET:
		data->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		complete(&data->ctrl_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_SET_BUFS: {
		rc = process_set_bufs(kcmd, data);
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_INPUT_SIGNAL_EVENT: {
		rc = process_set_input_event(kcmd, opener);
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_ERROR_EVENT: {
		rc = process_set_error_event(kcmd, opener);
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_FRAME_DROP_EVENT: {
		rc = process_set_frame_drop_event(kcmd, opener);
		break;
	}
	default:
	break;
	}
	return rc;
}


static long ais_v4l2loopback_dev_ioctl(struct file *file, void *fh,
			bool valid_prio, unsigned int cmd, void *arg)
{
	int32_t rc = 0;

	struct v4l2_loopback_device *dev;

	struct ais_v4l2_control_t *kcmd = (struct ais_v4l2_control_t *)arg;

	MARK();
	dev = v4l2loopback_getdevice(file);

	CAM_DBG(CAM_V4L2, "enter");

	if (!dev) {
		CAM_ERR(CAM_V4L2, "v4l2loopback dev NULL");
		return -EINVAL;
	}

	if (!kcmd || !arg)
		return -EINVAL;

	if (cmd != VIDIOC_CAM_CONTROL)
		return -ENOIOCTLCMD;

	switch (kcmd->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		rc = process_capture_cmd(dev, kcmd, fh);
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		rc = process_output_cmd(dev, kcmd, fh);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static struct v4l2_loopback_opener *create_opener(struct file *file,
	enum v4l2_loopback_opener_type etype)
{
	/* create the opener */
	struct v4l2_loopback_opener *opener = kzalloc(sizeof(*opener), GFP_KERNEL);

	if (opener == NULL)
		return NULL;

	opener->type = etype;
	v4l2_fh_init(&opener->fh, video_devdata(file));
	file->private_data = &opener->fh;
	v4l2_fh_add(&opener->fh);
	return opener;
}
static __poll_t v4l2_loopback_poll(struct file *file,
						struct poll_table_struct *pts)
{
	struct v4l2_loopback_opener *opener;
	struct v4l2_loopback_device *dev;
	struct v4l2_streamdata *data;
	__poll_t req_events = poll_requested_events(pts);
	__poll_t ret_mask = 0;

	MARK();

	opener = fh_to_opener(file->private_data);
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return (__force __poll_t)-EINVAL;
	}

	data = opener->data;
	if (req_events & EPOLLPRI) {
		if (!v4l2_event_pending(&opener->fh))
			poll_wait(file, &opener->fh.wait, pts);
		if (v4l2_event_pending(&opener->fh)) {
			ret_mask |= EPOLLPRI;
			if (!(req_events & DEFAULT_POLLMASK))
				return ret_mask;
		}
	}

	switch (opener->type) {
	case V4L2L_WRITER:
		ret_mask |= EPOLLOUT | EPOLLWRNORM;
		break;
	case V4L2L_READER:
		if (dev->open_count.counter == 0)
			return POLLERR;

		if (!can_read(opener->data, opener)) {
			if (ret_mask)
				return ret_mask;

			poll_wait(file, &data->read_event, pts);
		}
		if (can_read(data, opener))
			ret_mask |= EPOLLIN | EPOLLRDNORM;
		if (v4l2_event_pending(&opener->fh))
			ret_mask |= EPOLLPRI;
		break;
	default:
		break;
	}

	MARK();
	return ret_mask;
}

/* do not want to limit device opens, it can be as many readers as user want,
 * writers are limited by means of setting writer field
 */
static int v4l2_loopback_open(struct file *file)
{
	struct v4l2_loopback_device *dev = NULL;
	struct v4l2_loopback_opener *opener = NULL;
	struct v4l2_loopback_opener *connected_opener = NULL;
	struct v4l2_streamdata *data = NULL;
	int rc = 0;
	enum v4l2_loopback_opener_type etype;

	MARK();

	dev = v4l2loopback_getdevice(file);

	if (dev == NULL) {
		CAM_ERR(CAM_V4L2, "\ndev value is null");
		return -EINVAL;
	}

	if (dev->state == V4L2L_OPENED) {
		CAM_ERR(CAM_V4L2, "\ndev state is busy dev=%s", dev->vdev->name);
		return -EBUSY;
	}

	/* WRITEONLY should be open operation from proxy */
	if ((file->f_mode & (FMODE_READ | FMODE_WRITE)) == FMODE_WRITE) {
		if (dev->state == V4L2L_READY_FOR_OUTPUT) {
			CAM_INFO(CAM_V4L2, "proxy open at the first time dev=%s",
				dev->vdev->name);
			etype = V4L2L_WRITER;
			dev->state = V4L2L_READY_FOR_CAPTURE;

			opener = create_opener(file, etype);
			if (opener == NULL)
				return -ENOMEM;

			dev->main_opener = opener;
		} else if (dev->state == V4L2L_READY_FOR_CAPTURE) {
		// when capture state, proxy open it can succeed
			CAM_INFO(CAM_V4L2, "proxy open when app open dev=%s",
				dev->vdev->name);
			etype = V4L2L_WRITER;

			opener = create_opener(file, etype);
			if (opener == NULL)
				return -ENOMEM;

			data = kzalloc(sizeof(*data), GFP_KERNEL);
			if (data == NULL)
				return -ENOMEM;

			init_stream_data(data);

			opener->data = data;
		}
	} else {
		/* app open it */
		if (dev->state == V4L2L_READY_FOR_CAPTURE) {
			CAM_INFO(CAM_V4L2, "app open dev=%s", dev->vdev->name);
			mutex_lock(&dev->dev_mutex);
			etype = V4L2L_READER;
			send_v4l2_event(dev->main_opener, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_OPEN_INPUT);
			rc = wait_for_completion_timeout(&dev->open_complete,
					msecs_to_jiffies(OPEN_TIMEOUT));
			if (rc) {
				rc = dev->qcarcam_ctrl_ret;
				if (!dev->qcarcam_ctrl_ret) {
					CAM_INFO(CAM_V4L2, "app open succeed dev=%s",
						dev->vdev->name);
				} else {
					CAM_ERR(CAM_V4L2, "app open fail dev=%s",
						dev->vdev->name);
					mutex_unlock(&dev->dev_mutex);
					return rc;
				}
			} else {
				CAM_ERR(CAM_V4L2, "open fail dev=%s, timeout %d",
					dev->vdev->name, rc);
				rc = -ETIMEDOUT;
				mutex_unlock(&dev->dev_mutex);
				return rc;
			}

			connected_opener = fh_to_opener(dev->open_ret_fh);

			if (connected_opener == NULL) {
				CAM_ERR(CAM_V4L2, "connected opener error");
				mutex_unlock(&dev->dev_mutex);
				return -EINVAL;
			}

			/* create the opener */
			opener = create_opener(file, etype);
			if (opener == NULL) {
				mutex_unlock(&dev->dev_mutex);
				return -ENOMEM;
			}

			atomic_inc(&dev->open_count);

			opener->connected_opener = connected_opener;
			connected_opener->connected_opener = opener;
			opener->data = connected_opener->data;

			CAM_INFO(CAM_V4L2, "capture opener %p, proxy opener %p, data %p",
				opener, connected_opener, data);
			mutex_unlock(&dev->dev_mutex);
		} else {
			CAM_ERR(CAM_V4L2, "invalid operation state %d", dev->state);
			return -EINVAL;
		}
	}

	if (dev->open_count.counter >= dev->max_openers) {
		CAM_WARN(CAM_V4L2, "open counter >= max_openers");
		dev->state = V4L2L_OPENED;
	}

	CAM_DBG(CAM_V4L2, "opened dev:%p", dev);
	MARK();
	return rc;
}

static int v4l2_loopback_close(struct file *file)
{
	struct v4l2_loopback_opener *opener;
	struct v4l2_loopback_device *dev;
	struct v4l2_streamdata *data;
	int is_main = 0;
	int is_writer = 0;
	int rc = 0;

	MARK();
	opener = fh_to_opener(file->private_data);
	dev    = v4l2loopback_getdevice(file);
	if (!dev) {
		CAM_ERR(CAM_V4L2, "dev is null");
		return -EINVAL;
	}

	if (!opener) {
		CAM_ERR(CAM_V4L2, "opener is null");
		return -EINVAL;
	}

	data = opener->data;

	if (opener == dev->main_opener)
		is_main = 1;

	if (opener->type == V4L2L_WRITER)
		is_writer = 1;

	if (is_main) {
		if (dev->state != V4L2L_READY_FOR_OUTPUT) {
			dev->state = V4L2L_READY_FOR_OUTPUT;
			atomic_set(&dev->open_count, 0);
			dev->main_opener = NULL;
			CAM_WARN(CAM_V4L2, "v4l2 open_count is 0");
		} else
			CAM_WARN(CAM_V4L2, "invalid proxy close, state %d", dev->state);
	} else if (is_writer) {
		// todo: refine the close flow
	} else {
		/* notify ais_v4l2_proxy to close the input */
		mutex_lock(&dev->dev_mutex);
		CAM_WARN(CAM_V4L2, "v4l2 open_count is %d when close", dev->open_count.counter);
		if (dev->state >= V4L2L_READY_FOR_CAPTURE) {
			if (opener->connected_opener) {

				send_v4l2_event(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
					AIS_V4L2_CLOSE_INPUT);
				rc = wait_for_completion_timeout(&dev->close_complete,
						msecs_to_jiffies(CLOSE_TIMEOUT));
				if (rc) {
					if (!dev->qcarcam_ctrl_ret) {
						CAM_INFO(CAM_V4L2, "app close succeed dev=%s",
							dev->vdev->name);
					} else {
						CAM_ERR(CAM_V4L2, "app close fail dev=%s",
							dev->vdev->name);
					}
				} else {
					CAM_ERR(CAM_V4L2, "close fail dev=%s, timeout %d",
					dev->vdev->name, rc);
				}

				atomic_dec(&dev->open_count);
				if (dev->open_count.counter < max_openers)
					dev->state = V4L2L_READY_FOR_CAPTURE;
			}
			if (opener->data) {
				free_stream_data(opener->data);
				opener->data = NULL;
			}
		} else {
			CAM_WARN(CAM_V4L2, "invalid close state %d", dev->state);
		}
		mutex_unlock(&dev->dev_mutex);
	}

	CAM_WARN(CAM_V4L2, "v4l2 del v4l2 fh open_count is %d when close",
		dev->open_count.counter);

	v4l2_fh_del(file->private_data);
	v4l2_fh_exit(file->private_data);

	kfree(opener);
	opener = NULL;

	MARK();

	return 0;
}

/* init functions */
/* frees buffers, if already allocated */
static int free_buffers(struct v4l2_streamdata *data)
{
	int i;

	MARK();
	if (data == NULL) {
		CAM_ERR(CAM_V4L2, "data value is null");
		return -EINVAL;
	}

	CAM_INFO(CAM_V4L2, "freeing buffer %p", data);

	mutex_lock(&data->buf_mutex);

	for (i = 0; i < data->buffers_number; ++i) {
		if (data->dmabufs[i]) {
			data->buffers[i].kvaddr = 0;
			dma_buf_put((struct dma_buf *)data->dmabufs[i]);
			data->dmabufs[i] = NULL;
		}
	}

	mutex_unlock(&data->buf_mutex);

	return 0;
}

/* allocates buffers, if buffer_size is set */
static int allocate_dma_buffers(struct v4l2_loopback_opener *opener,
	struct v4l2_streamdata *data)
{
	int rc = 0;

	send_v4l2_event_ex1(opener->connected_opener, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_ALLOC_BUFS, data->used_buffers, data->use_buf_width);
	/* wait for the signal */
	rc = wait_for_completion_timeout(&data->allocbufs_complete,
		msecs_to_jiffies(ALLOCBUFS_TIMEOUT));
	if (rc) {
		rc = 0;
	} else {
		CAM_ERR(CAM_V4L2, "allocate fail, timeout %d", rc);
		rc = -ETIMEDOUT;
		return rc;
	}

	if (!data->allocbufs_ret)
		init_buffers(data);

	return data->allocbufs_ret;
}


/* init inner buffers, they are capture mode and flags are set as
 * for capture mod buffers
 */
static void init_buffers(struct v4l2_streamdata *data)
{
	int i;
	int buffer_size;
	int bytesused;
	struct timespec64 ts;

	MARK();
	buffer_size = data->buffer_size;
	bytesused = data->pix_format.sizeimage;

	for (i = 0; i < data->buffers_number; ++i) {
		struct v4l2_buffer *b = &data->buffers[i].buffer;

		b->index             = i;
		b->bytesused         = bytesused;
		b->length            = buffer_size;
		b->field             = V4L2_FIELD_NONE;
		b->flags             = 0;
		b->m.offset          = i * buffer_size;
		b->memory            = V4L2_MEMORY_MMAP;
		b->sequence          = 0;
		b->timestamp.tv_sec  = 0;
		b->timestamp.tv_usec = 0;
		b->type              = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		/* all the buffers are acquired by the proxy when init */
		data->buffers[i].state = V4L2L_BUF_USER_ACQUIRED;
		data->buffers[i].ext.batch_num = 0;

		cam_common_util_get_curr_timestamp(&ts);
		b->timestamp.tv_sec = ts.tv_sec;
		b->timestamp.tv_usec = ts.tv_nsec / NSEC_PER_USEC;
	}

	MARK();
}

/* init default capture parameters, only fps may be changed in future */
static void init_capture_param(struct v4l2_captureparm *capture_param)
{
	MARK();
	capture_param->capability               = 0;
	capture_param->capturemode              = 0;
	capture_param->extendedmode             = 0;
	capture_param->readbuffers              = max_buffers;
	capture_param->timeperframe.numerator   = 1;
	capture_param->timeperframe.denominator = 30;
}

int init_stream_data(struct v4l2_streamdata *data)
{
	data->buffers_number = MAX_AIS_BUFFERS_NUM;
	data->used_buffers = max_buffers;
	//dev->max_openers = max_openers;

	spin_lock_init(&data->lock);

	// when alloc buffer, all the buffers are acquired by user
	INIT_LIST_HEAD(&data->outbufs_list);
	INIT_LIST_HEAD(&data->capbufs_list);

	memset(data->dmabufs, 0, sizeof(data->dmabufs));

	init_capture_param(&data->capture_param);

	data->buffer_size = 0;
	data->is_streaming = 0;

	/* Set initial format */
	data->pix_format.width = 0; /* V4L2LOOPBACK_SIZE_DEFAULT_WIDTH; */
	data->pix_format.height = 0; /* V4L2LOOPBACK_SIZE_DEFAULT_HEIGHT; */
	data->pix_format.pixelformat = formats[0].fourcc;
	data->pix_format.colorspace =
		V4L2_COLORSPACE_SRGB; /* do we need to set this ? */
	data->pix_format.field = V4L2_FIELD_NONE;

	data->buffer_size = PAGE_ALIGN(data->pix_format.sizeimage);

	data->io_mode = V4L2L_IO_MODE_DMABUF;

	CAM_DBG(CAM_V4L2, "buffer_size = %ld (=%d)",
		data->buffer_size, data->pix_format.sizeimage);

	init_waitqueue_head(&data->read_event);
	init_completion(&data->gparam_complete);
	init_completion(&data->sparam_complete);
	init_completion(&data->ctrl_complete);
	init_completion(&data->allocbufs_complete);

	mutex_init(&data->buf_mutex);
	mutex_init(&data->capbufs_mutex);
	mutex_init(&data->outbufs_mutex);

	return 0;

}

void free_stream_data(struct v4l2_streamdata *data)
{
	free_buffers(data);
	mutex_destroy(&data->buf_mutex);
	mutex_destroy(&data->capbufs_mutex);
	mutex_destroy(&data->outbufs_mutex);
	kfree(data);
}

/* fills and register video device */
static void init_vdev(struct video_device *vdev, int nr)
{
	MARK();
	vidioc_fill_name(vdev->name, sizeof(vdev->name), nr);

#ifdef V4L2LOOPBACK_WITH_STD
	vdev->tvnorms      = V4L2_STD_ALL;
#endif /* V4L2LOOPBACK_WITH_STD */

	vdev->vfl_type     = VFL_TYPE_VIDEO;
	vdev->fops         = &v4l2_loopback_fops;
	vdev->ioctl_ops    = &v4l2_loopback_ioctl_ops;
	vdev->release      = &video_device_release;
	vdev->minor        = -1;

	/* >=linux-4.7.0 */
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_OUTPUT |
			    V4L2_CAP_READWRITE | V4L2_CAP_STREAMING;
#ifdef V4L2_CAP_VIDEO_M2M
	vdev->device_caps |= V4L2_CAP_VIDEO_M2M;
#endif

	if (debug > 1)
		vdev->dev_debug =
			V4L2_DEV_DEBUG_IOCTL | V4L2_DEV_DEBUG_IOCTL_ARG;

	/* since kernel-3.7, there is a new field 'vfl_dir' that has to be
	 * set to VFL_DIR_M2M for bidrectional devices
	 */
	vdev->vfl_dir = VFL_DIR_M2M;

	MARK();
}



/* init loopback main structure */
static int v4l2_loopback_init(struct v4l2_loopback_device *dev, int nr)
{
	int ret;
	struct v4l2_ctrl_handler *hdl = &dev->ctrl_handler;
	struct v4l2_ctrl *ctrl;

	snprintf(dev->v4l2_dev.name, sizeof(dev->v4l2_dev.name),
				"v4l2loopback-video%03d", video_nr[nr]);

	CAM_INFO(CAM_V4L2, "v4l2_dev.name %s", dev->v4l2_dev.name);

	card_label[nr] = dev->v4l2_dev.name;

	ret = v4l2_device_register(NULL, &dev->v4l2_dev);
	if (ret)
		return ret;

	MARK();

	mutex_init(&dev->dev_mutex);

	dev->vdev = video_device_alloc();
	if (dev->vdev == NULL) {
		ret = -ENOMEM;
		goto error;
	}

	video_set_drvdata(dev->vdev,
		kzalloc(sizeof(struct v4l2loopback_private), GFP_KERNEL));
	if (video_get_drvdata(dev->vdev) == NULL) {
		ret = -ENOMEM;
		goto error;
	}
	((struct v4l2loopback_private *)video_get_drvdata
		(dev->vdev))->devicenr = nr;

	init_vdev(dev->vdev, nr);
	dev->vdev->v4l2_dev = &dev->v4l2_dev;

	atomic_set(&dev->open_count, 0);
	dev->state = V4L2L_READY_FOR_OUTPUT;
	dev->announce_all_caps = (!exclusive_caps[nr]);
	dev->max_openers = max_openers;
	dev->stream_status = 0;

	ret = v4l2_ctrl_handler_init(hdl, 1);
	if (ret)
		goto error;

	ctrl = v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_buffernum, NULL);
	if (ctrl)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;
	else {
		CAM_ERR(CAM_V4L2, "fail to new custom v4l2loopback_ctrl_buffernum");
		goto error;
	}

	if (hdl->error) {
		CAM_ERR(CAM_V4L2, "ctrl handler error");
		ret = hdl->error;
		goto error;
	}
	dev->v4l2_dev.ctrl_handler = hdl;

	init_completion(&dev->open_complete);
	init_completion(&dev->close_complete);

	MARK();
	return 0;

error:
	v4l2_ctrl_handler_free(&dev->ctrl_handler);
	v4l2_device_unregister(&dev->v4l2_dev);
	kfree(dev->vdev);
	return ret;

};

/* LINUX KERNEL */
static const struct v4l2_file_operations v4l2_loopback_fops = {
	.owner   = THIS_MODULE,
	.open    = v4l2_loopback_open,
	.release = v4l2_loopback_close,
	.poll    = v4l2_loopback_poll,
	.unlocked_ioctl   = video_ioctl2,
};

static const struct v4l2_ioctl_ops v4l2_loopback_ioctl_ops = {
	.vidioc_querycap         = &vidioc_querycap,
	.vidioc_enum_framesizes  = &vidioc_enum_framesizes,
	//.vidioc_enum_frameintervals = &vidioc_enum_frameintervals,

#ifdef V4L2LOOPBACK_WITH_OUTPUT
	.vidioc_enum_output       = &vidioc_enum_output,
	.vidioc_g_output          = &vidioc_g_output,
	.vidioc_s_output          = &vidioc_s_output,
#endif

	//.vidioc_enum_input       = &vidioc_enum_input,
	//.vidioc_g_input          = &vidioc_g_input,
	//.vidioc_s_input          = &vidioc_s_input,

	.vidioc_enum_fmt_vid_cap = &vidioc_enum_fmt_cap,
	.vidioc_g_fmt_vid_cap    = &vidioc_g_fmt_cap,
	.vidioc_s_fmt_vid_cap    = &vidioc_s_fmt_cap,
	.vidioc_try_fmt_vid_cap  = &vidioc_try_fmt_cap,

	.vidioc_enum_fmt_vid_out = &vidioc_enum_fmt_out,
	.vidioc_s_fmt_vid_out    = &vidioc_s_fmt_out,
	.vidioc_g_fmt_vid_out    = &vidioc_g_fmt_out,
	.vidioc_try_fmt_vid_out  = &vidioc_try_fmt_out,

#ifdef V4L2L_OVERLAY
	//.vidioc_s_fmt_vid_overlay = &vidioc_s_fmt_overlay,
	//.vidioc_g_fmt_vid_overlay = &vidioc_g_fmt_overlay,
#endif

#ifdef V4L2LOOPBACK_WITH_STD
	//.vidioc_s_std            = &vidioc_s_std,
	//.vidioc_g_std            = &vidioc_g_std,
	//.vidioc_querystd         = &vidioc_querystd,
#endif /* V4L2LOOPBACK_WITH_STD */

	.vidioc_g_parm           = &vidioc_g_parm,
	.vidioc_s_parm           = &vidioc_s_parm,

	.vidioc_reqbufs          = &vidioc_reqbufs,
	.vidioc_querybuf         = &vidioc_querybuf,
	.vidioc_expbuf           = &vidioc_expbuf,
	.vidioc_qbuf             = &vidioc_qbuf,
	.vidioc_dqbuf            = &vidioc_dqbuf,

	.vidioc_streamon         = &vidioc_streamon,
	.vidioc_streamoff        = &vidioc_streamoff,

#ifdef CONFIG_VIDEO_V4L1_COMPAT
	.vidiocgmbuf             = &vidiocgmbuf,
#endif

	.vidioc_subscribe_event = cam_subscribe_event,
	.vidioc_unsubscribe_event = cam_unsubscribe_event,
	.vidioc_default = ais_v4l2loopback_dev_ioctl
};

static void zero_devices(void)
{
	int i;

	MARK();
	for (i = 0; i < MAX_DEVICES; i++)
		devs[i] = NULL;
}

static void free_devices(void)
{
	int i;

	MARK();
	for (i = 0; i < devices; i++) {
		if (devs[i] != NULL) {

			v4l2loopback_remove_sysfs(devs[i]->vdev);
			kfree(video_get_drvdata(devs[i]->vdev));
			video_unregister_device(devs[i]->vdev);
			v4l2_device_unregister(&devs[i]->v4l2_dev);
			v4l2_ctrl_handler_free(&devs[i]->ctrl_handler);
			mutex_destroy(&devs[i]->dev_mutex);
			kfree(devs[i]);
			devs[i] = NULL;
		}
	}
}

int v4l2loopback_init_module(void)
{
	int ret;
	int i;
	int video_idx = 51;

	MARK();
	zero_devices();

	devices = 16;
	for (i = 0; i < devices; ++i)
		video_nr[i] = video_idx + i;

	max_buffers = MAX_AIS_BUFFERS_NUM;

	if (devices < 0) {
		devices = 1;

		/* try guessing the devices from the "video_nr" parameter */
		for (i = MAX_DEVICES - 1; i >= 0; i--) {
			if (video_nr[i] >= 0) {
				devices = i + 1;
				break;
			}
		}
	}

	if (devices > MAX_DEVICES) {
		devices = MAX_DEVICES;
		CAM_DBG(CAM_V4L2,
				"v4l2loopback: number of devices is limited to: %d"
				, MAX_DEVICES);
	}

	if (max_buffers > MAX_BUFFERS) {
		max_buffers = MAX_BUFFERS;
		CAM_DBG(CAM_V4L2,
				"v4l2loopback: number of buffers is limited to: %d"
				, MAX_BUFFERS);
	}

	if (max_openers < 0) {
		CAM_DBG(CAM_V4L2,
				"v4l2loopback: allowing %d openers rather than %d"
				, 2, max_openers);
		max_openers = 2;
	}

	if (max_width < 1) {
		max_width = V4L2LOOPBACK_SIZE_MAX_WIDTH;
		CAM_DBG(CAM_V4L2, "v4l2loopback: using max_width %d", max_width);
	}
	if (max_height < 1) {
		max_height = V4L2LOOPBACK_SIZE_MAX_HEIGHT;
		CAM_DBG(CAM_V4L2, "v4l2loopback: using max_height %d", max_height);
	}

	/* kfree on module release */
	for (i = 0; i < devices; i++) {
		CAM_DBG(CAM_V4L2, "creating v4l2loopback-device #%d", i);
		devs[i] = kzalloc(sizeof(*devs[i]), GFP_KERNEL);
		if (devs[i] == NULL) {
			free_devices();
			return -ENOMEM;
		}
		ret = v4l2_loopback_init(devs[i], i);
		if (ret < 0) {
			free_devices();
			return ret;
		}

		/* register the device -> it creates /dev/video* */
		if (video_register_device(devs[i]->vdev,
					VFL_TYPE_VIDEO, video_nr[i]) < 0) {
			video_device_release(devs[i]->vdev);
			CAM_ERR(CAM_V4L2,
			"v4l2loopback: failed video_register_device()");
			free_devices();
			return -EFAULT;
		}
		v4l2loopback_create_sysfs(devs[i]->vdev);
	}

	CAM_INFO(CAM_V4L2, "v4l2loopback driver version %d.%d.%d loaded",
			(V4L2LOOPBACK_VERSION_CODE >> 16) & 0xff,
			(V4L2LOOPBACK_VERSION_CODE >>  8) & 0xff,
			(V4L2LOOPBACK_VERSION_CODE) & 0xff);

	return 0;
}

void v4l2loopback_cleanup_module(void)
{
	MARK();
	/* unregister the device -> it deletes /dev/video* */
	free_devices();
	CAM_INFO(CAM_V4L2, "module removed");
}

MODULE_DESCRIPTION("AIS V4L2LOOPBACK driver");
MODULE_LICENSE("GPL v2");

