#include <stdio.h>
#include <errno.h>
#include <string.h> // strerror

#include <sys/mman.h> // mmap

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

static int v4l2_set_format(DeviceV4L2* dev, uint32_t type, int w, int h) {
	int status = 0;
	LOGI("Setting format for type=%s(%d)", v4l2BufTypeName(type), type);

	dev->fmt.type = type;
	if (0 != ioctl(dev->fd, VIDIOC_G_FMT, &dev->fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_G_FMT): %d, %s", dev->fd, errno, strerror(errno));
		status = 1;
		goto tail;
	}

	v4l2PrintFormat(&dev->fmt);

	if (w == 0 || h == 0) {
		return 0;
	}

	LOGI("Seting format to %dx%d", w, h);
	dev->fmt.fmt.pix_mp.width = w;
	dev->fmt.fmt.pix_mp.height = h;

	if (0 != ioctl(dev->fd, VIDIOC_S_FMT, &dev->fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_S_FMT): %d, %s", dev->fd, errno, strerror(errno));
		status = 1;
		goto tail;
	}

tail:
	return status;
}

static int v4l2_init_buffers_mmap(DeviceV4L2 *dev, const struct v4l2_requestbuffers* req) {
	dev->buffers = calloc(req->count, sizeof(*dev->buffers));
	dev->buffers_count = req->count;

	for (int i = 0; i < dev->buffers_count; ++i) {
		Buffer *const buf = dev->buffers + i;
		buf->buffer = (struct v4l2_buffer){
			.type = req->type,
			.memory = req->memory,
			.index = i,
		};

		if (0 != ioctl(dev->fd, VIDIOC_QUERYBUF, &buf->buffer)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QUERYBUF, [%d]): %d, %s", dev->fd, i, errno, strerror(errno));
			goto fail;
		}

		LOGI("Queried buffer %d:", i);
		v4l2PrintBuffer(&buf->buffer);

		buf->v.mmap.ptr = mmap(NULL, buf->buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, buf->buffer.m.offset);
		if (buf->v.mmap.ptr == MAP_FAILED) {
			LOGE("Failed to mmap(%d, buffer[%d]): %d, %s", dev->fd, i, errno, strerror(errno));
			goto fail;
		}
	}

	return 0;

fail:
	// TODO remove ones we've already queried?
	free(dev->buffers);
	dev->buffers = NULL;
	dev->buffers_count = 0;
	return 1;
}

static int v4l2_requestbuffers(DeviceV4L2* dev, uint32_t type, uint32_t count, uint32_t memory_type) {
	struct v4l2_requestbuffers req = {0};
	req.type = type;
	req.count = count;
	req.memory = memory_type;
	if (0 != ioctl(dev->fd, VIDIOC_REQBUFS, &req)) {
		LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", dev->fd, errno, strerror(errno));
		return -1;
	}

	v4l2PrintRequestBuffers(&req);

	switch (req.memory) {
		case V4L2_MEMORY_MMAP:
			return v4l2_init_buffers_mmap(dev, &req);
			break;
		default:
			LOGE("%s: Unimplemented memory type %s (%d)", __func__, v4l2MemoryTypeName(req.memory), (int)req.memory);
			return 1;
			break;
	}

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

	LOGI("Opened \"%s\" => %d", devname, dev.fd);

	if (0 != v4l2QueryCapability(&dev)) {
		goto fail;
	}

	v4l2_enum_input(dev.fd);
	v4l2_enum_controls_ext(dev.fd);

	// TODO if capture
	// TODO how to enumerate?
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

	// TODO if output
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_OUTPUT);
	v4l2_enum_formats(dev.fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);

	// TODO VIDIOC_ENUM_FRAMESIZES
	// TODO VIDIOC_ENUM_FRAMEINTERVALS

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

	// TODO free buffers

	close(dev->fd);
	free(dev);
}

int devV4L2Start(struct DeviceV4L2 *dev, const V4L2PrepareOpts *opts) {
	if (0 != v4l2_set_format(dev, opts->buffer_type, opts->width, opts->height)) {
		return 1;
	}

	if (0 != v4l2_requestbuffers(dev, opts->buffer_type, opts->buffers_count, opts->memory_type)) {
		return 1;
	}

	// Enqueue buffers for capture
	for (int i = 0; i < dev->buffers_count; ++i) {
		struct v4l2_buffer buf = {
			.type = opts->buffer_type,
			.memory = opts->memory_type,
			.index = i,
		};

		if (0 != ioctl(dev->fd, VIDIOC_QBUF, &buf)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QBUF, %i): %d, %s",
				dev->fd, i, errno, strerror(errno));
			return -1;
		}
	}

	if (0 != ioctl(dev->fd, VIDIOC_STREAMON, &opts->buffer_type)) {
		LOGE("Failed to ioctl(%d, VIDIOC_STREAMON, %s): %d, %s",
			dev->fd, v4l2BufTypeName(opts->buffer_type), errno, strerror(errno));
		return 1;
	}
	
	return 0;
}

//void devV4L2Stop(struct DeviceV4L2 *dev) {}

const Buffer *devV4L2PullBuffer(struct DeviceV4L2 *dev) {
	// TODO pick the right stream
	struct v4l2_buffer buf = {
		.type = dev->buffers[0].buffer.type,
		.memory = dev->buffers[0].buffer.memory,
	};

	if (0 != ioctl(dev->fd, VIDIOC_DQBUF, &buf)) {
		LOGE("Failed to ioctl(%d, VIDIOC_DQBUF): %d, %s",
			dev->fd, errno, strerror(errno));
		return NULL;
	}

	Buffer *const ret = dev->buffers + buf.index;
	// TODO check differences
	ret->buffer = buf;
	return ret;
}

int devV4L2PushBuffer(struct DeviceV4L2 *dev, const Buffer *buf) {
	if (0 != ioctl(dev->fd, VIDIOC_QBUF, &buf->buffer)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QBUF): %d, %s",
			dev->fd, errno, strerror(errno));
		return errno;
	}

	return 0;
}
