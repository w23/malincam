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

static int v4l2EnumFormatsForBufferType(Endpoint *point, int fd, uint32_t type, int mbus_code) {
	arrayInit(&point->formats, struct v4l2_fmtdesc);
	LOGI("Enumerating formats for type=%s(%d)", v4l2BufTypeName(type), type);
	for (int i = 0;; ++i) {
		struct v4l2_fmtdesc fmt;
		fmt.index = i;
		fmt.type = type;
		fmt.mbus_code = mbus_code;
		if (0 != ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {
			if (EINVAL == errno) {
				LOGI("Endpoint has %d formats", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("Endpoint has no formats");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_ENUM_FMT): %d, %s", fd, errno, strerror(errno));
			return errno;
		}

		//v4l2PrintFormatDesc(&fmt);
		LOGI("  fmt[%d] = {%s, %s}", i, v4l2PixFmtName(fmt.pixelformat), fmt.description);
		v4l2PrintFormatFlags(fmt.flags);

		// Enumerate possible sizes
		for (int i = 0;; ++i) {
			struct v4l2_frmsizeenum fse = { .index = i, .pixel_format = fmt.pixelformat };
			if (0 != ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fse)) {
				if (EINVAL == errno) {
					LOGI("  Format has %d framesizes", i);
					break;
				}
			}

			v4l2PrintFrmSizeEnum(&fse);

			// Only discrete supports index > 0
			if (fse.type != V4L2_FRMSIZE_TYPE_DISCRETE)
				break;
		}

		arrayAppend(&point->formats, &fmt);
	}
}

static int endpointGetFormat(Endpoint *ep, int fd) {
		ep->format = (struct v4l2_format){0};
		ep->format.type = ep->type;
		if (0 != ioctl(fd, VIDIOC_G_FMT, &ep->format)) {
			LOGE("Failed to ioctl(%d, VIDIOC_G_FMT, %s): %d, %s",
				fd, v4l2BufTypeName(ep->type), errno, strerror(errno));
			return -1;
		}

		return 0;
}

static int v4l2AddEndpoint(DeviceV4L2 *dev, uint32_t buffer_type) {
	Endpoint point;
	point.type = buffer_type;

	if (0 != v4l2EnumFormatsForBufferType(&point, dev->fd, buffer_type, 0))
		goto fail;

	// Read supported memory types
	{
		struct v4l2_requestbuffers req = {0};
		req.type = buffer_type;
		req.count = 0;
		req.memory = V4L2_MEMORY_MMAP;
		if (0 != ioctl(dev->fd, VIDIOC_REQBUFS, &req)) {
			LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", dev->fd, errno, strerror(errno));
			goto fail;
		}

		LOGI("Endpoint capabilities:");
		v4l2PrintBufferCapabilityBits(req.capabilities);
		point.buffer_capabilities = req.capabilities;
	}

	// Read current format
	if (0 != endpointGetFormat(&point, dev->fd)) {
			goto fail;
		v4l2PrintFormat(&point.format);
	}

	arrayAppend(&dev->endpoints, &point);
	return 0;

fail:
	arrayDestroy(&point.formats);
	return -1;
}

static int v4l2QueryCapability(DeviceV4L2 *dev) {
	if (0 != ioctl(dev->fd, VIDIOC_QUERYCAP, &dev->caps)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QUERYCAP): %d, %s", dev->fd, errno, strerror(errno));
		return errno;
	}

	v4l2PrintCapability(&dev->caps);

	dev->this_device_caps = dev->caps.capabilities & V4L2_CAP_DEVICE_CAPS
		? dev->caps.device_caps : dev->caps.capabilities;

	arrayInit(&dev->endpoints, Endpoint);

	if ((V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M) & dev->this_device_caps) {
		if (0 != v4l2AddEndpoint(dev, V4L2_BUF_TYPE_VIDEO_CAPTURE))
			goto fail;
	}

	if ((V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE) & dev->this_device_caps) {
		if (0 != v4l2AddEndpoint(dev, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE))
			goto fail;
	}

	if ((V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M) & dev->this_device_caps) {
		if (0 != v4l2AddEndpoint(dev, V4L2_BUF_TYPE_VIDEO_OUTPUT))
			goto fail;
	}

	if ((V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE) & dev->this_device_caps) {
		if (0 != v4l2AddEndpoint(dev, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE))
			goto fail;
	}

	return 0;

fail:
	// FIXME destroy endpoints
	return -1;
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

static int endpontSetFormat(Endpoint *ep, int fd, uint32_t pixelformat, int w, int h) {
	LOGI("Setting format for endpoint=%s(%d)", v4l2BufTypeName(ep->type), ep->type);

	if (0 != endpointGetFormat(ep, fd))
		return -1;

	if (w == 0 && h == 0 && pixelformat == 0) {
		return 0;
	}

	LOGE("%s is not implemented == need bytesperline, sizeimage, mplanes compute code", __func__);
	return 0;

#if 0
	// TODO
	LOGI("Seting format to %s %dx%d", v4l2PixFmtName(pixelformat), w, h);
	switch (ep->type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
			ep->format.fmt.pix_mp.width = w;
			ep->format.fmt.pix_mp.height = h;
			ep->format.fmt.pix_mp.pixelformat = pixelformat;
			// FIXME bytesperline
			// TODO mplanes?
			break;
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			ep->format.fmt.pix.width = w;
			ep->format.fmt.pix.height = h;
			ep->format.fmt.pix.pixelformat = pixelformat;
			break;
	}


	if (0 != ioctl(dev->fd, VIDIOC_S_FMT, &dev->fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_S_FMT): %d, %s", dev->fd, errno, strerror(errno));
		status = 1;
		goto tail;
	}

tail:
	return status;
#endif
}

static int endpointQueryBuffersMmap(Endpoint *ep, int fd, const struct v4l2_requestbuffers* req) {
	ep->buffers = calloc(req->count, sizeof(*ep->buffers));
	ep->buffers_count = req->count;

	for (int i = 0; i < ep->buffers_count; ++i) {
		Buffer *const buf = ep->buffers + i;
		buf->buffer = (struct v4l2_buffer){
			.type = req->type,
			.memory = req->memory,
			.index = i,
		};

		if (0 != ioctl(fd, VIDIOC_QUERYBUF, &buf->buffer)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QUERYBUF, [%d]): %d, %s", fd, i, errno, strerror(errno));
			goto fail;
		}

		LOGI("Queried buffer %d:", i);
		v4l2PrintBuffer(&buf->buffer);

		buf->v.mmap.ptr = mmap(NULL, buf->buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf->buffer.m.offset);
		if (buf->v.mmap.ptr == MAP_FAILED) {
			LOGE("Failed to mmap(%d, buffer[%d]): %d, %s", fd, i, errno, strerror(errno));
			goto fail;
		}
	}

	return 0;

fail:
	// FIXME remove ones we've already queried?
	free(ep->buffers);
	ep->buffers = NULL;
	ep->buffers_count = 0;
	return 1;
}

