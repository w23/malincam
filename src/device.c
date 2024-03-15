#include "device.h"

#include "v4l2-print.h"
#include "common.h"

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
	DeviceStream stream = {0};
	stream.dev_fd = fd;
	stream.type = buffer_type;
	stream.state = STREAM_STATE_IDLE;

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
	
	/* TODO verbose logs
	LOGI("Current format:");
	v4l2PrintFormat(&stream.format);
	*/

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

/*
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
*/

#if 0
static void v4l2_enum_controls(Device *const dev) {
	//arrayInit(&dev->ctrls, struct v4l2_query_ext_ctrl);

	struct v4l2_queryctrl qctrl = {0};
	for (int i = 0;; i++) {
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
		if (0 > ioctl(dev->fd, VIDIOC_QUERYCTRL, &qctrl)) {
			LOGI("Total controls: %d", i);
			break;
		}

		LOGI(" ctrl[%d]: id=(%d)%s type=%s name='%s' range=[%d.+%u.%d] default=%d flags=%08x", i,
			qctrl.id, v4l2CtrlIdName(qctrl.id),
			v4l2CtrlTypeName(qctrl.type),
			qctrl.name,
			qctrl.minimum, qctrl.step, qctrl.maximum,
			qctrl.default_value,
			qctrl.flags
		);

		// TODO qctrl.type == V4L2_CTRL_TYPE_MENU

		//arrayAppend(&dev->ctrls, &qctrl);
	}
}
#endif

static int v4l2Selection(int fd, enum v4l2_buf_type type, uint32_t cmd, uint32_t target, struct v4l2_rect *rekt) {
	struct v4l2_selection sel = {
		.type = type,
		.target = target,
	};

	if (rekt && cmd == VIDIOC_S_SELECTION) {
		sel.r = *rekt;
	}

	if (0 != ioctl(fd, cmd, &sel)) {
		LOGE("Failed to ioctl(%d, VIDIOC_%c_SELECTION, [target=%s]): %d, %s",
			fd, cmd == VIDIOC_S_SELECTION ? 'S' : 'G',
			v4l2SelTgtName(target),
			errno, strerror(errno));
		return -1;
	}
	
	LOGI("VIDIOC_%c_SELECTION:", cmd == VIDIOC_S_SELECTION ? 'S' : 'G');
	v4l2PrintSelection(&sel);

	if (rekt) {
		*rekt = sel.r;
	}

	return 0;
}

static int setCrop(DeviceStream *st, int w, int h) {
	struct v4l2_rect crop_bounds = {0};
	struct v4l2_rect crop_default = {0};
	struct v4l2_rect native = {0};
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_NATIVE_SIZE, &native);
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_CROP_BOUNDS, &crop_bounds);
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_CROP_DEFAULT, &crop_default);
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_CROP, &st->crop);

	// TODO do math, validate, etc
	if (w != 0 && h != 0) {
		struct v4l2_rect rect = crop_default;
		rect.width = w;
		rect.height = h;
		v4l2Selection(st->dev_fd, st->type, VIDIOC_S_SELECTION, V4L2_SEL_TGT_CROP, &rect);
		v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_CROP, &st->crop);
	}

	return 0;
}

static int setCompose(DeviceStream *st, int w, int h) {
	struct v4l2_rect compose_bounds = {0};
	struct v4l2_rect compose_default = {0};
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_COMPOSE_BOUNDS, &compose_bounds);
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_COMPOSE_DEFAULT, &compose_default);
	v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_COMPOSE, &st->compose);

	// TODO do math, validate, etc
	if (w != 0 && h != 0) {
		struct v4l2_rect rect = compose_default;
		rect.width = w;
		rect.height = h;
		v4l2Selection(st->dev_fd, st->type, VIDIOC_S_SELECTION, V4L2_SEL_TGT_COMPOSE, &rect);
		v4l2Selection(st->dev_fd, st->type, VIDIOC_G_SELECTION, V4L2_SEL_TGT_COMPOSE, &st->compose);
	}

	return 0;
}

