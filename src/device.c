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

#include <stdlib.h> // malloc

#include "common.h"
#include "device.h"

int deviceStreamQueryFormats(DeviceStream *st, int mbus_code) {
	LOGI("Enumerating formats for type=%s(%x) mbus_code=%s(%x)",
		v4l2BufTypeName(st->type), st->type,
		v4l2MbusFmtName(mbus_code), mbus_code);
	arrayResize(&st->formats);

	for (int i = 0;; ++i) {
		struct v4l2_fmtdesc fmt;
		fmt.index = i;
		fmt.type = st->type;
		fmt.mbus_code = mbus_code;
		if (0 != ioctl(st->dev_fd, VIDIOC_ENUM_FMT, &fmt)) {
			if (EINVAL == errno) {
				LOGI("Enumerated %d formats", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("DeviceStream has no formats");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_ENUM_FMT): %d, %s", st->dev_fd, errno, strerror(errno));
			return errno;
		}

		//v4l2PrintFormatDesc(&fmt);
		LOGI("  fmt[%d] = {%s, %s}", i, v4l2PixFmtName(fmt.pixelformat), fmt.description);
		v4l2PrintFormatFlags(fmt.flags);

		// Enumerate possible sizes
		for (int i = 0;; ++i) {
			struct v4l2_frmsizeenum fse = { .index = i, .pixel_format = fmt.pixelformat };
			if (0 != ioctl(st->dev_fd, VIDIOC_ENUM_FRAMESIZES, &fse)) {
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

		arrayAppend(&st->formats, &fmt);
	}
}

static int streamGetFormat(DeviceStream *st, int fd) {
		st->format = (struct v4l2_format){0};
		st->format.type = st->type;
		if (0 != ioctl(fd, VIDIOC_G_FMT, &st->format)) {
			LOGE("Failed to ioctl(%d, VIDIOC_G_FMT, %s): %d, %s",
				fd, v4l2BufTypeName(st->type), errno, strerror(errno));
			return -1;
		}

		return 0;
}

static uint32_t v4l2ReadBufferTypeCapabilities(int fd, uint32_t buffer_type) {
	struct v4l2_requestbuffers req = {0};
	req.type = buffer_type;
	req.count = 0;
	req.memory = V4L2_MEMORY_MMAP;
	if (0 != ioctl(fd, VIDIOC_REQBUFS, &req)) {
		LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", fd, errno, strerror(errno));
		return 0;
	}

	return req.capabilities;
}

static int streamInit(DeviceStream *st, int fd, uint32_t buffer_type) {
	DeviceStream stream;
	stream.dev_fd = fd;
	stream.type = buffer_type;

	arrayInit(&stream.formats, struct v4l2_fmtdesc);

	// Read supported memory types
	stream.buffer_capabilities = v4l2ReadBufferTypeCapabilities(fd, buffer_type);
	if (stream.buffer_capabilities == 0) {
		goto fail;
	}
	LOGI("DeviceStream capabilities:");
	v4l2PrintBufferCapabilityBits(stream.buffer_capabilities);

	// Read current format
	if (0 != streamGetFormat(&stream, fd)) {
		goto fail;
	}
	LOGI("Current format:");
	v4l2PrintFormat(&stream.format);

	*st = stream;
	return 0;

fail:
	arrayDestroy(&stream.formats);
	return -1;
}

static int v4l2QueryCapability(Device *dev) {
	if (0 != ioctl(dev->fd, VIDIOC_QUERYCAP, &dev->caps)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QUERYCAP): %d, %s", dev->fd, errno, strerror(errno));
		return errno;
	}

	v4l2PrintCapability(&dev->caps);

	dev->this_device_caps = dev->caps.capabilities & V4L2_CAP_DEVICE_CAPS
		? dev->caps.device_caps : dev->caps.capabilities;

	if ((V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M) & dev->this_device_caps) {
		if (0 != streamInit(&dev->capture, dev-> fd, V4L2_BUF_TYPE_VIDEO_CAPTURE))
			goto fail;
	} else if ((V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE) & dev->this_device_caps) {
		if (0 != streamInit(&dev->capture, dev-> fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE))
			goto fail;
	} else {
		dev->capture.type = 0;
	}

	if ((V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M) & dev->this_device_caps) {
		if (0 != streamInit(&dev->output, dev-> fd, V4L2_BUF_TYPE_VIDEO_OUTPUT))
			goto fail;
	} else if ((V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE) & dev->this_device_caps) {
		if (0 != streamInit(&dev->output, dev-> fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE))
			goto fail;
	} else {
		dev->output.type = 0;
	}

	return 0;

fail:
	// FIXME destroy streams
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

static int fillFormatInfo(struct v4l2_format *fmt, uint32_t pixelformat, int w, int h) {
	// FIXME why (copypasted from current format)
	// TODO probably the best way to do this would be to:
	// 1. Enumerate all the supported formats.
	// 2. Pick the format with the same pixelformat from the list of supported ones.
	fmt->fmt.pix.field = 1;
	fmt->fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
	fmt->fmt.pix.quantization = V4L2_QUANTIZATION_LIM_RANGE;
	fmt->fmt.pix.xfer_func = V4L2_XFER_FUNC_SRGB;

	switch (pixelformat) {
		case V4L2_PIX_FMT_YUYV:
			{
				const int bits_per_pixel = 16;
				fmt->fmt.pix.pixelformat = pixelformat;
				fmt->fmt.pix.width = w;
				fmt->fmt.pix.height = h;
				fmt->fmt.pix.sizeimage = w * h * bits_per_pixel / 8;
				fmt->fmt.pix.bytesperline = w * bits_per_pixel / 8;
				return 0;
			}
			break;
		case V4L2_PIX_FMT_SBGGR10:
		case V4L2_PIX_FMT_SGBRG10:
		case V4L2_PIX_FMT_SGRBG10:
		case V4L2_PIX_FMT_SRGGB10:

		case V4L2_PIX_FMT_SBGGR12:
		case V4L2_PIX_FMT_SGBRG12:
		case V4L2_PIX_FMT_SGRBG12:
		case V4L2_PIX_FMT_SRGGB12:

		case V4L2_PIX_FMT_SBGGR14:
		case V4L2_PIX_FMT_SGBRG14:
		case V4L2_PIX_FMT_SGRBG14:
		case V4L2_PIX_FMT_SRGGB14:
/*
		case V4L2_PIX_FMT_SGBRG14P:
		case V4L2_PIX_FMT_SBGGR10P:
		case V4L2_PIX_FMT_SGBRG10P:
		case V4L2_PIX_FMT_SGRBG10P:
		case V4L2_PIX_FMT_SRGGB10P:
		case V4L2_PIX_FMT_SBGGR12P:
		case V4L2_PIX_FMT_SGBRG12P:
		case V4L2_PIX_FMT_SGRBG12P:
		case V4L2_PIX_FMT_SRGGB12P:
		case V4L2_PIX_FMT_SBGGR14P:
		case V4L2_PIX_FMT_SGRBG14P:
		case V4L2_PIX_FMT_SRGGB14P:
*/
			{

				// w/2 * G + (w/2) * (R+B) 16bit samples
				const int bits_per_pixel = 16;
				fmt->fmt.pix.pixelformat = pixelformat;
				fmt->fmt.pix.width = w;
				fmt->fmt.pix.height = h;
				fmt->fmt.pix.sizeimage = w * h * bits_per_pixel / 8;
				fmt->fmt.pix.bytesperline = w * bits_per_pixel / 8;
				return 0;
			}
			break;
	}

	return EINVAL;
}

static int endpontSetFormat(DeviceStream *st, uint32_t pixelformat, int w, int h) {
	LOGI("Setting format for Devicestream=%s(%d)", v4l2BufTypeName(st->type), st->type);

	if (IS_STREAM_MPLANE(st)) {
		LOGE("%s: type=%s is multiplane and is not supported", __func__, v4l2BufTypeName(st->type));
		return EINVAL;
	}

	if (0 != streamGetFormat(st, st->dev_fd))
		return -1;

	if (w == 0 && h == 0 && pixelformat == 0) {
		return 0;
	}

	struct v4l2_format fmt = {
		.type = st->type,
	};

	if (0 != fillFormatInfo(&fmt, pixelformat, w, h)) {
		LOGE("Unsupported pixel format %s(%#x)", v4l2PixFmtName(pixelformat), pixelformat);
		LOGE("%s is not implemented == need bytesperline, sizeimage, mplanes compute code", __func__);
		return EINVAL;
	}

	if (0 != ioctl(st->dev_fd, VIDIOC_S_FMT, &fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_S_FMT, %s): %d, %s",
			st->dev_fd, v4l2BufTypeName(st->type), errno, strerror(errno));
		return -1;
	}

	// Remember as current format
	st->format = fmt;

	return 0;

#if 0
	// TODO
	LOGI("Seting format to %s %dx%d", v4l2PixFmtName(pixelformat), w, h);
	switch (st->type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
			st->format.fmt.pix_mp.width = w;
			st->format.fmt.pix_mp.height = h;
			st->format.fmt.pix_mp.pixelformat = pixelformat;
			// FIXME bytesperline
			// TODO mplanes?
			break;
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			st->format.fmt.pix.width = w;
			st->format.fmt.pix.height = h;
			st->format.fmt.pix.pixelformat = pixelformat;
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

static int bufferExportDmabufFd(int fd, uint32_t buf_type, uint32_t index, uint32_t plane) {
	struct v4l2_exportbuffer exp = {
		.type = buf_type,
		.index = index,
		.plane = plane,
	};

	if (0 != ioctl(fd, VIDIOC_EXPBUF, &exp)) {
		const int err = errno;
		LOGE("Failed to ioctl(%d, VIDIOC_EXPBUF, [%s, i=%d, p=%d]): %d, %s",
			fd, v4l2BufTypeName(buf_type), index, plane, errno, strerror(errno));
		return -err;
	}

	return exp.fd;
}

static int bufferDmabufExport(DeviceStream *st, Buffer *const buf) {
	const int planes_num = IS_STREAM_MPLANE(st) ? st->format.fmt.pix_mp.num_planes : 1;
	ASSERT(planes_num < VIDEO_MAX_PLANES);

	for (int i = 0; i < planes_num; ++i) {
		const int fd = bufferExportDmabufFd(st->dev_fd, st->type, buf->buffer.index, 0);
		if (fd <= 0) {
			// FIXME leaks 0..i fds
			// TODO clean here, or?
			return -fd;
		}

		buf->v.dmabuf.fd[i] = fd;
	}

	return 0;
}

static int bufferPrepare(DeviceStream *st, Buffer *const buf) {
	switch (buf->buffer.memory) {
		case V4L2_MEMORY_MMAP:
			{
				buf->v.mmap.ptr = mmap(NULL, buf->buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, st->dev_fd, buf->buffer.m.offset);
				const int err = errno;
				if (buf->v.mmap.ptr == MAP_FAILED) {
					LOGE("Failed to mmap(%d, buffer[%d]): %d, %s", st->dev_fd, buf->buffer.index, errno, strerror(errno));
					return err;
				}

				return 0;
			}

		case V4L2_MEMORY_DMABUF:
			// Export DMABUF fds if this is a capture stream
			if (IS_STREAM_CAPTURE(st)) {
				return bufferDmabufExport(st, buf);
			}
			return 0;

		default:
			LOGE("%s: Unimplemented memory type %s (%d)", __func__, v4l2MemoryTypeName(buf->buffer.memory), (int)buf->buffer.memory);
			return EINVAL;
	}

	return EINVAL;
}

static int streamRequestBuffers(DeviceStream *st, uint32_t count, uint32_t memory_type) {
	// TODO check if anything changed that require buffer request?
	// - frame size, pixelformat, etc

	struct v4l2_requestbuffers req = {0};
	req.type = st->type;
	req.count = count;
	req.memory = memory_type;
	if (0 != ioctl(st->dev_fd, VIDIOC_REQBUFS, &req)) {
		LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", st->dev_fd, errno, strerror(errno));
		return -1;
	}

	v4l2PrintRequestBuffers(&req);

	st->buffers = calloc(req.count, sizeof(*st->buffers));
	st->buffers_count = req.count;

	for (int i = 0; i < st->buffers_count; ++i) {
		Buffer *const buf = st->buffers + i;
		buf->buffer = (struct v4l2_buffer){
			.type = req.type,
			.memory = req.memory,
			.index = i,
		};

		if (0 != ioctl(st->dev_fd, VIDIOC_QUERYBUF, &buf->buffer)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QUERYBUF, [%d]): %d, %s", st->dev_fd, i, errno, strerror(errno));
			goto fail;
		}

		LOGI("Queried buffer %d:", i);
		v4l2PrintBuffer(&buf->buffer);

		if (0 != bufferPrepare(st, buf)) {
			LOGE("Error preparing buffer %d", i);
			goto fail;
		}
	} // for st->buffers_count

	return 0;

fail:
	// FIXME remove ones we've already queried?
	free(st->buffers);
	st->buffers = NULL;
	st->buffers_count = 0;
	return 1;
}

static int streamEnqueueBuffers(DeviceStream *st) {
	for (int i = 0; i < st->buffers_count; ++i) {
		struct v4l2_buffer buf = {
			.type = st->type,
			.memory = st->buffers[0].buffer.memory, // TODO should it be set per-Devicestream globally?
			.index = i,
		};

		if (0 != ioctl(st->dev_fd, VIDIOC_QBUF, &buf)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QBUF, %i): %d, %s",
				st->dev_fd, i, errno, strerror(errno));
			return -1;
		}
	}

	return 0;
}

static void streamDestroy(DeviceStream *st) {
	for (int i = 0; i < st->buffers_count; ++i) {
		Buffer *const buf = st->buffers + i;
		switch (buf->buffer.memory) {
			case V4L2_MEMORY_MMAP:
				if (buf->v.mmap.ptr && 0 != munmap(buf->v.mmap.ptr, buf->buffer.length)) {
					LOGE("munmap(%p) => %s (%d)", buf->v.mmap.ptr, strerror(errno), errno);
				}
				break;
		}
	}

	if (st->buffers)
		free(st->buffers);

	arrayDestroy(&st->formats);
}

struct Device* deviceOpen(const char *devname) {
	Device dev = {0};
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

	Device* const ret = (Device*)malloc(sizeof(Device));
	*ret = dev;
	return ret;

fail:
	if (dev.fd > 0)
		close(dev.fd);

	return NULL;
}

void deviceClose(struct Device* dev) {
	if (!dev)
		return;

	streamDestroy(&dev->capture);
	streamDestroy(&dev->output);

	close(dev->fd);
	free(dev);
}

int deviceStreamPrepare(DeviceStream *st, const DeviceStreamPrepareOpts *opts) {
	if (0 != endpontSetFormat(st, opts->pixelformat, opts->width, opts->height)) {
		return -1;
	}

	if (0 != streamRequestBuffers(st, opts->buffers_count, opts->memory_type)) {
		return -2;
	}

	return 0;
}

int deviceStreamStart(DeviceStream *st) {
	if (st->state != STREAM_STATE_IDLE)
		return -EINPROGRESS;

	// Capture stream should have all of its buffers ready for writing
	// TODO what should be done about starting-stopping stream?
	if (IS_STREAM_CAPTURE(st)) {
		if (0 != streamEnqueueBuffers(st)) {
			return -3;
		}
	}

	if (0 != ioctl(st->dev_fd, VIDIOC_STREAMON, &st->type)) {
		LOGE("Failed to ioctl(%d, VIDIOC_STREAMON, %s): %d, %s",
			st->dev_fd, v4l2BufTypeName(st->type), errno, strerror(errno));
		return 1;
	}
	
	st->state = STREAM_STATE_STREAMING;
	return 0;
}

int deviceStreamStop(DeviceStream *st) {
	// TODO check whether all buffers are done?

	if (0 != ioctl(st->dev_fd, VIDIOC_STREAMOFF, &st->type)) {
		LOGE("Failed to ioctl(%d, VIDIOC_STREAMON, %s): %d, %s",
			st->dev_fd, v4l2BufTypeName(st->type), errno, strerror(errno));
		return 1;
	}

	return 0;
}

const Buffer *deviceStreamPullBuffer(DeviceStream *st) {
	if (st->state != STREAM_STATE_STREAMING)
		return NULL;

	struct v4l2_buffer buf = {
		.type = st->type,
		.memory = st->buffers[0].buffer.memory,
	};

	if (0 != ioctl(st->dev_fd, VIDIOC_DQBUF, &buf)) {
		if (errno != EAGAIN) {
			LOGE("Failed to ioctl(%d, VIDIOC_DQBUF): %d, %s",
				st->dev_fd, errno, strerror(errno));
		}

		return NULL;
	}

	Buffer *const ret = st->buffers + buf.index;

	// TODO check differences
	ret->buffer = buf;
	return ret;
}

int deviceStreamPushBuffer(DeviceStream *st, const Buffer *buf) {
	if (0 != ioctl(st->dev_fd, VIDIOC_QBUF, &buf->buffer)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QBUF): %d, %s",
			st->dev_fd, errno, strerror(errno));
		return errno;
	}

	return 0;
}