static int endpointRequestBuffers(Endpoint *ep, int fd, uint32_t count, uint32_t memory_type) {
	// TODO check if anything changed that require buffer request?
	// - frame size, pixelformat, etc

	struct v4l2_requestbuffers req = {0};
	req.type = ep->type;
	req.count = count;
	req.memory = memory_type;
	if (0 != ioctl(fd, VIDIOC_REQBUFS, &req)) {
		LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", fd, errno, strerror(errno));
		return -1;
	}

	v4l2PrintRequestBuffers(&req);

	switch (req.memory) {
		case V4L2_MEMORY_MMAP:
			return endpointQueryBuffersMmap(ep, fd, &req);
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

	// TODO VIDIOC_ENUM_FRAMEINTERVALS
	// Depends on selected resolution. So cannot really query before the resolution is picked.

	// TODO set frameinterval, use VIDIOC_S_PARM

	DeviceV4L2* const ret = (DeviceV4L2*)malloc(sizeof(DeviceV4L2));
	*ret = dev;
	return ret;

fail:
	if (dev.fd > 0)
		close(dev.fd);

	return NULL;
}

static void endpointDestroy(Endpoint *ep) {
	for (int i = 0; i < ep->buffers_count; ++i) {
		Buffer *const buf = ep->buffers + i;
		switch (buf->buffer.memory) {
			case V4L2_MEMORY_MMAP:
				if (buf->v.mmap.ptr && 0 != munmap(buf->v.mmap.ptr, buf->buffer.length)) {
					LOGE("munmap(%p) => %s (%d)", buf->v.mmap.ptr, strerror(errno), errno);
				}
				break;
		}
	}

	if (ep->buffers)
		free(ep->buffers);

	arrayDestroy(&ep->formats);
}

void devV4L2Close(struct DeviceV4L2* dev) {
	if (!dev)
		return;

	for (int i = 0; i < dev->endpoints.size; ++i)
		endpointDestroy(arrayAt(&dev->endpoints, Endpoint, i));
	arrayDestroy(&dev->endpoints);

	close(dev->fd);
	free(dev);
}

static int endpointEnqueueBuffers(Endpoint *ep, int fd) {
	for (int i = 0; i < ep->buffers_count; ++i) {
		struct v4l2_buffer buf = {
			.type = ep->type,
			.memory = ep->buffers[0].buffer.memory, // TODO should it be set per-endpoint globally?
			.index = i,
		};

		if (0 != ioctl(fd, VIDIOC_QBUF, &buf)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QBUF, %i): %d, %s",
				fd, i, errno, strerror(errno));
			return -1;
		}
	}

	return 0;
}