static void setPixelFormat(struct v4l2_format *fmt, uint32_t pixelformat, int w, int h) {
	if (!IS_TYPE_MPLANE(fmt->type)) {
		struct v4l2_pix_format *const pix = &fmt->fmt.pix;
		pix->pixelformat = pixelformat;
		pix->width = w;
		pix->height = h;

		pix->sizeimage = 0;
		pix->bytesperline = 0;
	} else {
		struct v4l2_pix_format_mplane *const pix_mp = &fmt->fmt.pix_mp;
		pix_mp->pixelformat = pixelformat;
		pix_mp->width = w;
		pix_mp->height = h;

		// TODO proper multiplanar formats
		pix_mp->num_planes = 1;
		pix_mp->plane_fmt[0].sizeimage = 0;
		pix_mp->plane_fmt[0].bytesperline = 0;
	}
}

static int streamSetFormat(DeviceStream *st, uint32_t pixelformat, int w, int h) {
	LOGI("Setting format %s(%x) %dx%d for Devicestream=%s(%d)",
		v4l2PixFmtName(pixelformat), pixelformat, w, h,
		v4l2BufTypeName(st->type), st->type);

	if (w == 0 && h == 0 && pixelformat == 0) {
		return 0;
	}

	struct v4l2_format fmt = st->format;
	ASSERT(fmt.type == st->type);
	setPixelFormat(&fmt, pixelformat, w, h);

	if (0 != ioctl(st->dev_fd, VIDIOC_S_FMT, &fmt)) {
		LOGE("Failed to ioctl(%d, VIDIOC_S_FMT, %s): %d, %s",
			st->dev_fd, v4l2BufTypeName(st->type), errno, strerror(errno));
		return -1;
	}

	// Remember as current format
	st->format = fmt;
	LOGI("Set format:");
	v4l2PrintFormat(&st->format);

	return 0;
}

static int bufferExportDmabufFd(int fd, uint32_t buf_type, uint32_t index, uint32_t plane) {
	struct v4l2_exportbuffer exp = {
		.type = buf_type,
		.index = index,
		.plane = plane,
		.flags = O_CLOEXEC | O_RDONLY,
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

		buf->dmabuf_fd[i] = fd;
	}

	return 0;
}

static int bufferMmap(DeviceStream *st, Buffer *const buf) {
	if (IS_STREAM_MPLANE(st)) {
		const int planes_num = st->format.fmt.pix_mp.num_planes;
		ASSERT(planes_num < VIDEO_MAX_PLANES);

		for (int i = 0; i < planes_num; ++i) {
			const uint32_t offset = buf->buffer.m.planes[i].m.mem_offset;
			const uint32_t length = buf->buffer.m.planes[i].length;
			buf->mapped[i] = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, st->dev_fd, offset);
			const int err = errno;
			if (buf->mapped[i] == MAP_FAILED) {
				// FIXME munmap already mmapped
				LOGE("Failed to mmap(%d, buffer[%d]): %d, %s", st->dev_fd, buf->buffer.index, errno, strerror(errno));
				return err;
			}

			LOGI("buf.index=%d plane=%d mmap=%p", buf->buffer.index, i, buf->mapped[i]);
		} // for planes
	} else {
		buf->mapped[0] = mmap(NULL, buf->buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, st->dev_fd, buf->buffer.m.offset);
		const int err = errno;
		if (buf->mapped[0] == MAP_FAILED) {
			LOGE("Failed to mmap(%d, buffer[%d]): %d, %s", st->dev_fd, buf->buffer.index, errno, strerror(errno));
			return err;
		}
		LOGI("buf.index=%d mmap=%p", buf->buffer.index, buf->mapped[0]);
	}

	return 0;
}

