/* -*- c-file-style: "linux" -*-*/
/*
 * v4l2loopback.c  --  video4linux2 loopback driver
 *
 * Copyright (C) 2005-2009 Vasily Levin (vasaka@gmail.com)
 * Copyright (C) 2010-2019 IOhannes m zmoelnig (zmoelnig@iem.at)
 * Copyright (C) 2011 Stefan Diewald (stefan.diewald@mytum.de)
 * Copyright (C) 2012 Anton Novikov (random.plant@gmail.com)
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved
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
#include <linux/time.h>
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
# define MAX_DEVICES 8
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
#define CID_KEEP_FORMAT        (V4L2LOOPBACK_CID_BASE + 0)
#define CID_SUSTAIN_FRAMERATE  (V4L2LOOPBACK_CID_BASE + 1)
#define CID_TIMEOUT            (V4L2LOOPBACK_CID_BASE + 2)
#define CID_TIMEOUT_IMAGE_IO   (V4L2LOOPBACK_CID_BASE + 3)
#define CID_CROP_DATASIZE      (V4L2_CTRL_CLASS_USER + 0x1000)

static int v4l2loopback_s_ctrl(struct v4l2_ctrl *ctrl);
static int v4l2loopback_g_ctrl(struct v4l2_ctrl *ctrl);

static int v4l2loopback_datasize_g_ctrl(struct v4l2_ctrl *ctrl);
static int v4l2loopback_datasize_s_ctrl(struct v4l2_ctrl *ctrl);
static int v4l2loopback_datasize_try_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops v4l2loopback_ctrl_ops = {
	.s_ctrl = v4l2loopback_s_ctrl,
	.try_ctrl = v4l2loopback_datasize_try_ctrl,
	.g_volatile_ctrl = v4l2loopback_g_ctrl,
};

static const struct v4l2_ctrl_ops v4l2loopback_datasize_ctrl_ops = {
	.s_ctrl = v4l2loopback_datasize_s_ctrl,
	.try_ctrl = v4l2loopback_datasize_try_ctrl,
	.g_volatile_ctrl = v4l2loopback_datasize_g_ctrl
};

static const struct v4l2_ctrl_config v4l2loopback_ctrl_keepformat = {
	.ops = &v4l2loopback_ctrl_ops,
	.id = CID_KEEP_FORMAT,
	.name = "keep_format",
	.type = V4L2_CTRL_TYPE_BOOLEAN,
	.min = 0,
	.max = 1,
	.step = 1,
	.def = 0,
};
static const struct v4l2_ctrl_config v4l2loopback_ctrl_sustainframerate = {
	.ops = &v4l2loopback_ctrl_ops,
	.id = CID_SUSTAIN_FRAMERATE,
	.name = "sustain_framerate",
	.type = V4L2_CTRL_TYPE_BOOLEAN,
	.min = 0,
	.max = 1,
	.step = 1,
	.def = 0,
};
static const struct v4l2_ctrl_config v4l2loopback_ctrl_timeout = {
	.ops = &v4l2loopback_ctrl_ops,
	.id = CID_TIMEOUT,
	.name = "timeout",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.min = 0,
	.max = MAX_TIMEOUT,
	.step = 1,
	.def = 0,
};
static const struct v4l2_ctrl_config v4l2loopback_ctrl_timeoutimageio = {
	.ops = &v4l2loopback_ctrl_ops,
	.id = CID_TIMEOUT_IMAGE_IO,
	.name = "timeout_image_io",
	.type = V4L2_CTRL_TYPE_BOOLEAN,
	.min = 0,
	.max = 1,
	.step = 1,
	.def = 0,
};
static const struct v4l2_ctrl_config v4l2loopback_ctrl_datasize = {
	.ops = &v4l2loopback_datasize_ctrl_ops,
	.id = CID_CROP_DATASIZE,
	.name = "crop_datasize",
	.type = V4L2_CTRL_TYPE_INTEGER,
	.min = V4L2LOOPBACK_SIZE_MIN_WIDTH * V4L2LOOPBACK_SIZE_MIN_HEIGHT,
	.max = V4L2LOOPBACK_SIZE_MAX_WIDTH * V4L2LOOPBACK_SIZE_MAX_HEIGHT,
	.step = 1,
	.def = V4L2LOOPBACK_SIZE_DEFAULT_WIDTH *
			V4L2LOOPBACK_SIZE_DEFAULT_HEIGHT,
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
	/* pixel and stream format */
	struct v4l2_pix_format pix_format;
	struct v4l2_captureparm capture_param;

	/* param for vendor extension&other qcarcam API*/
	u8 qcarcam_code;
	u8 qcarcam_param[MAX_AIS_V4L2_PAYLOAD_SIZE];
	long qcarcam_param_size;
	struct completion sparam_complete;
	struct completion gparam_complete;
	int qcarcam_sparam_ret;

	/* qcarcam control ret*/
	int qcarcam_ctrl_ret;
	struct completion ctrl_complete;

	struct v4l2_crop frame_crop;
	unsigned long frame_jiffies;

	/* ctrls */
	int keep_format; /* CID_KEEP_FORMAT; stay ready_for_capture even when
			  * all openers close() the device
			  */
	int sustain_framerate; /* CID_SUSTAIN_FRAMERATE; duplicate
				* frames to maintain (close to)
				* nominal framerate
				*/

	enum V4L2_LOOPBACK_IO_MODE io_mode;

	/* buffers stuff */
	u8 *image;        /* pointer to actual buffers data,
			   * this is MMAP mode
			   */
	unsigned long int imagesize;  /* size of buffers data */

	struct dma_buf *dmabufs[MAX_BUFFERS];
	struct completion allocbufs_complete;
	int allocbufs_ret;
	int buffers_number;  /* should not be big, 4 is a good choice */
	struct v4l2l_buffer buffers[MAX_BUFFERS]; /* inner driver buffers */
	int used_buffers; /* number of the actually used buffers */
	int max_openers;  /* how many times can this device be opened */

	int write_position; /* number of last written frame + 1 */
	struct list_head outbufs_list; /* buffers in output DQBUF order */
	struct list_head capbufs_list; /* buffers in capture DQBUF order */
	struct mutex outbufs_mutex;
	struct mutex capbufs_mutex;

	int bufpos2index[MAX_BUFFERS]; /* mapping of (read/write_position
					* % used_buffers)
					* to inner buffer index
					*/
	long buffer_size;

	/* sustain_framerate stuff */
	struct timer_list sustain_timer;
	unsigned int reread_count;

	/* timeout stuff */
	unsigned long timeout_jiffies; /* CID_TIMEOUT; 0 means disabled */
	int timeout_image_io; /* CID_TIMEOUT_IMAGE_IO; next opener will
			       * read/write to timeout_image
			       */
	u8 *timeout_image; /* copy of it will be captured when timeout passes */
	struct v4l2l_buffer timeout_image_buffer;
	struct timer_list timeout_timer;
	int timeout_happened;

	/* sync stuff */
	atomic_t open_count;


	enum V4L2_LOOPBACK_STATE state;

	int announce_all_caps;/* set to false, if device caps (OUTPUT/CAPTURE)
			       * should only be announced if the resp. "ready"
			       * flag is set; default=TRUE
			       */



	wait_queue_head_t read_event;
	spinlock_t lock;
	struct mutex buf_mutex;
};

/* types of opener shows what opener wants to do with loopback */
enum v4l2_loopback_opener_type {
	V4L2L_UNNEGOTIATED = 0,
	V4L2L_READER = 1,
	V4L2L_WRITER = 2,
};

/* struct keeping state and type of opener */
struct v4l2_loopback_opener {
	enum v4l2_loopback_opener_type type;
	int vidioc_enum_frameintervals_calls;
	int read_position; /* number of last processed frame + 1 or
			    * write_position - 1 if reader went out of sync
			    */
	unsigned int reread_count;
	struct v4l2_buffer *buffers;
	int buffers_number;  /* should not be big, 4 is a good choice */
	int timeout_image_io;
	 struct v4l2_fh fh;
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

static void send_v4l2_event(struct video_device *vdev, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;

	v4l2_event_queue(vdev, &event);
	pr_debug("send v4l2 event for ais_v4l2loopback :%d\n",
		cmd);
}

static void send_v4l2_event_ex(struct video_device *vdev, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd, u8 data0)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;
	event.u.data[0] = data0;

	v4l2_event_queue(vdev, &event);
	pr_debug("send v4l2 event for ais_v4l2loopback :%d\n",
		cmd);
}

static void send_v4l2_event_ex2(struct video_device *vdev, unsigned int type,
	enum AIS_V4L2_NOTIFY_CMD cmd, u8 data0, u8 size, __u64 payload)
{
	struct v4l2_event event;

	event.id = cmd;
	event.type = type;
	event.u.data[0] = data0;
	event.u.data[1] = size;

	if (size == 0) {
		v4l2_event_queue(vdev, &event);
		pr_debug("send v4l2 event for ais_v4l2loopback :%d\n",
			cmd);
	} else if (size <= MAX_AIS_V4L2_PARAM_EVNET_SIZE) {
		if (copy_from_user(&event.u.data[2],
					u64_to_user_ptr(payload),
					size)) {
			pr_err("fail to copy from user when send v4l2");
		} else {
			v4l2_event_queue(vdev, &event);
			pr_debug("send v4l2 event for ais_v4l2loopback :%d\n",
				cmd);
		}
	} else {
		v4l2_event_queue(vdev, &event);
	}
}

