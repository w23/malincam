#include <linux/videodev2.h>

#include <stdio.h>
#include <errno.h>
#include <string.h> // strerror

// open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h> // close
#include <sys/ioctl.h> // ioctl
#include <stdint.h> // uint32_t et al.

#include <stdlib.h> // malloc

#include "common.h"

typedef struct DeviceV4L2 {
	int fd;
	char *name;

	struct v4l2_capability caps;
} DeviceV4L2;

#define MCAM_V4L2_CAPS(X) \
	X(V4L2_CAP_VIDEO_CAPTURE) \
	X(V4L2_CAP_VIDEO_OUTPUT) \
	X(V4L2_CAP_VIDEO_OVERLAY) \
	X(V4L2_CAP_VBI_CAPTURE) \
	X(V4L2_CAP_VBI_OUTPUT) \
	X(V4L2_CAP_SLICED_VBI_CAPTURE) \
	X(V4L2_CAP_SLICED_VBI_OUTPUT) \
	X(V4L2_CAP_RDS_CAPTURE) \
	X(V4L2_CAP_VIDEO_OUTPUT_OVERLAY) \
	X(V4L2_CAP_HW_FREQ_SEEK) \
	X(V4L2_CAP_RDS_OUTPUT) \
	X(V4L2_CAP_VIDEO_CAPTURE_MPLANE) \
	X(V4L2_CAP_VIDEO_OUTPUT_MPLANE) \
	X(V4L2_CAP_VIDEO_M2M_MPLANE) \
	X(V4L2_CAP_VIDEO_M2M) \
	X(V4L2_CAP_TUNER) \
	X(V4L2_CAP_AUDIO) \
	X(V4L2_CAP_RADIO) \
	X(V4L2_CAP_MODULATOR) \
	X(V4L2_CAP_SDR_CAPTURE) \
	X(V4L2_CAP_EXT_PIX_FORMAT) \
	X(V4L2_CAP_SDR_OUTPUT) \
	X(V4L2_CAP_META_CAPTURE) \
	X(V4L2_CAP_READWRITE) \
	X(V4L2_CAP_STREAMING) \
	X(V4L2_CAP_META_OUTPUT) \
	X(V4L2_CAP_TOUCH) \
	X(V4L2_CAP_IO_MC) \
	X(V4L2_CAP_DEVICE_CAPS) \

static void v4l2_print_caps(uint32_t caps) {
#define X(cap) if (caps & cap) LOGI("  %s", #cap);
	MCAM_V4L2_CAPS(X)
#undef X
}

#define MCAM_V4L2_BUF_CAPS(X) \
	X(V4L2_BUF_CAP_SUPPORTS_MMAP) \
	X(V4L2_BUF_CAP_SUPPORTS_USERPTR) \
	X(V4L2_BUF_CAP_SUPPORTS_DMABUF) \
	X(V4L2_BUF_CAP_SUPPORTS_REQUESTS) \
	X(V4L2_BUF_CAP_SUPPORTS_ORPHANED_BUFS) \
	X(V4L2_BUF_CAP_SUPPORTS_M2M_HOLD_CAPTURE_BUF) \
	X(V4L2_BUF_CAP_SUPPORTS_MMAP_CACHE_HINTS) \

static void v4l2_print_buf_caps(uint32_t caps) {
#define X(cap) if (caps & cap) LOGI("  %s", #cap);
	MCAM_V4L2_BUF_CAPS(X)
#undef X
}

static void v4l2PrintCapability(const struct v4l2_capability* caps) {
	LOGI("caps->driver = %s", caps->driver);
	LOGI("caps->card = %s", caps->card);
	LOGI("caps->bus_info = %s", caps->bus_info);
#define FROM_KERNEL_VERSION(v) (((v)>>16)&0xff), (((v)>>8)&0xff), ((v)&0xff)
	LOGI("caps->version = %d.%d.%d", FROM_KERNEL_VERSION(caps->version));
	LOGI("caps->capabilities = %08x:", caps->capabilities);
	v4l2_print_caps(caps->capabilities);
	
	if (caps->capabilities & V4L2_CAP_DEVICE_CAPS) {
		LOGI("caps->device_caps = %08x:", caps->device_caps);
		v4l2_print_caps(caps->device_caps);
	}
}

static const char *v4l2_input_type_name(uint32_t type) {
	switch (type) {
		case V4L2_INPUT_TYPE_TUNER: return "V4L2_INPUT_TYPE_TUNER";
		case V4L2_INPUT_TYPE_CAMERA: return "V4L2_INPUT_TYPE_CAMERA";
		case V4L2_INPUT_TYPE_TOUCH: return "V4L2_INPUT_TYPE_TOUCH";
		default: return "UNKNOWN";
	}
}