static int bufferPrepare(DeviceStream *st, Buffer *const buf, buffer_memory_e buffer_memory) {
	switch (buffer_memory) {
		case BUFFER_MEMORY_MMAP:
			return bufferMmap(st, buf);

		case BUFFER_MEMORY_DMABUF_EXPORT:
			return bufferDmabufExport(st, buf);

		case BUFFER_MEMORY_USERPTR:
			// Nothing to do, buffer pointer will be provided externally
		case BUFFER_MEMORY_DMABUF_IMPORT:
			// Nothing to do, fds will be provided externally
			return 0;

		default:
			LOGE("%s: Unimplemented memory type %s (%d)", __func__, v4l2MemoryTypeName(buf->buffer.memory), (int)buf->buffer.memory);
			return EINVAL;
	}

	return EINVAL;
}

static int streamRequestBuffers(DeviceStream *st) {
	// TODO check if anything changed that require buffer request?
	// - frame size, pixelformat, etc

	if (st->buffers) {
		LOGE("%s: stream=%p(fd=%d) already has buffers (active?) FIXME free them?", __func__, (void*)st, st->dev_fd);
		return -EBUSY;
	}

	struct v4l2_requestbuffers req = {0};
	req.type = st->type;
	req.count = st->buffers_count;
	switch (st->buffer_memory) {
		case BUFFER_MEMORY_MMAP:
		case BUFFER_MEMORY_DMABUF_EXPORT:
			req.memory = V4L2_MEMORY_MMAP;
			break;
		case BUFFER_MEMORY_USERPTR:
			req.memory = V4L2_MEMORY_USERPTR;
			break;
		case BUFFER_MEMORY_DMABUF_IMPORT:
			req.memory = V4L2_MEMORY_DMABUF;
			break;
		default:
			LOGE("Invalid buffer memory type %08x", st->buffer_memory);
			return EINVAL;
	}

	/* TODO verbose logs
	LOGI("Requesting buffers: ");
	v4l2PrintRequestBuffers(&req);
	*/

	if (0 > ioctl(st->dev_fd, VIDIOC_REQBUFS, &req)) {
		LOGE("Failed to ioctl(%d, VIDIOC_REQBUFS): %d, %s", st->dev_fd, errno, strerror(errno));
		return -1;
	}

	/* TODO verbose logs
	LOGI("Requested buffers: ");
	v4l2PrintRequestBuffers(&req);
	*/

	if ((int)req.count != st->buffers_count) {
		LOGE("%s: stream=%p(fd=%d) requested %d buffers, but returned %d",
			__func__, (void*)st, st->dev_fd, st->buffers_count, req.count);
		st->buffers_count = req.count;
	}

	st->buffers = calloc(req.count, sizeof(*st->buffers));

	struct v4l2_plane bmplanes[VIDEO_MAX_PLANES];
	for (int i = 0; i < st->buffers_count; ++i) {
		Buffer *const buf = st->buffers + i;
		buf->buffer = (struct v4l2_buffer){
			.type = req.type,
			.memory = req.memory,
			.index = i,
		};

		if (IS_STREAM_MPLANE(st)) {
			buf->buffer.m.planes = bmplanes;
			buf->buffer.length = st->format.fmt.pix_mp.num_planes;
		}

		if (0 != ioctl(st->dev_fd, VIDIOC_QUERYBUF, &buf->buffer)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QUERYBUF, [%d]): %d, %s", st->dev_fd, i, errno, strerror(errno));
			goto fail;
		}

		/* TODO verbose logs
		LOGI("Queried buffer %d:", i);
		v4l2PrintBuffer(&buf->buffer);
		*/

		if (0 != bufferPrepare(st, buf, st->buffer_memory)) {
			LOGE("Error preparing buffer %d", i);
			goto fail;
		}
	} // for st->buffers_count

	return 0;

fail:
	// FIXME remove ones we've already queried?
	free(st->buffers);
	st->buffers = NULL;
	return 1;
}

static int streamEnqueueBuffers(DeviceStream *st) {
	for (int i = 0; i < st->buffers_count; ++i) {
		if (0 != ioctl(st->dev_fd, VIDIOC_QBUF, &st->buffers[i].buffer)) {
			LOGE("Failed to ioctl(%d, VIDIOC_QBUF, %i): %d, %s",
				st->dev_fd, i, errno, strerror(errno));
			return -1;
		}
	}

	return 0;
}