static const struct v4l2l_format *format_by_fourcc(int fourcc)
{
	unsigned int i;

	for (i = 0; i < FORMATS; i++) {
		if (formats[i].fourcc == fourcc)
			return formats + i;
	}

	pr_err("unsupported format '%c%c%c%c'\n",
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
		f->bytesperline = width; /* Y plane */
		if (f->sizeimage == 0)
			f->sizeimage = (width * height * fmt->depth) >> 3;
	} else if (fmt->flags & FORMAT_FLAGS_COMPRESSED) {
		/* doesn't make sense for compressed formats */
		f->bytesperline = 0;
		if (f->sizeimage == 0)
			f->sizeimage = (width * height * fmt->depth) >> 3;
	} else {
		f->bytesperline = (width * fmt->depth) >> 3;
		if (f->sizeimage == 0)
			f->sizeimage = height * f->bytesperline;
	}
}

static int set_timeperframe(struct v4l2_loopback_device *dev,
		struct v4l2_fract *tpf)
{
	if ((tpf->denominator < 1) || (tpf->numerator < 1))
		return -EINVAL;
	dev->capture_param.timeperframe = *tpf;
	dev->frame_jiffies = max(1UL,
		msecs_to_jiffies(1000) * tpf->numerator / tpf->denominator);
	return 0;
}

static struct v4l2_loopback_device *v4l2loopback_cd2dev(struct device *cd);

/* device attributes */
/* available via sysfs: /sys/devices/virtual/video4linux/video* */

static ssize_t attr_show_format(struct device *cd,
		struct device_attribute *attr, char *buf)
{
	/* gets the current format as "FOURCC:WxH@f/s",
	 * e.g. "YUYV:320x240@1000/30"
	 */
	struct v4l2_loopback_device *dev = v4l2loopback_cd2dev(cd);
	const struct v4l2_fract *tpf;
	char buf4cc[5], buf_fps[32];

	if (!dev || !(dev->state & V4L2L_READY_FOR_CAPTURE))
		return 0;
	tpf = &dev->capture_param.timeperframe;

	fourcc2str(dev->pix_format.pixelformat, buf4cc);
	buf4cc[4] = 0;
	if (tpf->numerator == 1)
		snprintf(buf_fps, sizeof(buf_fps), "%d", tpf->denominator);
	else
		snprintf(buf_fps, sizeof(buf_fps), "%d/%d",
				tpf->denominator, tpf->numerator);
	return snprintf(buf, sizeof(buf_fps), "%4s:%dx%d@%s\n",
		buf4cc, dev->pix_format.width, dev->pix_format.height, buf_fps);
}

static ssize_t attr_store_format(struct device *cd,
		struct device_attribute *attr, const char *buf, size_t len)
{
	struct v4l2_loopback_device *dev = v4l2loopback_cd2dev(cd);
	int fps_num = 0, fps_den = 1;

	if (!dev) {
		pr_err("dev is null");
		return 0;
	}

	/* only fps changing is supported */
	if (sscanf(buf, "@%d/%d", &fps_num, &fps_den) > 0) {
		struct v4l2_fract f = {
			.numerator   = fps_den,
			.denominator = fps_num
		};
		int err = 0;

		err = set_timeperframe(dev, &f);
		if (err < 0)
			return err;
		return len;
	}
	return -EINVAL;
}

static DEVICE_ATTR(format, 0644, attr_show_format,
		attr_store_format);

static ssize_t attr_show_buffers(struct device *cd,
		struct device_attribute *attr, char *buf)
{
	struct v4l2_loopback_device *dev = v4l2loopback_cd2dev(cd);

	if (dev == NULL) {
		pr_err("\ndev value is null\n");
		return -EINVAL;
	}

	return snprintf(buf, sizeof(dev->used_buffers), "%d\n",
			dev->used_buffers);
}

static DEVICE_ATTR(buffers, 0444, attr_show_buffers, NULL);

static ssize_t attr_show_maxopeners(struct device *cd,
		struct device_attribute *attr, char *buf)
{
	struct v4l2_loopback_device *dev = v4l2loopback_cd2dev(cd);

	if (dev == NULL) {
		pr_err("\ndev value is null\n");
		return -EINVAL;
	}

	return snprintf(buf, sizeof(dev->max_openers), "%d\n",
			dev->max_openers);
}

static ssize_t attr_store_maxopeners(struct device *cd,
		struct device_attribute *attr, const char *buf, size_t len)
{
	struct v4l2_loopback_device *dev = NULL;
	unsigned long curr = 0;

	if (kstrtoul(buf, 0, &curr))
		return -EINVAL;

	dev = v4l2loopback_cd2dev(cd);

	if (dev == NULL) {
		pr_err("\ndev value is null\n");
		return -EINVAL;
	}

	if (dev->max_openers == curr)
		return len;

	if (dev->open_count.counter > curr) {
		/* request to limit to less openers as are
		 * currently attached to us
		 */
		return -EINVAL;
	}

	dev->max_openers = (int)curr;

	return len;
}


static DEVICE_ATTR(max_openers, 0644, attr_show_maxopeners,
		attr_store_maxopeners);

static void v4l2loopback_remove_sysfs(struct video_device *vdev)
{
#define V4L2_SYSFS_DESTROY(x) device_remove_file(&vdev->dev, &dev_attr_##x)

	if (vdev) {
		do {
			V4L2_SYSFS_DESTROY(format);
			V4L2_SYSFS_DESTROY(buffers);
			V4L2_SYSFS_DESTROY(max_openers);
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
		V4L2_SYSFS_CREATE(format);
		V4L2_SYSFS_CREATE(buffers);
		V4L2_SYSFS_CREATE(max_openers);
		/* ... */
	} while (0);

	if (res >= 0)
		return;
	dev_err(&vdev->dev, "%s error: %d\n", __func__, res);
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
		pr_err("v4l2-loopback: illegal device %d\n", nr);
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
		pr_err("v4l2-loopback: illegal device %d\n", nr);
		return NULL;
	}
	return devs[nr];
}

/* forward declarations */
static void init_buffers(struct v4l2_loopback_device *dev);
static int allocate_buffers(struct v4l2_loopback_device *dev);
static int allocate_dma_buffers(struct v4l2_loopback_device *dev);

static int free_buffers(struct v4l2_loopback_device *dev);
static void try_free_buffers(struct v4l2_loopback_device *dev);
static int allocate_timeout_image(struct v4l2_loopback_device *dev);
static void check_timers(struct v4l2_loopback_device *dev);
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
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	devnr = ((struct v4l2loopback_private *)
			video_get_drvdata(dev->vdev))->devicenr;

	strlcpy(cap->driver, "v4l2 loopback", sizeof(cap->driver));
	vidioc_fill_name(cap->card, sizeof(cap->card), devnr);
	snprintf(cap->bus_info, sizeof(cap->bus_info),
			"platform:v4l2loopback-%03d", devnr);

	/* since 3.1.0, the v4l2-core system is supposed to set the version */
	cap->version = V4L2LOOPBACK_VERSION_CODE;
	cap->capabilities =
		V4L2_CAP_DEVICE_CAPS |
		V4L2_CAP_STREAMING | V4L2_CAP_READWRITE;

#ifdef V4L2_CAP_VIDEO_M2M
	cap->capabilities |= V4L2_CAP_VIDEO_M2M;
#endif /* V4L2_CAP_VIDEO_M2M */
	if (dev->announce_all_caps) {
		cap->capabilities |= V4L2_CAP_VIDEO_CAPTURE |
			V4L2_CAP_VIDEO_OUTPUT;
	} else {

		if (dev->state & V4L2L_READY_FOR_CAPTURE)
			cap->capabilities |= V4L2_CAP_VIDEO_CAPTURE;
		if (dev->state & V4L2L_READY_FOR_OUTPUT)
			cap->capabilities |= V4L2_CAP_VIDEO_OUTPUT;
	}
	cap->device_caps = (cap->capabilities & ~V4L2_CAP_DEVICE_CAPS);
	cap->device_caps = cap->capabilities;
	cap->capabilities |= V4L2_CAP_DEVICE_CAPS;

	memset(cap->reserved, 0, sizeof(cap->reserved));

	return 0;
}

static int vidioc_enum_framesizes(struct file *file, void *fh,
		struct v4l2_frmsizeenum *argp)
{
	struct v4l2_loopback_device *dev;

	/* LATER: what does the index really  mean?
	 * if it's about enumerating formats, we can safely ignore it
	 * (CHECK)
	 */

	/* there can be only one... */
	if (argp->index)
		return -EINVAL;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		/* format has already been negotiated
		 * cannot change during runtime
		 */
		argp->type = V4L2_FRMSIZE_TYPE_DISCRETE;

		argp->discrete.width = dev->pix_format.width;
		argp->discrete.height = dev->pix_format.height;
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

/* returns frameinterval (fps) for the set resolution
 * called on VIDIOC_ENUM_FRAMEINTERVALS
 */
static int vidioc_enum_frameintervals(struct file *file, void *fh,
		struct v4l2_frmivalenum *argp)
{
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);
	struct v4l2_loopback_opener *opener = fh_to_opener(fh);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		if (opener->vidioc_enum_frameintervals_calls > 0)
			return -EINVAL;
		if (argp->width == dev->pix_format.width &&
				argp->height == dev->pix_format.height) {
			argp->type = V4L2_FRMIVAL_TYPE_DISCRETE;
			argp->discrete = dev->capture_param.timeperframe;
			opener->vidioc_enum_frameintervals_calls++;
			return 0;
		}
		return -EINVAL;
	}
	return 0;
}

/* ------------------ CAPTURE ----------------------- */

/* returns device formats
 * called on VIDIOC_ENUM_FMT, with v4l2_buf_type
 * set to V4L2_BUF_TYPE_VIDEO_CAPTURE
 */