int devV4L2EndpointStart(struct DeviceV4L2 *dev, int endpoint_index, const V4L2PrepareOpts *opts) {
	Endpoint *const ep = arrayAt(&dev->endpoints, Endpoint, endpoint_index);

	if (0 != endpontSetFormat(ep, dev->fd, opts->pixelformat, opts->width, opts->height)) {
		return -1;
	}

	if (0 != endpointRequestBuffers(ep, dev->fd, opts->buffers_count, opts->memory_type)) {
		return -2;
	}

	if (0 != endpointEnqueueBuffers(ep, dev->fd)) {
		return -3;
	}

	if (0 != ioctl(dev->fd, VIDIOC_STREAMON, &ep->type)) {
		LOGE("Failed to ioctl(%d, VIDIOC_STREAMON, %s): %d, %s",
			dev->fd, v4l2BufTypeName(ep->type), errno, strerror(errno));
		return 1;
	}
	
	ep->state = ENDPOINT_STATE_STREAMING;
	return 0;
}

int devV4L2EndpointStop(struct DeviceV4L2 *dev, int endpoint_index) {
	Endpoint *const ep = arrayAt(&dev->endpoints, Endpoint, endpoint_index);

	// TODO check whether all buffers are done?

	if (0 != ioctl(dev->fd, VIDIOC_STREAMOFF, &ep->type)) {
		LOGE("Failed to ioctl(%d, VIDIOC_STREAMON, %s): %d, %s",
			dev->fd, v4l2BufTypeName(ep->type), errno, strerror(errno));
		return 1;
	}

	return 0;
}

const Buffer *devV4L2PullBuffer(struct DeviceV4L2 *dev) {
	for (int i = 0; i < dev->endpoints.size; ++i) {
		Endpoint *const ep = arrayAt(&dev->endpoints, Endpoint, i);

		if (ep->state != ENDPOINT_STATE_STREAMING)
			continue;

		struct v4l2_buffer buf = {
			.type = ep->type,
			.memory = ep->buffers[0].buffer.memory, // TODO active_memory_type
		};

		if (0 != ioctl(dev->fd, VIDIOC_DQBUF, &buf)) {
			if (errno != EAGAIN) {
				LOGE("Failed to ioctl(%d, VIDIOC_DQBUF): %d, %s",
					dev->fd, errno, strerror(errno));
				return NULL;
			}

			continue;
		}

		Buffer *const ret = ep->buffers + buf.index;

		// TODO check differences
		ret->buffer = buf;
		return ret;
	}

	return NULL;
}

int devV4L2PushBuffer(struct DeviceV4L2 *dev, const Buffer *buf) {
	if (0 != ioctl(dev->fd, VIDIOC_QBUF, &buf->buffer)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QBUF): %d, %s",
			dev->fd, errno, strerror(errno));
		return errno;
	}

	return 0;
}