static void streamDestroy(DeviceStream *st) {
	for (int i = 0; i < st->buffers_count; ++i) {
		//Buffer *const buf = st->buffers + i;
		switch (st->buffer_memory) {
			case BUFFER_MEMORY_MMAP:
				/* FIXME
				if (buf->mapped && 0 != munmap(buf->mapped, buf->buffer.length)) {
					LOGE("munmap(%p) => %s (%d)", buf->mapped, strerror(errno), errno);
				}
				*/
				break;
			case BUFFER_MEMORY_DMABUF_EXPORT:
				/* FIXME
				{
					const int planes_num = streamPlanesNum(st);
					for (int i = 0; i < planes_num; ++i) {
						close(buf->dmabuf_fd[i]);
					}
				}*/
				break;

			case BUFFER_MEMORY_NONE:
			case BUFFER_MEMORY_USERPTR:
			case BUFFER_MEMORY_DMABUF_IMPORT:
				// Nothing to do
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

	//v4l2_enum_input(dev.fd);

	// TODO VIDIOC_ENUM_FRAMEINTERVALS
	// Depends on selected resolution. So cannot really query before the resolution is picked.

	// TODO set frameinterval, use VIDIOC_S_PARM

	Device* const ret = (Device*)malloc(sizeof(Device));
	*ret = dev;
	ret->controls = v4l2ControlsCreateFromV4l2Fd(ret->fd);
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

	v4l2ControlsDestroy(&dev->controls);

	close(dev->fd);
	free(dev);
}

int deviceEventSubscribe(Device *dev, uint32_t event) {
	struct v4l2_event_subscription subs = {
		.type = event,
		.id = 0,
		.flags = 0,
	};

	if (0 != ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &subs)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBSCRIBE_EVENT, %x): %d, %s",
			dev->fd, event, errno, strerror(errno));
		return 1;
	}

	return 0;
}

// Returns <0 on error, =0 on no events, =1 on event
int deviceEventGet(Device *dev, struct v4l2_event *out) {
	if (0 != ioctl(dev->fd, VIDIOC_DQEVENT, out)) {
		if (errno == ENOENT) {
			// No events, wait
			return 0;
		}

		LOGE("Failed to ioctl(%d, VIDIOC_DQEVENT): %d, %s",
			dev->fd, errno, strerror(errno));
		return -1;
	}

	return 1;
}

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

#define V4L2_DUMP_FORMATS
#ifdef V4L2_DUMP_FORMATS
		//v4l2PrintFormatDesc(&fmt);
		LOGI("  fmt[%d] = {%s, %s}", i, v4l2PixFmtName(fmt.pixelformat), fmt.description);
		v4l2PrintFormatFlags(fmt.flags);
#endif

		// Enumerate possible sizes
		for (int i = 0;; ++i) {
			struct v4l2_frmsizeenum fse = { .index = i, .pixel_format = fmt.pixelformat };
			if (0 != ioctl(st->dev_fd, VIDIOC_ENUM_FRAMESIZES, &fse)) {
				if (EINVAL == errno) {
					LOGI("  Format has %d framesizes", i);
					break;
				}
			}

#ifdef V4L2_DUMP_FORMATS
			v4l2PrintFrmSizeEnum(&fse);
#endif

			// Only discrete supports index > 0
			if (fse.type != V4L2_FRMSIZE_TYPE_DISCRETE)
				break;
		}

		arrayAppend(&st->formats, &fmt);
	}
}

int deviceStreamPrepare(DeviceStream *st, const DeviceStreamPrepareOpts *opts) {
	if (st->state == STREAM_STATE_STREAMING) {
		LOGE("%s: stream=%p is streaming", __func__, (void*)st);
		return -EBUSY;
	}

	if (0 != streamSetFormat(st, opts->pixelformat, opts->width, opts->height)) {
		return -1;
	}

	st->crop = (struct v4l2_rect){
		.left = 0,
		.top = 0,
		.width = opts->width,
		.height = opts->height,
	};
	setCrop(st, opts->crop_width, opts->crop_height);

	st->compose = st->crop;
	setCompose(st, opts->crop_width, opts->crop_height);

	st->buffer_memory = opts->buffer_memory;
	st->buffers_count = opts->buffers_count;

	if (0 != streamRequestBuffers(st)) {
		return -2;
	}

	st->state = STREAM_STATE_PREPARED;
	return 0;
}