static void v4l2PrintInput(const struct v4l2_input* input) {
	LOGI("  input->index = %d", input->index);
	LOGI("  input->name = %s", input->name);
	LOGI("  input->type = %s (%d)", v4l2_input_type_name(input->type), input->type);
	LOGI("  input->audioset = %08x", input->audioset);
	LOGI("  input->tuner = %x", input->tuner);
	LOGI("  input->std = %llx", (long long)input->std); // TODO
	LOGI("  input->status = %d", input->status); // TODO
	LOGI("  input->capabilities = %08x", input->capabilities); // TODO
}

static int v4l2QueryCapability(DeviceV4L2 *dev) {
	if (0 != ioctl(dev->fd, VIDIOC_QUERYCAP, &dev->caps)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QUERYCAP): %d, %s", dev->fd, errno, strerror(errno));
		return 1;
	}

	v4l2PrintCapability(&dev->caps);
	return 0;
}

static int v4l2_enum_input(int fd) {
	for (int i = 0;; ++i) {
		struct v4l2_input input;
		input.index = i;
		if (0 != ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
			if (EINVAL == errno) {
				LOGI("Device has %d inputs", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("Device has no inputs");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_ENUMINPUT): %d, %s", fd, errno, strerror(errno));
			return 1;
		}

		v4l2PrintInput(&input);
	}
}

static int v4l2_enum_controls_ext(int fd) {
	struct v4l2_queryctrl qctrl;
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl(fd, VIDIOC_QUERYCTRL, &qctrl)) {
		LOGI("qctrl.name = %s", qctrl.name);
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	return 0;
}

static const char *v4l2_buf_type_name(uint32_t type) {
	switch (type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE: return "V4L2_BUF_TYPE_VIDEO_CAPTURE";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT: return "V4L2_BUF_TYPE_VIDEO_OUTPUT";
		case V4L2_BUF_TYPE_VIDEO_OVERLAY: return "V4L2_BUF_TYPE_VIDEO_OVERLAY";
		case V4L2_BUF_TYPE_VBI_CAPTURE: return "V4L2_BUF_TYPE_VBI_CAPTURE";
		case V4L2_BUF_TYPE_VBI_OUTPUT: return "V4L2_BUF_TYPE_VBI_OUTPUT";
		case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE: return "V4L2_BUF_TYPE_SLICED_VBI_CAPTURE";
		case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT: return "V4L2_BUF_TYPE_SLICED_VBI_OUTPUT";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY: return "V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY";
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE: return "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: return "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE";
		case V4L2_BUF_TYPE_SDR_CAPTURE: return "V4L2_BUF_TYPE_SDR_CAPTURE";
		case V4L2_BUF_TYPE_SDR_OUTPUT: return "V4L2_BUF_TYPE_SDR_OUTPUT";
		case V4L2_BUF_TYPE_META_CAPTURE: return "V4L2_BUF_TYPE_META_CAPTURE";
		case V4L2_BUF_TYPE_META_OUTPUT: return "V4L2_BUF_TYPE_META_OUTPUT";
		default: return "UNKNOWN";
	}
}

static int v4l2_enum_formats(int fd, uint32_t type) {
	LOGI("Enumerating formats for type=%s(%d)", v4l2_buf_type_name(type), type);
	for (int i = 0;; ++i) {
		struct v4l2_fmtdesc fmt;
		fmt.index = i;
		fmt.mbus_code = 0;
		fmt.type = type;
		if (0 != ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {
			if (EINVAL == errno) {
				LOGI("Device has %d formats", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("Device has no formats");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_ENUM_FMT): %d, %s", fd, errno, strerror(errno));
			return 1;
		}

		LOGI("fmt[%d].flags = %08x", i, fmt.flags);
		LOGI("fmt[%d].description = %s", i, fmt.description);
		LOGI("fmt[%d].pixelformat = %08x", i, fmt.pixelformat);
	}
}

static int v4l2_set_format(int fd, uint32_t type, int w, int h) {
#if 0
	ioctl(40, VIDIOC_G_FMT, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, fmt.pix_mp={width=32,   height=32,   pixelformat=v4l2_fourcc('J', 'P', 'E', 'G') /* V4L2_PIX_FMT_JPEG */, field=V4L2_FIELD_NONE, colorspace=V4L2_COLORSPACE_JPEG, plane_fmt=[{sizeimage=4194304, bytesperline=0}], num_planes=1}}) = 0
	ioctl(40, VIDIOC_S_FMT, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, fmt.pix_mp={width=1920, height=1056, pixelformat=v4l2_fourcc('J', 'P', 'E', 'G') /* V4L2_PIX_FMT_JPEG */, field=V4L2_FIELD_ANY,  colorspace=V4L2_COLORSPACE_JPEG, plane_fmt=[{sizeimage=0, bytesperline=0}],       num_planes=1}} =>
	                                                                 {fmt.pix_mp={width=1920, height=1056, pixelformat=v4l2_fourcc('J', 'P', 'E', 'G') /* V4L2_PIX_FMT_JPEG */, field=V4L2_FIELD_NONE, colorspace=V4L2_COLORSPACE_JPEG, plane_fmt=[{sizeimage=4194304, bytesperline=0}], num_planes=1}}) = 0
#endif
	int status = 0;
	LOGI("Setting format for type=%s(%d)", v4l2_buf_type_name(type), type);

	struct v4l2_format fmt;
	fmt.type = type;
	if (0 != ioctl(fd, VIDIOC_G_FMT, &fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_G_FMT): %d, %s", fd, errno, strerror(errno));
		status = 1;
		goto tail;
	}

#define _(v) (v)
#define FIELDS(X) \
	X(width, "%u", _) \
	X(height, "%u", _) \
	X(field, "%08x", _) \
	X(colorspace, "%08x", _) \
	X(num_planes, "%08x", _) \
	X(flags, "%08x", _) \
	X(quantization, "%08x", _) \
	X(xfer_func, "%08x", _) \

#define X(name, ffmt, func) \
	LOGI("fmt.pix_mp." # name " = " ffmt, func(fmt.fmt.pix_mp.name));
	FIELDS(X)
#undef X
#undef _

	for (int i = 0; i < fmt.fmt.pix_mp.num_planes; ++i) {
		LOGI("fmt.pix_mp.plane_fmt[%d].sizeimage = %d", i, fmt.fmt.pix_mp.plane_fmt[i].sizeimage);
		LOGI("fmt.pix_mp.plane_fmt[%d].bytesperline = %d", i, fmt.fmt.pix_mp.plane_fmt[i].bytesperline);
	}

	fmt.fmt.pix_mp.width = w;
	fmt.fmt.pix_mp.height = h;

	if (0 != ioctl(fd, VIDIOC_S_FMT, &fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_S_FMT): %d, %s", fd, errno, strerror(errno));
		status = 1;
		goto tail;
	}

	LOGI("Set format to %dx%d", w, h);

tail:
	return status;
}

static int v4l2_requestbuffers(int fd, uint32_t type) {
#if 0
	ioctl(40, VIDIOC_REQBUFS, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, memory=V4L2_MEMORY_MMAP, count=3 => 3}) = 0
	write(2, "device/buffer_list.c: SNAPSHOT:c"..., 114) = 114
	ioctl(40, VIDIOC_QUERYBUF_TIME32, 0x7ea46f70) = 0
	mmap2(NULL, 4194304, PROT_READ|PROT_WRITE, MAP_SHARED, 40, 0x40000000) = 0x662a1000
	ioctl(40, VIDIOC_EXPBUF, 0x7ea46ef4)    = 0
	ioctl(40, VIDIOC_QUERYBUF_TIME32, 0x7ea46f70) = 0
	mmap2(NULL, 4194304, PROT_READ|PROT_WRITE, MAP_SHARED, 40, 0x40400000) = 0x65ea1000
	ioctl(40, VIDIOC_EXPBUF, 0x7ea46ef4)    = 0
	ioctl(40, VIDIOC_QUERYBUF_TIME32, 0x7ea46f70) = 0
	mmap2(NULL, 4194304, PROT_READ|PROT_WRITE, MAP_SHARED, 40, 0x40800000) = 0x65aa1000
	ioctl(40, VIDIOC_EXPBUF, 0x7ea46ef4)    = 0
#endif

	struct v4l2_requestbuffers req = {0};
	req.type = type;
	req.count = 2;
	req.memory = V4L2_MEMORY_MMAP;
	if (0 != ioctl(fd, VIDIOC_REQBUFS, &req)) {
		LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", fd, errno, strerror(errno));
		return -1;
	}

	LOGI("req.count = %d", req.count);
	v4l2_print_buf_caps(req.capabilities);
	LOGI("req.flags = %08x", req.flags);

	return 0;
}

struct DeviceV4L2* devV4L2Open(const char *devname) {
	DeviceV4L2 dev = {0};
	LOGI("Opening %s...", devname);
	dev.fd = open(devname, O_RDWR | O_NONBLOCK);
	if (dev.fd < 0) {
		LOGE("Failed to open \"%s\": %d, %s", devname, errno, strerror(errno));
		goto fail;
	}

	LOGI("Opened \"%s\" = %d", devname, dev.fd);

	if (0 != v4l2QueryCapability(&dev)) {
		goto fail;
	}

	v4l2_enum_input(dev.fd);
	v4l2_enum_controls_ext(dev.fd);
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_OUTPUT);
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);

	// TODO VIDIOC_ENUM_FRAMESIZES
	// TODO VIDIOC_ENUM_FRAMEINTERVALS

	v4l2_set_format(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, 1280, 720);
	v4l2_requestbuffers(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);

	//v4l2_set_format(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, 1280, 720);
	//v4l2_requestbuffers(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

	DeviceV4L2* const ret = (DeviceV4L2*)malloc(sizeof(DeviceV4L2));
	*ret = dev;
	return ret;

fail:
	if (dev.fd > 0)
		close(dev.fd);
	return NULL;
}

void devV4L2Close(struct DeviceV4L2* dev) {
	if (!dev)
		return;

	close(dev->fd);
	free(dev);
}