static int vidioc_enum_fmt_cap(struct file *file, void *fh,
		struct v4l2_fmtdesc *f)
{
	struct v4l2_loopback_device *dev;

	MARK();

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (f->index)
		return -EINVAL;
	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		const __u32 format = dev->pix_format.pixelformat;

		snprintf(f->description, sizeof(f->description),
				"[%c%c%c%c]",
				(format >>  0) & 0xFF,
				(format >>  8) & 0xFF,
				(format >> 16) & 0xFF,
				(format >> 24) & 0xFF);

		f->pixelformat = dev->pix_format.pixelformat;
	} else {
		return -EINVAL;
	}
	f->flags = 0;
	MARK();
	return 0;
}

/* returns current video format format fmt
 * called on VIDIOC_G_FMT, with v4l2_buf_type
 * set to V4L2_BUF_TYPE_VIDEO_CAPTURE
 */
static int vidioc_g_fmt_cap(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (!(dev->state & V4L2L_READY_FOR_CAPTURE))
		return -EINVAL;

	fmt->fmt.pix = dev->pix_format;
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
	char buf[5];

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (!(dev->state & V4L2L_READY_FOR_CAPTURE)) {
		pr_err("setting fmt_cap not possible yet\n");
		return -EBUSY;
	}

	if (fmt->fmt.pix.pixelformat != dev->pix_format.pixelformat) {
		pr_err("can't support set format now\n");
		return -EINVAL;
	}

	fmt->fmt.pix = dev->pix_format;

	buf[4] = 0;
	pr_debug("capFOURCC=%s\n", fourcc2str(dev->pix_format.pixelformat,
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

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (dev->state & V4L2L_READY_FOR_CAPTURE) {
		const __u32 format = dev->pix_format.pixelformat;

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

		f->pixelformat = dev->pix_format.pixelformat;
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

/* returns current video format format fmt */
/* NOTE: this is called from the producer
 * so if format has not been negotiated yet,
 * it should return ALL of available formats,
 * called on VIDIOC_G_FMT, with v4l2_buf_type set to V4L2_BUF_TYPE_VIDEO_OUTPUT
 */
static int vidioc_g_fmt_out(struct file *file, void *priv,
		struct v4l2_format *fmt)
{
	struct v4l2_loopback_device *dev;

	MARK();

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	/*
	 * LATER: this should return the currently valid format
	 * gstreamer doesn't like it, if this returns -EINVAL, as it
	 * then concludes that there is _no_ valid format
	 * CHECK whether this assumption is wrong,
	 * or whether we have to always provide a valid format
	 */

	fmt->fmt.pix = dev->pix_format;
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

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	/* TODO(vasaka) loopback does not care about formats writer want to set,
	 * maybe it is a good idea to restrict format somehow
	 */

	if (w > max_width)
		w = max_width;
	if (h > max_height)
		h = max_height;

	pr_debug("trying image %dx%d\n", w, h);

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
	dev->pix_format = fmt->fmt.pix;

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

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	ret = vidioc_try_fmt_out(file, priv, fmt);

	pr_info("s_fmt_out(%d) %d...%d %d %d\n", ret,
		(dev->state & V4L2L_READY_FOR_CAPTURE),
		fmt->fmt.pix.width, fmt->fmt.pix.height,
		dev->pix_format.sizeimage);

	buf[4] = 0;
	pr_debug("outFOURCC=%s\n",
		fourcc2str(dev->pix_format.pixelformat, buf));

	if (ret < 0)
		return ret;

	dev->buffer_size = PAGE_ALIGN(dev->pix_format.sizeimage);
	fmt->fmt.pix.sizeimage = dev->buffer_size;
	pr_info("buffer size %u\n", dev->buffer_size);

	return ret;
}

/* returns the crop capabilities for a device *dev
 * as this is a loopback and does not control an actual sensor,
 * just return the reported maximum supported dimensions
 */
static int v4l2_loopback_cropcap(struct v4l2_loopback_device *dev,
		struct v4l2_cropcap *cropcap)
{
	cropcap->defrect = (struct v4l2_rect) {0, 0,
			V4L2LOOPBACK_SIZE_MAX_WIDTH,
			V4L2LOOPBACK_SIZE_MAX_HEIGHT};
	cropcap->bounds = (struct v4l2_rect) {0, 0,
			V4L2LOOPBACK_SIZE_MAX_WIDTH,
			V4L2LOOPBACK_SIZE_MAX_HEIGHT};
	cropcap->pixelaspect = (struct v4l2_fract){1, 1};
	cropcap->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	return 0;
}

/* set new dimensions for cropping and limit them to acceptable values
 */
static int v4l2_loopback_s_crop(struct v4l2_loopback_device *dev,
		const struct v4l2_crop *crop, bool limit_to_pix_format)
{
	struct v4l2_rect c;
	int ret = 0;

	if (crop != NULL) {
		c = crop->c;

		if (limit_to_pix_format) {
			c.left = min(c.left,
				(typeof(c.left)) dev->pix_format.width - 1);
			c.top = min(c.top,
				(typeof(c.top)) dev->pix_format.height - 1);
		}

		c.left = max(c.left, 0);
		c.top = max(c.top, 0);

		if (limit_to_pix_format) {
			c.width = min(c.width, (typeof(c.width))
				dev->pix_format.width - c.left);
			c.height = min(c.height, (typeof(c.height))
				dev->pix_format.height - c.top);
		}

		c.width = max_t(typeof(c.width),
			c.width, V4L2LOOPBACK_SIZE_MIN_WIDTH);
		c.height = max_t(typeof(c.height),
			c.height, V4L2LOOPBACK_SIZE_MIN_HEIGHT);

		c.left = min(c.left, max_width - 1);
		c.top = min(c.top, max_height - 1);
		c.width = min(c.width, (typeof(c.width)) max_width - c.left);
		c.height = min(c.height, (typeof(c.height)) max_height - c.top);

		if (dev->buffer_size > 0 &&
				c.width * c.height > dev->buffer_size)
			c.height =
				(typeof(c.height)) dev->buffer_size / c.width;

		dev->frame_crop.c = c;
		dev->frame_crop.type = crop->type;
	} else {
		ret = -EINVAL;
	}
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
		pr_err("dev is null");
		return -EINVAL;
	}

	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
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
		pr_err("dev is null");
		return -EINVAL;
	}

	if (parm->type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
		parm->type !=  V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;

	return 0;
}

#ifdef V4L2LOOPBACK_WITH_STD
/* sets a tv standard, actually we do not need to handle this any special way
 * added to support effecttv
 * called on VIDIOC_S_STD
 */
static int vidioc_s_std(struct file *file, void *private_data,
		v4l2_std_id *_std)
{
	v4l2_std_id req_std = 0, supported_std = 0;
	const v4l2_std_id all_std = V4L2_STD_ALL, no_std = 0;

	if (_std) {
		req_std = *_std;
		*_std = all_std;
	}

	/* we support everything in V4L2_STD_ALL, but not more... */
	supported_std = (all_std & req_std);
	if (no_std == supported_std)
		return -EINVAL;

	return 0;
}


/* gets a fake video standard
 * called on VIDIOC_G_STD
 */
static int vidioc_g_std(struct file *file, void *private_data,
		v4l2_std_id *norm)
{
	if (norm)
		*norm = V4L2_STD_ALL;
	return 0;
}
/* gets a fake video standard
 * called on VIDIOC_QUERYSTD
 */
static int vidioc_querystd(struct file *file, void *private_data,
		v4l2_std_id *norm)
{
	if (norm)
		*norm = V4L2_STD_ALL;
	return 0;
}
#endif /* V4L2LOOPBACK_WITH_STD */

static int v4l2loopback_set_ctrl(struct v4l2_loopback_device *dev,
				  u32 id,
				  s64 val)
{
	switch (id) {
	case CID_KEEP_FORMAT:
		if (val < 0 || val > 1)
			return -EINVAL;
		dev->keep_format = val;
		try_free_buffers(dev);
		break;
	case CID_SUSTAIN_FRAMERATE:
		if (val < 0 || val > 1)
			return -EINVAL;
		spin_lock_bh(&dev->lock);
		dev->sustain_framerate = val;
		check_timers(dev);
		spin_unlock_bh(&dev->lock);
		break;
	case CID_TIMEOUT:
		if (val < 0 || val > MAX_TIMEOUT)
			return -EINVAL;
		spin_lock_bh(&dev->lock);
		dev->timeout_jiffies = msecs_to_jiffies(val);
		check_timers(dev);
		spin_unlock_bh(&dev->lock);
		allocate_timeout_image(dev);
		break;
	case CID_TIMEOUT_IMAGE_IO:
		if (val < 0 || val > 1)
			return -EINVAL;
		dev->timeout_image_io = val;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int v4l2loopback_datasize_get_ctrl(struct v4l2_loopback_device *dev,
				  u32 id,
				  s32 *val)
{
	struct v4l2_pix_format pix_format = {};
	const struct v4l2l_format *format;
	__u32 pixfmt;
	__u32 w;
	__u32 h;

	switch (id) {
	case CID_CROP_DATASIZE:
		pixfmt = dev->pix_format.pixelformat;
		format = format_by_fourcc(pixfmt);
		if (!format) {
			pr_err("format is null");
			break;
		}
		w = dev->frame_crop.c.width - dev->frame_crop.c.left;
		h = dev->frame_crop.c.height - dev->frame_crop.c.top;
		pix_format_set_size(&pix_format, format, w, h);
		*val = (s32) pix_format.sizeimage;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int v4l2loopback_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_loopback_device *dev = container_of(ctrl->handler,
			struct v4l2_loopback_device, ctrl_handler);

	return v4l2loopback_set_ctrl(dev, ctrl->id, ctrl->val);
}

int v4l2loopback_g_ctrl(struct v4l2_ctrl *ctrl)
{
	switch(ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = 4;
		pr_info("min_buffers is 4\n");
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int v4l2loopback_datasize_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct v4l2_loopback_device *dev = container_of(ctrl->handler,
			struct v4l2_loopback_device, ctrl_handler);

	return v4l2loopback_datasize_get_ctrl(dev, ctrl->id, &ctrl->val);
}

static int v4l2loopback_datasize_s_ctrl(struct v4l2_ctrl *ctrl)
{
	return -EINVAL;
}

static int v4l2loopback_datasize_try_ctrl(struct v4l2_ctrl *ctrl)
{
	return -EINVAL;
}

/* returns set of device outputs, in our case there is only one
 * called on VIDIOC_ENUMOUTPUT
 */
static int vidioc_enum_output(struct file *file, void *fh,
		struct v4l2_output *outp)
{
	__u32 index = outp->index;
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	MARK();
	if (!dev->announce_all_caps && dev->state != V4L2L_READY_FOR_OUTPUT)
		return -ENOTTY;

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
		pr_err("dev is null");
		return -EINVAL;
	}

	if (!dev->announce_all_caps && dev->state != V4L2L_READY_FOR_OUTPUT)
		return -ENOTTY;
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
		pr_err("dev is null");
		return -EINVAL;
	}

	if (!dev->announce_all_caps && dev->state != V4L2L_READY_FOR_OUTPUT)
		return -ENOTTY;

	if (i)
		return -EINVAL;

	return 0;
}

/* returns set of device inputs, in our case there is only one,
 * but later I may add more
 * called on VIDIOC_ENUMINPUT
 */
static int vidioc_enum_input(struct file *file, void *fh,
		struct v4l2_input *inp)
{
	__u32 index = inp->index;

	MARK();
	if (index != 0)
		return -EINVAL;

	/* clear all data (including the reserved fields) */
	memset(inp, 0, sizeof(*inp));

	inp->index = index;
	strlcpy(inp->name, "loopback", sizeof(inp->name));
	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->audioset = 0;
	inp->tuner = 0;
	inp->status = 0;

#ifdef V4L2LOOPBACK_WITH_STD
	inp->std = V4L2_STD_ALL;
# ifdef V4L2_IN_CAP_STD
	inp->capabilities |= V4L2_IN_CAP_STD;
# endif
#endif /* V4L2LOOPBACK_WITH_STD */

	return 0;
}

/* which input is currently active,
 * called on VIDIOC_G_INPUT
 */
static int vidioc_g_input(struct file *file, void *fh, unsigned int *i)
{
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (!dev->announce_all_caps && !(dev->state & V4L2L_READY_FOR_CAPTURE))
		return -ENOTTY;
	if (i)
		*i = 0;
	return 0;
}

/* set input, can make sense if we have more than one video src,
 * called on VIDIOC_S_INPUT
 */
static int vidioc_s_input(struct file *file, void *fh, unsigned int i)
{
	struct v4l2_loopback_device *dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (!dev->announce_all_caps && !(dev->state & V4L2L_READY_FOR_CAPTURE))
		return -ENOTTY;
	if (i == 0)
		return 0;
	return -EINVAL;
}

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

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(fh);

	/* release buffer when request buffer 0*/
	if (b->count == 0) {
		free_buffers(dev);
		send_v4l2_event_ex(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_ALLOC_BUFS, 0);
		dev->used_buffers = 0;
		return 0;
	}

	if (opener->timeout_image_io) {
		if (b->memory != V4L2_MEMORY_MMAP && b->memory !=
				V4L2_MEMORY_USERPTR)
			return -EINVAL;
		b->count = 1;
		return 0;
	}

	init_buffers(dev);
	switch (b->memory) {
	case V4L2_MEMORY_USERPTR:
	case V4L2_MEMORY_MMAP:
		/* do nothing here, buffers are always allocated */
		if (b->count < 1 || dev->buffers_number < 1)
			return 0;

		if (b->count > dev->buffers_number)
			b->count = dev->buffers_number;

		dev->used_buffers = b->count;

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
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;

	MARK();

	type = b->type;
	index = b->index;
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(fh);

	if ((b->type != V4L2_BUF_TYPE_VIDEO_CAPTURE) &&
			(b->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)) {
		return -EINVAL;
	}
	if (b->index > max_buffers)
		return -EINVAL;

	if (opener->timeout_image_io)
		*b = dev->timeout_image_buffer.buffer;
	else
		*b = dev->buffers[b->index % dev->used_buffers].buffer;

	b->type = type;
	b->index = index;
	pr_debug("buffer type: %d (of %d with size=%ld)\n",
			b->memory, dev->buffers_number, dev->buffer_size);
	pr_info ("buffer %d %u\n", b->index, b->length);

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
	struct v4l2_loopback_device *dev;
	int fd = 0;
	int rc;
	struct dma_buf *dmabuf = NULL;

	MARK();

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (e->index == -1U) {
		pr_info("support DMABUF\n");
		return 0;
	}

	if (e->index >= dev->used_buffers) {
		pr_warn("invalid index %d", e->index);
		return -EINVAL;
	}

	switch (e->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE: {
		pr_info("export buffer %d\n", e->index);
		if (dev->dmabufs[0] == NULL ||
			dev->dmabufs[e->index] == NULL) {
			dev->io_mode = V4L2L_IO_MODE_DMABUF;

			rc = allocate_dma_buffers(dev);
			if (rc) {
				pr_err("fail to allocate dma buffers\n");
				return -EINVAL;
			}
		}

		fd = dma_buf_fd(dev->dmabufs[e->index], O_CLOEXEC);
		if (fd < 0) {
			pr_err("dma get fd fail, *fd=%d\n", fd);
			return -EINVAL;
		}

		/*
		 * increment the ref count so that ref count becomes 2 here
		 * when we close fd, refcount becomes 1 and when we do
		 * dmap_put_buf, ref count becomes 0 and memory will be freed.
		 */
		dmabuf = dma_buf_get(fd);
		if (dmabuf == NULL) {
			pr_err("dma_buf_get failed, fd=%d\n", fd);
			return -EINVAL;
		}

		break;
	}
	case V4L2_BUF_TYPE_VIDEO_OUTPUT: {
		break;
	}

	}

	pr_debug("exit %s\n", __func__);

	e->fd = fd;
	return 0;
}

static int can_read(struct v4l2_loopback_device *dev,
		struct v4l2_loopback_opener *opener)
{
	int ret;

	mutex_lock(&dev->capbufs_mutex);
	ret = !list_empty(&dev->capbufs_list);
	mutex_unlock(&dev->capbufs_mutex);

	return ret;
}


static int set_bufstate(struct v4l2_loopback_device *dev,
	struct v4l2l_buffer* b, enum V4L2_LOOPBACK_BUF_STATE state)
{
	int ret = 0;

	mutex_lock(&dev->buf_mutex);

	switch(b->state)
	{
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
		pr_err("[dev %s] fail to set buffer state %u current state %u\n",
			dev->vdev->name, state, b->state);
	}

	mutex_unlock(&dev->buf_mutex);

	return ret;
}

static void set_allbufs_state(struct v4l2_loopback_device *dev,
	enum V4L2_LOOPBACK_BUF_STATE state)
{
	int i = 0;

	mutex_lock(&dev->buf_mutex);
	for (i = 0; i < dev->used_buffers; ++i) {
		dev->buffers[i].state = state;
	}

	mutex_unlock(&dev->buf_mutex);
}

/* put buffer to queue
 * called on VIDIOC_QBUF
 */
static int vidioc_qbuf(struct file *file,
		void *private_data, struct v4l2_buffer *buf)
{
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	struct v4l2l_buffer *b;
	int index;
	int rc;


	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(private_data);

	if (buf->index > max_buffers)
		return -EINVAL;
	if (opener->timeout_image_io)
		return 0;

	index = buf->index % dev->used_buffers;
	b = &dev->buffers[index];

	switch (buf->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pr_debug("[dev %s] capture QBUF index: %d\n", dev->vdev->name, index);

		rc = set_bufstate(dev, b, V4L2L_BUF_READY_Q);
		if (rc < 0)
			pr_err("[dev %s] capture QBUF index: %d fail\n", dev->vdev->name, b->buffer.index);
		else {
			mutex_lock(&dev->outbufs_mutex);
			list_add_tail(&b->list_head, &dev->outbufs_list);
			mutex_unlock(&dev->outbufs_mutex);
			send_v4l2_event(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_OUTPUT_BUF_READY);
		}
		return rc;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT: {
		__u64 payload = 0;
		pr_debug("[dev %s] output QBUF pos: %d index: %d\n",
				dev->vdev->name, dev->write_position, index);
		if (buf->timestamp.tv_sec == 0 && buf->timestamp.tv_usec == 0)
			cam_common_util_get_curr_timestamp(&b->buffer.timestamp);
		else
			b->buffer.timestamp = buf->timestamp;

		b->buffer.sequence = buf->sequence;
		rc = set_bufstate(dev, b, V4L2L_BUF_DONE_Q);
		if (rc < 0)
			pr_err("[dev %s] output QBUF index: %d fail\n", dev->vdev->name, b->buffer.index);
		else {
			mutex_lock(&dev->capbufs_mutex);
			list_add_tail(&b->list_head, &dev->capbufs_list);
			mutex_unlock(&dev->capbufs_mutex);

			// copy from user to b->ext
			payload = *((__u64*)&buf->reserved2);
			if (payload !=0) {
				if (copy_from_user(&(b->ext),
					u64_to_user_ptr(payload),
					sizeof(b->ext))) {
					rc = -EFAULT;
					pr_err("copy_from_user fail on qbuf\n");
				}
			}

			wake_up_all(&dev->read_event);
		}
		return rc;
	}
	default:
		pr_err("unsupported buf type %d\n", buf->type);
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
	struct v4l2l_buffer *b;
	int rc = 0;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(private_data);
	if (opener->timeout_image_io) {
		*buf = dev->timeout_image_buffer.buffer;
		return 0;
	}

	switch (buf->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		mutex_lock(&dev->capbufs_mutex);
		if (!list_empty(&dev->capbufs_list)) {
			b = list_first_entry(&dev->capbufs_list,
						struct v4l2l_buffer, list_head);

			rc = set_bufstate(dev, b, V4L2L_BUF_USER_ACQUIRED);
			if (rc < 0) {
				mutex_unlock(&dev->capbufs_mutex);
				pr_err("[dev %s] capture DQBUF index: %d fail\n", dev->vdev->name, b->buffer.index);
			} else {
				__u64 payload = 0;
				list_del_init(&b->list_head);
				mutex_unlock(&dev->capbufs_mutex);

				pr_debug("[dev %s] capture DQBUF index: %d\n", dev->vdev->name, b->buffer.index);
				// copy to user *buf.reversed2 from b->ext
				payload = *((__u64*)&buf->reserved2);
				if (payload != 0) {
					if (copy_to_user(u64_to_user_ptr(payload),
						&b->ext,
						sizeof(b->ext))) {
						rc = -EFAULT;
						pr_err("copy_to_user fail on dqbuf\n");
					}
					b->ext.batch_num = 0;
				}

				*buf = b->buffer;
			}
		} else {
			mutex_unlock(&dev->capbufs_mutex);

			pr_warn("[dev %s] capture list is empty\n", dev->vdev->name);
			if (file->f_flags & O_NONBLOCK) {
				return -EAGAIN;
			}

			wait_event_interruptible(dev->read_event, can_read(dev, opener));
		}

		return rc;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		mutex_lock(&dev->outbufs_mutex);
		if (!list_empty(&dev->outbufs_list)) {
			b = list_first_entry(&dev->outbufs_list,
					struct v4l2l_buffer, list_head);

			rc = set_bufstate(dev, b, V4L2L_BUF_PROXY_ACQUIRED);
			if (rc < 0) {
				mutex_unlock(&dev->outbufs_mutex);
				pr_err("[dev %s] output DQBUF index: %d fail\n", dev->vdev->name, b->buffer.index);
			}
			else {
				list_del_init(&b->list_head);
				pr_debug("[dev %s] output DQBUF index: %d\n", dev->vdev->name, b->buffer.index);
				mutex_unlock(&dev->outbufs_mutex);

				*buf = b->buffer;
				buf->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			}
		} else {
			mutex_unlock(&dev->outbufs_mutex);
			pr_warn("[dev %s] output list is empty\n", dev->vdev->name);
			// for proxy, must use NONBLOCK method
			return -EAGAIN;
		}
		return rc;
	default:
		pr_err("[dev %s] unsupported buf type %d\n", dev->vdev->name, buf->type);
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
	struct v4l2l_buffer *pos, *n;
	int rc = 0;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	opener = fh_to_opener(private_data);

	switch (type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		return 0;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE: {
		pr_info("streamon\n");
		set_allbufs_state(dev, V4L2L_BUF_PROXY_ACQUIRED);
		mutex_lock(&dev->outbufs_mutex);
		list_for_each_entry_safe(pos, n,
			&dev->outbufs_list, list_head) {
			list_del_init(&pos->list_head);
		}
		INIT_LIST_HEAD(&dev->outbufs_list);
		mutex_unlock(&dev->outbufs_mutex);

		mutex_lock(&dev->capbufs_mutex);
		list_for_each_entry_safe(pos, n,
			&dev->capbufs_list, list_head) {
			list_del_init(&pos->list_head);
		}
		INIT_LIST_HEAD(&dev->capbufs_list);
		mutex_unlock(&dev->capbufs_mutex);

		send_v4l2_event(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_START_INPUT);
		rc = wait_for_completion_timeout(&dev->ctrl_complete,
					msecs_to_jiffies(START_TIMEOUT));
		if (rc) {
			rc = dev->qcarcam_ctrl_ret;
		} else {
			pr_err("streamon fail, timeout %d\n", rc);
			rc = -ETIMEDOUT;
		}
		return rc;
	}
	default:
		return -EINVAL;
	}
}

/* stop streaming
 * called on VIDIOC_STREAMOFF
 */
static int vidioc_streamoff(struct file *file,
		void *private_data, enum v4l2_buf_type type)
{
	struct v4l2_loopback_device *dev;
	struct v4l2l_buffer *pos, *n;
	int rc = 0;

	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	switch (type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		return 0;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pr_info("streamoff\n");
		set_allbufs_state(dev, V4L2L_BUF_PENDING);

		send_v4l2_event(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_STOP_INPUT);
		rc = wait_for_completion_timeout(&dev->ctrl_complete,
					msecs_to_jiffies(STOP_TIMEOUT));
		if (rc) {
			rc = dev->qcarcam_ctrl_ret;
			pr_info("clear list when streamoff\n");
			mutex_lock(&dev->outbufs_mutex);
			list_for_each_entry_safe(pos, n,
				&dev->outbufs_list, list_head) {
				list_del_init(&pos->list_head);
			}
			INIT_LIST_HEAD(&dev->outbufs_list);
			mutex_unlock(&dev->outbufs_mutex);

			mutex_lock(&dev->capbufs_mutex);
			list_for_each_entry_safe(pos, n,
				&dev->capbufs_list, list_head) {
				list_del_init(&pos->list_head);
			}
			INIT_LIST_HEAD(&dev->capbufs_list);
			mutex_unlock(&dev->capbufs_mutex);

		} else {
			pr_err("streamoff fail, timeout %d\n", rc);
			rc = -ETIMEDOUT;
		}



		return rc;
	default:
		return -EINVAL;
	}
	return -EINVAL;
}

#ifdef CONFIG_VIDEO_V4L1_COMPAT
static int vidiocgmbuf(struct file *file, void *fh, struct video_mbuf *p)
{
	struct v4l2_loopback_device *dev;

	MARK();
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}
	p->frames = dev->buffers_number;
	p->offsets[0] = 0;
	p->offsets[1] = 0;
	p->size = dev->buffer_size;
	return 0;
}
#endif

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

static int process_capture_cmd(struct v4l2_loopback_device *dev,
	struct ais_v4l2_control_t *kcmd)
{
	int rc = 0;

	switch (kcmd->cmd) {
	case AIS_V4L2_CAPTURE_PRIV_SET_PARAM: {
		if (kcmd->size > MAX_AIS_V4L2_PAYLOAD_SIZE) {
			rc = -EINVAL;
		} else if (u64_to_user_ptr(kcmd->payload) == NULL) {
			rc = -EINVAL;
			pr_err("payload is NULL on set param\n");
		} else if (copy_from_user(dev->qcarcam_param,
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
			rc = -EFAULT;
			pr_err("copy_from_user fail on set param\n");
		} else {
			pr_info("s_param %u\n", kcmd->param_type);

			dev->qcarcam_param_size = kcmd->size;
			dev->qcarcam_code = kcmd->param_type;

			send_v4l2_event_ex(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_SET_PARAM, kcmd->param_type);
			/* wait for the signal */
			rc = wait_for_completion_timeout(&dev->sparam_complete,
					msecs_to_jiffies(SPARAM_TIMEOUT));
			if (rc) {
				rc = dev->qcarcam_sparam_ret;
			} else {
				pr_err("s_param fail, timeout %d\n", rc);
				rc = -ETIMEDOUT;
			}
		}

		break;
	}
	case AIS_V4L2_CAPTURE_PRIV_GET_PARAM: {
		if (kcmd->size > MAX_AIS_V4L2_PAYLOAD_SIZE) {
			rc = -EINVAL;
		} else if (u64_to_user_ptr(kcmd->payload) == NULL) {
			rc = -EINVAL;
			pr_err("payload is NULL on set param\n");
		} else {
			pr_info("get_param %u\n", kcmd->param_type);

			/* send the payload to userspace by v4l2_event
			 * when payload size is large, can't send by v4l2 */
			if (kcmd->size > MAX_AIS_V4L2_PARAM_EVNET_SIZE &&
				copy_from_user(dev->qcarcam_param,
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
				rc = -EFAULT;
				pr_err("copy_from_user fail on get param\n");
				return rc;
			}

			dev->qcarcam_param_size = kcmd->size;
			dev->qcarcam_code = kcmd->param_type;

			/* send get_param to output end */
			send_v4l2_event_ex2(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_GET_PARAM, kcmd->param_type, kcmd->size, kcmd->payload);

			/* wait for the signal */
			rc = wait_for_completion_timeout(&dev->gparam_complete,
				msecs_to_jiffies(GPARAM_TIMEOUT));
			if (rc) {
				rc = 0;
			} else {
				pr_err("g_param fail, timeout %d\n", rc);
				rc = -ETIMEDOUT;
				return rc;
			}

			if (dev->qcarcam_param_size == 0) {
				/* g_param fail */
				rc = -EINVAL;
				pr_err("get param size is 0\n");
			} else if (copy_to_user(u64_to_user_ptr(kcmd->payload),
						dev->qcarcam_param,
						dev->qcarcam_param_size)) {
				/* copy the param from kernel */
				rc = -EFAULT;
				pr_err("copy_from_user fail on get param\n");
			} else {
				kcmd->size = dev->qcarcam_param_size;
				kcmd->param_type = dev->qcarcam_code;
				pr_info("succeed to get param\n");
			}
		}
		break;
	}
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int process_output_cmd(struct v4l2_loopback_device *dev,
	struct ais_v4l2_control_t *kcmd)
{
	int rc = 0;
	struct v4l2_event event;
	struct ais_v4l2_buffers_t bufs;
	struct dma_buf *dmabuf = NULL;
	uint32_t i;

	switch (kcmd->cmd) {
	case AIS_V4L2_OUTPUT_PRIV_SET_PARAM_EVENT: {

		if (kcmd->size > MAX_AIS_V4L2_PARAM_EVNET_SIZE) {
			rc = -EINVAL;
			pr_err("fail to set param event, size=%u\n",
				kcmd->size);
		} else if (copy_from_user(&event.u.data[2],
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
			rc = -EFAULT;
			pr_err("fail to copy from user on set param event\n");
		} else {
			event.type = AIS_V4L2_CLIENT_CAPTURE;
			event.id = AIS_V4L2_PARAM_EVENT;

			event.u.data[0] = kcmd->param_type;
			event.u.data[1] = kcmd->size;
			v4l2_event_queue(dev->vdev, &event);
			pr_debug("send AIS_V4L2_PARAM_EVENT :%d %d\n",
				event.u.data[2], event.u.data[3]);
		}
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_PARAM_RET:
		/* signal capture_s_param_cond */
		dev->qcarcam_sparam_ret = kcmd->param_type;
		complete(&dev->sparam_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_SET_PARAM2:
		if (kcmd->size > MAX_AIS_V4L2_PAYLOAD_SIZE) {
			rc = -EINVAL;
		} else if (copy_from_user(dev->qcarcam_param,
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
			rc = -EFAULT;
			pr_err("fail to AIS_V4L2_OUTPUT_PRIV_SET_PARAM2\n");
		} else {
			dev->qcarcam_param_size = kcmd->size;
			dev->qcarcam_code = kcmd->param_type;
			/* signal capture_g_param_cond */
			complete(&dev->gparam_complete);
		}
		break;
	case AIS_V4L2_OUTPUT_PRIV_GET_PARAM:
		/* copy the param from kernel */
		if (copy_to_user(u64_to_user_ptr(kcmd->payload),
						dev->qcarcam_param,
						dev->qcarcam_param_size)) {
			rc = -EFAULT;
			pr_err("fail to AIS_V4L2_OUTPUT_PRIV_GET_PARAM\n");
		} else {
			kcmd->size = dev->qcarcam_param_size;
			kcmd->param_type = dev->qcarcam_code;
		}
		break;
	case AIS_V4L2_OUTPUT_PRIV_OPEN_RET:
		dev->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		complete(&dev->ctrl_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_START_RET:
		dev->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		complete(&dev->ctrl_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_STOP_RET:
		dev->qcarcam_ctrl_ret = kcmd->ctrl_ret;
		complete(&dev->ctrl_complete);
		break;
	case AIS_V4L2_OUTPUT_PRIV_SET_BUFS: {
		if (kcmd->size != sizeof(bufs)) {
			rc = -EINVAL;
			pr_err("kcmd->size != sizeof(bufs)\n");
		} else if (copy_from_user(&bufs,
				u64_to_user_ptr(kcmd->payload),
				sizeof(bufs))) {
			rc = -EFAULT;
			pr_err("fail to AIS_V4L2_OUTPUT_PRIV_SET_BUFS\n");
		} else {
			pr_info("setbufs %u begin\n", bufs.nbufs);

			for (i = 0; i < bufs.nbufs; ++i) {
				dmabuf = dma_buf_get(bufs.fds[i]);
				if (dmabuf == NULL) {
					pr_err("dma_buf_get failed, fd=%d\n", bufs.fds[i]);
					return -EINVAL;
				}

				dev->dmabufs[i] = dmabuf;
			}

			dev->allocbufs_ret = kcmd->ctrl_ret;
			complete(&dev->allocbufs_complete);

			pr_info("setbufs %u end\n", bufs.nbufs);
		}
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_INPUT_SIGNAL_EVENT: {
		if (copy_from_user(&event.u.data[2],
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
			rc = -EFAULT;
			pr_err("fail to copy from user on input signal\n");
		} else {
			event.type = AIS_V4L2_CLIENT_CAPTURE;
			event.id = AIS_V4L2_EVENT_INPUT_SIGNAL;

			event.u.data[0] = kcmd->param_type;
			event.u.data[1] = kcmd->size;
			v4l2_event_queue(dev->vdev, &event);
			pr_debug("send AIS_V4L2_EVENT_INPUT_SIGNAL : %d\n",
				event.u.data[2]);
		}
		break;
	}
	case AIS_V4L2_OUTPUT_PRIV_SET_ERROR_EVENT: {
		if (copy_from_user(&event.u.data[2],
				u64_to_user_ptr(kcmd->payload),
				kcmd->size)) {
			rc = -EFAULT;
			pr_err("fail to copy from user on input signal\n");
		} else {
			event.type = AIS_V4L2_CLIENT_CAPTURE;
			event.id = AIS_V4L2_EVENT_ERROR;

			event.u.data[0] = kcmd->param_type;
			event.u.data[1] = kcmd->size;
			v4l2_event_queue(dev->vdev, &event);
			pr_debug("send AIS_V4L2_EVENT_ERROR :%d\n",
				event.u.data[2]);
		}
		break;
	}
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

	pr_debug("%s enter\n", __func__);

	if (!dev) {
		pr_err("v4l2loopback dev NULL\n");
		return -EINVAL;
	}

	if (!kcmd || !arg)
		return -EINVAL;

	if (cmd != VIDIOC_CAM_CONTROL)
		return -ENOIOCTLCMD;

	switch (kcmd->type) {
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		rc = process_capture_cmd(dev, kcmd);
		break;
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		rc = process_output_cmd(dev, kcmd);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

/* file operations */
static void vm_open(struct vm_area_struct *vma)
{
	struct v4l2l_buffer *buf;

	MARK();

	buf = vma->vm_private_data;
	buf->use_count++;
}

static void vm_close(struct vm_area_struct *vma)
{
	struct v4l2l_buffer *buf;

	MARK();
	buf = vma->vm_private_data;
	buf->use_count--;
}

static const struct vm_operations_struct vm_ops = {
	.open = vm_open,
	.close = vm_close,
};

static unsigned int v4l2_loopback_poll(struct file *file,
						struct poll_table_struct *pts)
{
	struct v4l2_loopback_opener *opener;
	struct v4l2_loopback_device *dev;
	unsigned long req_events = poll_requested_events(pts);
	int ret_mask = 0;

	MARK();

	opener = fh_to_opener(file->private_data);
	dev = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}

	if (req_events & POLLPRI) {
		if (!v4l2_event_pending(&opener->fh))
			poll_wait(file, &opener->fh.wait, pts);
		if (v4l2_event_pending(&opener->fh)) {
			ret_mask |= POLLPRI;
			if (!(req_events & DEFAULT_POLLMASK))
				return ret_mask;
		}
	}

	switch (opener->type) {
	case V4L2L_WRITER:
		ret_mask |= POLLOUT | POLLWRNORM;
		break;
	case V4L2L_READER:
		if (!can_read(dev, opener)) {
			if (ret_mask)
				return ret_mask;

			poll_wait(file, &dev->read_event, pts);
		}
		if (can_read(dev, opener))
			ret_mask |= POLLIN | POLLRDNORM;
		if (v4l2_event_pending(&opener->fh))
			ret_mask |= POLLPRI;
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
	struct v4l2_loopback_device *dev;
	struct v4l2_loopback_opener *opener;
	int rc = 0;
	enum v4l2_loopback_opener_type etype;

	MARK();
	dev = v4l2loopback_getdevice(file);

	if (dev == NULL) {
		pr_err("\ndev value is null\n");
		return -EINVAL;
	}

	if (dev->open_count.counter >= dev->max_openers)
		return -EBUSY;

	/* WRITEONLY should be open operation from proxy */
	if ((file->f_mode & (FMODE_READ | FMODE_WRITE)) == FMODE_WRITE) {
		if (dev->state == V4L2L_READY_FOR_OUTPUT) {
			etype = V4L2L_WRITER;
			dev->state = V4L2L_READY_FOR_CAPTURE;
		} else {
			pr_err("invalid open, only proxy can open\n");
			return -EINVAL;
		}
	} else {
		/* app open it */
		if (dev->state == V4L2L_READY_FOR_CAPTURE) {
			etype = V4L2L_READER;
			send_v4l2_event(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_OPEN_INPUT);
			rc = wait_for_completion_timeout(&dev->ctrl_complete,
					msecs_to_jiffies(OPEN_TIMEOUT));
			if (rc) {
				rc = dev->qcarcam_ctrl_ret;
				if (!dev->qcarcam_ctrl_ret) {
					dev->state = V4L2L_OPENED;
					pr_info("app open succeed\n");
				} else {
					pr_err("app open fail\n");
					return rc;
				}
			} else {
				pr_err("open fail, timeout %d\n", rc);
				rc = -ETIMEDOUT;
				return rc;
			}
		} else {
			pr_err("invalid operation state %d\n", dev->state);
			return -EINVAL;
		}
	}

	/* kfree on close */
	opener = kzalloc(sizeof(*opener), GFP_KERNEL);
	if (opener == NULL)
		return -ENOMEM;

	opener->type = etype;

	v4l2_fh_init(&opener->fh, video_devdata(file));
	file->private_data = &opener->fh;
	atomic_inc(&dev->open_count);

	opener->timeout_image_io = dev->timeout_image_io;
	dev->timeout_image_io = 0;

	if (opener->timeout_image_io) {
		int r = allocate_timeout_image(dev);

		if (r < 0) {
			pr_err("timeout image allocation failed\n");
			return r;
		}
	}
	v4l2_fh_add(&opener->fh);

	pr_debug("opened dev:%pK with image:%pK\n",
			dev, dev ? dev->image : NULL);
	MARK();
	return rc;
}

static int v4l2_loopback_close(struct file *file)
{
	struct v4l2_loopback_opener *opener;
	struct v4l2_loopback_device *dev;
	int iswriter = 0;

	MARK();
	opener = fh_to_opener(file->private_data);
	dev    = v4l2loopback_getdevice(file);
	if (!dev) {
		pr_err("dev is null");
		return -EINVAL;
	}


	if (opener->type == V4L2L_WRITER)
		iswriter = 1;

	if (iswriter) {
		/* todo: should notify the capture app there is error*/
		if (dev->state != V4L2L_READY_FOR_OUTPUT)
			dev->state = V4L2L_READY_FOR_OUTPUT;
		else
			pr_warn("invalid proxy close, state %d\n", dev->state);
	} else {
		/* notify ais_v4l2_proxy to close the input */
		if (dev->state > V4L2L_READY_FOR_CAPTURE) {
			send_v4l2_event(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
				AIS_V4L2_CLOSE_INPUT);
			free_buffers(dev);
			reinit_completion(&dev->gparam_complete);
			reinit_completion(&dev->sparam_complete);
			reinit_completion(&dev->ctrl_complete);
			INIT_LIST_HEAD(&dev->outbufs_list);
			INIT_LIST_HEAD(&dev->capbufs_list);

			dev->state = V4L2L_READY_FOR_CAPTURE;
		} else {
			pr_warn("invalid close state %d\n", dev->state);
		}
	}

	v4l2_fh_del(file->private_data);
	v4l2_fh_exit(file->private_data);

	atomic_dec(&dev->open_count);
	if (dev->open_count.counter == 0) {
		del_timer_sync(&dev->sustain_timer);
		del_timer_sync(&dev->timeout_timer);
	}

	kfree(opener);

	MARK();

	return 0;
}

/* init functions */
/* frees buffers, if already allocated */
static int free_buffers(struct v4l2_loopback_device *dev)
{
	int i;

	MARK();
	pr_info("freeing image@%pK for dev:%pK\n",
		dev ? dev->image : NULL, dev);

	if (dev == NULL) {
		pr_err("\ndev value is null\n");
		return -EINVAL;
	}

	mutex_lock(&dev->buf_mutex);

	if (dev->image) {
		vfree(dev->image);
		dev->image = NULL;
	}
	if (dev->timeout_image) {
		vfree(dev->timeout_image);
		dev->timeout_image = NULL;
	}
	for (i = 0; i < dev->buffers_number; ++i) {
		if (dev->dmabufs[i]) {
			dev->buffers[i].kvaddr = 0;
			dma_buf_put((struct dma_buf *)dev->dmabufs[i]);
			dev->dmabufs[i] = NULL;
		}
	}

	dev->imagesize = 0;

	mutex_unlock(&dev->buf_mutex);

	return 0;
}

/* frees buffers, if they are no longer needed */
static void try_free_buffers(struct v4l2_loopback_device *dev)
{
	MARK();
	if ((dev->open_count.counter && !dev->keep_format) == 0) {
		free_buffers(dev);
		dev->write_position = 0;
	}
}

/* allocates buffers, if buffer_size is set */
static int allocate_buffers(struct v4l2_loopback_device *dev)
{
	MARK();

	/* vfree on close file operation in case no open handles left */
	if (dev->buffer_size == 0)
		return -EINVAL;

	if (dev->image) {
		pr_debug("allocating buffers again: %ld %ld\n",
			dev->buffer_size * dev->buffers_number, dev->imagesize);
		/* FIXME: prevent double allocation more intelligently! */
		if (dev->buffer_size * dev->buffers_number == dev->imagesize)
			return 0;

		/* if there is only one writer, no problem should occur */
		if (dev->open_count.counter == 1)
			free_buffers(dev);
		else
			return -EINVAL;
	}

	dev->imagesize = dev->buffer_size * dev->buffers_number;

	pr_debug("allocating %ld = %ldx%d\n",
			dev->imagesize, dev->buffer_size, dev->buffers_number);

	dev->image = vmalloc(dev->imagesize);
	if (dev->timeout_jiffies > 0)
		allocate_timeout_image(dev);

	if (dev->image == NULL)
		return -ENOMEM;
	pr_debug("vmallocated %ld bytes\n", dev->imagesize);
	MARK();
	init_buffers(dev);
	return 0;
}

/* allocates buffers, if buffer_size is set */
static int allocate_dma_buffers(struct v4l2_loopback_device *dev)
{
	int rc = 0;

	send_v4l2_event_ex(dev->vdev, AIS_V4L2_CLIENT_OUTPUT,
			AIS_V4L2_ALLOC_BUFS, dev->used_buffers);
	/* wait for the signal */
	rc = wait_for_completion_timeout(&dev->allocbufs_complete,
		msecs_to_jiffies(ALLOCBUFS_TIMEOUT));
	if (rc) {
		rc = 0;
	} else {
		pr_err("allocate fail, timeout %d\n", rc);
		rc = -ETIMEDOUT;
		return rc;
	}

	if (!dev->allocbufs_ret)
		init_buffers(dev);

	return dev->allocbufs_ret;
}


/* init inner buffers, they are capture mode and flags are set as
 * for capture mod buffers
 */
static void init_buffers(struct v4l2_loopback_device *dev)
{
	int i;
	int buffer_size;
	int bytesused;

	MARK();
	buffer_size = dev->buffer_size;
	bytesused = dev->pix_format.sizeimage;

	for (i = 0; i < dev->buffers_number; ++i) {
		struct v4l2_buffer *b = &dev->buffers[i].buffer;

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
		dev->buffers[i].state = V4L2L_BUF_USER_ACQUIRED;
		dev->buffers[i].ext.batch_num = 0;

		cam_common_util_get_curr_timestamp(&b->timestamp);
	}
	dev->timeout_image_buffer = dev->buffers[0];
	dev->timeout_image_buffer.buffer.m.offset = MAX_BUFFERS * buffer_size;
	MARK();
}

static int allocate_timeout_image(struct v4l2_loopback_device *dev)
{
	MARK();
	if (dev->buffer_size <= 0)
		return -EINVAL;

	if (dev->timeout_image == NULL) {
		dev->timeout_image = v4l2l_vzalloc(dev->buffer_size);
		if (dev->timeout_image == NULL)
			return -ENOMEM;
	}
	return 0;
}

/* fills and register video device */
static void init_vdev(struct video_device *vdev, int nr)
{
	MARK();
	vidioc_fill_name(vdev->name, sizeof(vdev->name), nr);

#ifdef V4L2LOOPBACK_WITH_STD
	vdev->tvnorms      = V4L2_STD_ALL;
#endif /* V4L2LOOPBACK_WITH_STD */

	vdev->vfl_type     = VFL_TYPE_GRABBER;
	vdev->fops         = &v4l2_loopback_fops;
	vdev->ioctl_ops    = &v4l2_loopback_ioctl_ops;
	vdev->release      = &video_device_release;
	vdev->minor        = -1;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE;


	if (debug > 1)
		vdev->dev_debug =
			V4L2_DEV_DEBUG_IOCTL | V4L2_DEV_DEBUG_IOCTL_ARG;

	/* since kernel-3.7, there is a new field 'vfl_dir' that has to be
	 * set to VFL_DIR_M2M for bidrectional devices
	 */
	vdev->vfl_dir = VFL_DIR_M2M;

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

static void check_timers(struct v4l2_loopback_device *dev)
{
	if (!(dev->state & V4L2L_READY_FOR_CAPTURE))
		return;

	if (dev->timeout_jiffies > 0 && !timer_pending(&dev->timeout_timer))
		mod_timer(&dev->timeout_timer, jiffies + dev->timeout_jiffies);
	if (dev->sustain_framerate && !timer_pending(&dev->sustain_timer))
		mod_timer(&dev->sustain_timer, jiffies
				+ dev->frame_jiffies * 3 / 2);
}
#ifdef HAVE_TIMER_SETUP
static void sustain_timer_clb(struct timer_list *t)
{
	struct v4l2_loopback_device *dev = from_timer(dev, t, sustain_timer);
#else
static void sustain_timer_clb(unsigned long nr)
{
	struct v4l2_loopback_device *dev = devs[nr];
#endif
	spin_lock(&dev->lock);
	if (dev->sustain_framerate) {
		dev->reread_count++;
		pr_debug("reread: %d %d\n",
				dev->write_position, dev->reread_count);
		if (dev->reread_count == 1)
			mod_timer(&dev->sustain_timer, jiffies
					+ max(1UL, dev->frame_jiffies / 2));
		else
			mod_timer(&dev->sustain_timer, jiffies
					+ dev->frame_jiffies);
		wake_up_all(&dev->read_event);
	}
	spin_unlock(&dev->lock);
}
#ifdef HAVE_TIMER_SETUP
static void timeout_timer_clb(struct timer_list *t)
{
	struct v4l2_loopback_device *dev = from_timer(dev, t, timeout_timer);
#else
static void timeout_timer_clb(unsigned long nr)
{
	struct v4l2_loopback_device *dev = devs[nr];
#endif
	spin_lock(&dev->lock);
	if (dev->timeout_jiffies > 0) {
		dev->timeout_happened = 1;
		mod_timer(&dev->timeout_timer, jiffies + dev->timeout_jiffies);
		wake_up_all(&dev->read_event);
	}
	spin_unlock(&dev->lock);
}

/* init loopback main structure */
static int v4l2_loopback_init(struct v4l2_loopback_device *dev, int nr)
{
	int ret;
	struct v4l2_ctrl_handler *hdl = &dev->ctrl_handler;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_ctrl *ctrl;

	snprintf(dev->v4l2_dev.name, sizeof(dev->v4l2_dev.name),
				"v4l2loopback-%03d", nr);
	ret = v4l2_device_register(NULL, &dev->v4l2_dev);
	if (ret)
		return ret;

	MARK();
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
	init_capture_param(&dev->capture_param);
	set_timeperframe(dev, &dev->capture_param.timeperframe);
	dev->keep_format = 0;
	dev->sustain_framerate = 0;
	dev->buffers_number = MAX_AIS_BUFFERS_NUM;
	dev->used_buffers = max_buffers;
	dev->max_openers = max_openers;
	dev->write_position = 0;
	spin_lock_init(&dev->lock);

	// when alloc buffer, all the buffers are acquired by user
	INIT_LIST_HEAD(&dev->outbufs_list);
	INIT_LIST_HEAD(&dev->capbufs_list);

	memset(dev->bufpos2index, 0, sizeof(dev->bufpos2index));
	memset(dev->dmabufs, 0, sizeof(dev->dmabufs));
	atomic_set(&dev->open_count, 0);
	dev->state = V4L2L_READY_FOR_OUTPUT;
	dev->announce_all_caps = (!exclusive_caps[nr]);

	dev->buffer_size = 0;
	dev->image = NULL;
	dev->imagesize = 0;
#ifdef HAVE_TIMER_SETUP
	timer_setup(&dev->sustain_timer, sustain_timer_clb, 0);
	timer_setup(&dev->timeout_timer, timeout_timer_clb, 0);
#else
	setup_timer(&dev->sustain_timer, sustain_timer_clb, nr);
	setup_timer(&dev->timeout_timer, timeout_timer_clb, nr);
#endif
	dev->reread_count = 0;
	dev->timeout_jiffies = 0;
	dev->timeout_image = NULL;
	dev->timeout_happened = 0;

	ret = v4l2_ctrl_handler_init(hdl, 1);
	if (ret)
		goto error;
	v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_keepformat, NULL);
	v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_sustainframerate, NULL);
	v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_timeout, NULL);
	v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_timeoutimageio, NULL);

	ctrl = v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_buffernum, NULL);
	if (ctrl)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;
	else {
		pr_err("fail to new custom v4l2loopback_ctrl_buffernum \n");
		goto error;
	}

	ctrl = v4l2_ctrl_new_custom(hdl, &v4l2loopback_ctrl_datasize, NULL);
	if (ctrl)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;
	else {
		pr_err("fail to new custom v4l2loopback_ctrl_datasize \n");
		goto error;
	}

	if (hdl->error) {
		pr_err("ctrl handler error\n");
		ret = hdl->error;
		goto error;
	}
	dev->v4l2_dev.ctrl_handler = hdl;

	/* FIXME set buffers to 0 */

	/* Set initial format */
	dev->pix_format.width = 0; /* V4L2LOOPBACK_SIZE_DEFAULT_WIDTH; */
	dev->pix_format.height = 0; /* V4L2LOOPBACK_SIZE_DEFAULT_HEIGHT; */
	dev->pix_format.pixelformat = formats[0].fourcc;
	dev->pix_format.colorspace =
		V4L2_COLORSPACE_SRGB; /* do we need to set this ? */
	dev->pix_format.field = V4L2_FIELD_NONE;

	dev->buffer_size = PAGE_ALIGN(dev->pix_format.sizeimage);

	dev->io_mode = V4L2L_IO_MODE_DMABUF;

	pr_debug("buffer_size = %ld (=%d)\n",
		dev->buffer_size, dev->pix_format.sizeimage);
	allocate_buffers(dev);

	init_waitqueue_head(&dev->read_event);
	init_completion(&dev->gparam_complete);
	init_completion(&dev->sparam_complete);
	init_completion(&dev->ctrl_complete);
	init_completion(&dev->allocbufs_complete);

	mutex_init(&dev->buf_mutex);
	mutex_init(&dev->capbufs_mutex);
	mutex_init(&dev->outbufs_mutex);
	ret = v4l2_loopback_cropcap(dev, &cropcap);
	if (ret) {
		pr_err("fail to v4l2_loopback_cropcap \n");
		goto error;
	}

	crop.type = cropcap.type;
	crop.c = cropcap.defrect;
	ret = v4l2_loopback_s_crop(dev, &crop, false);
	if (ret) {
		pr_err("fail to v4l2_loopback_s_crop \n");
		goto error;
	}

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
	.vidioc_enum_frameintervals = &vidioc_enum_frameintervals,

	.vidioc_enum_output       = &vidioc_enum_output,
	.vidioc_g_output          = &vidioc_g_output,
	.vidioc_s_output          = &vidioc_s_output,

	.vidioc_enum_input       = &vidioc_enum_input,
	.vidioc_g_input          = &vidioc_g_input,
	.vidioc_s_input          = &vidioc_s_input,

	.vidioc_enum_fmt_vid_cap = &vidioc_enum_fmt_cap,
	.vidioc_g_fmt_vid_cap    = &vidioc_g_fmt_cap,
	.vidioc_s_fmt_vid_cap    = &vidioc_s_fmt_cap,
	.vidioc_try_fmt_vid_cap  = &vidioc_try_fmt_cap,

	.vidioc_enum_fmt_vid_out = &vidioc_enum_fmt_out,
	.vidioc_s_fmt_vid_out    = &vidioc_s_fmt_out,
	.vidioc_g_fmt_vid_out    = &vidioc_g_fmt_out,
	.vidioc_try_fmt_vid_out  = &vidioc_try_fmt_out,

#ifdef V4L2L_OVERLAY
	.vidioc_s_fmt_vid_overlay = &vidioc_s_fmt_overlay,
	.vidioc_g_fmt_vid_overlay = &vidioc_g_fmt_overlay,
#endif

#ifdef V4L2LOOPBACK_WITH_STD
	.vidioc_s_std            = &vidioc_s_std,
	.vidioc_g_std            = &vidioc_g_std,
	.vidioc_querystd         = &vidioc_querystd,
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
			free_buffers(devs[i]);
			mutex_destroy(&devs[i]->buf_mutex);
			mutex_destroy(&devs[i]->capbufs_mutex);
			mutex_destroy(&devs[i]->outbufs_mutex);
			v4l2loopback_remove_sysfs(devs[i]->vdev);
			kfree(video_get_drvdata(devs[i]->vdev));
			video_unregister_device(devs[i]->vdev);
			v4l2_device_unregister(&devs[i]->v4l2_dev);
			v4l2_ctrl_handler_free(&devs[i]->ctrl_handler);
			kfree(devs[i]);
			devs[i] = NULL;
		}
	}
}

static int __init v4l2loopback_init_module(void)
{
	int ret;
	int i;
	int video_idx = 51;

	MARK();
	zero_devices();

	devices = 8;
	for (i = 0; i < devices; ++i) {
		video_nr[i] = video_idx + i;
	}

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
		pr_debug(
				"v4l2loopback: number of devices is limited to: %d\n"
				, MAX_DEVICES);
	}

	if (max_buffers > MAX_BUFFERS) {
		max_buffers = MAX_BUFFERS;
		pr_debug(
				"v4l2loopback: number of buffers is limited to: %d\n"
				, MAX_BUFFERS);
	}

	if (max_openers < 0) {
		pr_debug(
				"v4l2loopback: allowing %d openers rather than %d\n"
				, 2, max_openers);
		max_openers = 2;
	}

	if (max_width < 1) {
		max_width = V4L2LOOPBACK_SIZE_MAX_WIDTH;
		pr_debug("v4l2loopback: using max_width %d\n", max_width);
	}
	if (max_height < 1) {
		max_height = V4L2LOOPBACK_SIZE_MAX_HEIGHT;
		pr_debug("v4l2loopback: using max_height %d\n", max_height);
	}

	/* kfree on module release */
	for (i = 0; i < devices; i++) {
		pr_debug("creating v4l2loopback-device #%d\n", i);
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
					VFL_TYPE_GRABBER, video_nr[i]) < 0) {
			video_device_release(devs[i]->vdev);
			pr_err(
			"v4l2loopback: failed video_register_device()\n");
			free_devices();
			return -EFAULT;
		}
		v4l2loopback_create_sysfs(devs[i]->vdev);
	}

	pr_info("module installed\n");

	pr_info("v4l2loopback driver version %d.%d.%d loaded\n",
			(V4L2LOOPBACK_VERSION_CODE >> 16) & 0xff,
			(V4L2LOOPBACK_VERSION_CODE >>  8) & 0xff,
			(V4L2LOOPBACK_VERSION_CODE) & 0xff);

	return 0;
}

static void v4l2loopback_cleanup_module(void)
{
	MARK();
	/* unregister the device -> it deletes /dev/video* */
	free_devices();
	pr_info("module removed\n");
}

late_initcall(v4l2loopback_init_module);
module_exit(v4l2loopback_cleanup_module);
MODULE_DESCRIPTION("AIS V4L2LOOPBACK driver");
MODULE_LICENSE("GPL v2");