int deviceStreamStart(DeviceStream *st) {
	LOGI("%s: stream=%p(fd=%d)", __func__, (void*)st, st->dev_fd);
	switch (st->state) {
		case STREAM_STATE_IDLE:
			LOGE("%s: stream=%p(fd=%d) is not prepared", __func__, (void*)st, st->dev_fd);
			return -ENOENT;
		case STREAM_STATE_STREAMING:
			LOGE("%s: stream=%p(fd=%d) is already streaming", __func__, (void*)st, st->dev_fd);
			return -EBUSY;
		case STREAM_STATE_PREPARED:
			break;
	}

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
	LOGI("%s: stream=%p(fd=%d)", __func__, (void*)st, st->dev_fd);
	if (st->state != STREAM_STATE_STREAMING) {
			LOGE("%s: stream=%p(fd=%d) is not streaming", __func__, (void*)st, st->dev_fd);
			return 0;
	}

	// VIDIOC_STREAMOFF will immediately stop streaming, cancel DMA, and dequeue all the buffers
	if (0 != ioctl(st->dev_fd, VIDIOC_STREAMOFF, &st->type)) {
		LOGE("Failed to ioctl(%d, VIDIOC_STREAMOFF, %s): %d, %s",
			st->dev_fd, v4l2BufTypeName(st->type), errno, strerror(errno));
		return 1;
	}

	st->state = STREAM_STATE_PREPARED;
	return 0;
}

const Buffer *deviceStreamPullBuffer(DeviceStream *st) {
	if (!st->buffers) {
		LOGE("%s: stream=%p(fd=%d) is not active", __func__, (void*)st, st->dev_fd);
		return NULL;
	}

	struct v4l2_plane planes[VIDEO_MAX_PLANES] = {0};

	struct v4l2_buffer buf = {
		.type = st->type,
		.memory = st->buffers[0].buffer.memory,
	};

	// LOL this isn't really documented, right?
	if (IS_STREAM_MPLANE(st)) {
		buf.length = VIDEO_MAX_PLANES;
		buf.m.planes = planes;
	}

	if (0 != ioctl(st->dev_fd, VIDIOC_DQBUF, &buf)) {
		if (errno != EAGAIN /* FIXME: */ && errno != EPIPE) {
			LOGE("Failed to ioctl(%d, VIDIOC_DQBUF): %d, %s",
				st->dev_fd, errno, strerror(errno));
			LOGE("Buffer was:");
			v4l2PrintBuffer(&buf);
		}

		return NULL;
	}

	Buffer *const ret = st->buffers + buf.index;

	// TODO check differences
	ret->buffer = buf;
	if (IS_STREAM_MPLANE(st)) {
		ret->buffer.m.planes = ret->planes;
		const int planes_count = (int)ret->buffer.length;
		for (int i = 0; i < planes_count; ++i) {
			ret->planes[i] = planes[i];
		}
	}

	//LOGI("%s: %d %p:", __func__, ret->buffer.index, (void*)ret);
	return ret;
}

int deviceStreamPushBuffer(DeviceStream *st, const Buffer *buf) {
	if (!st->buffers) {
		LOGE("%s: stream=%p(fd=%d) is not active", __func__, (void*)st, st->dev_fd);
		return -EIO;
	}

	//LOGI("%s: %d %p:", __func__, buf->buffer.index, (void*)buf);
	if (0 != ioctl(st->dev_fd, VIDIOC_QBUF, &buf->buffer)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QBUF): %d, %s",
			st->dev_fd, errno, strerror(errno));
		LOGE("Buffer was:");
		v4l2PrintBuffer(&buf->buffer);
		return errno;
	}

	return 0;
}
