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
#include "v4l2.h"

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

		LOGI("Input %d:", i);
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

static int v4l2_enum_formats(int fd, uint32_t type) {
	LOGI("Enumerating formats for type=%s(%d)", v4l2BufTypeName(type), type);
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

		v4l2PrintFormatDesc(&fmt);
	}
}

static int v4l2_set_format(int fd, uint32_t type, int w, int h) {
#if 0
	ioctl(40, VIDIOC_G_FMT, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, fmt.pix_mp={width=32,   height=32,   pixelformat=v4l2_fourcc('J', 'P', 'E', 'G') /* V4L2_PIX_FMT_JPEG */, field=V4L2_FIELD_NONE, colorspace=V4L2_COLORSPACE_JPEG, plane_fmt=[{sizeimage=4194304, bytesperline=0}], num_planes=1}}) = 0
	ioctl(40, VIDIOC_S_FMT, {type=V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, fmt.pix_mp={width=1920, height=1056, pixelformat=v4l2_fourcc('J', 'P', 'E', 'G') /* V4L2_PIX_FMT_JPEG */, field=V4L2_FIELD_ANY,  colorspace=V4L2_COLORSPACE_JPEG, plane_fmt=[{sizeimage=0, bytesperline=0}],       num_planes=1}} =>
	                                                                 {fmt.pix_mp={width=1920, height=1056, pixelformat=v4l2_fourcc('J', 'P', 'E', 'G') /* V4L2_PIX_FMT_JPEG */, field=V4L2_FIELD_NONE, colorspace=V4L2_COLORSPACE_JPEG, plane_fmt=[{sizeimage=4194304, bytesperline=0}], num_planes=1}}) = 0
#endif
	int status = 0;
	LOGI("Setting format for type=%s(%d)", v4l2BufTypeName(type), type);

	struct v4l2_format fmt;
	fmt.type = type;
	if (0 != ioctl(fd, VIDIOC_G_FMT, &fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_G_FMT): %d, %s", fd, errno, strerror(errno));
		status = 1;
		goto tail;
	}

	v4l2PrintFormat(&fmt);

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

	v4l2PrintRequestBuffers(&req);

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
