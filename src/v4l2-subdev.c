#include "v4l2-subdev.h"

#include "common.h"

// open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // close
#include <sys/ioctl.h> // ioctl

#include <stdlib.h> // malloc

#include <errno.h>
#include <string.h> // strerror

#include <stdint.h> // uint32_t and friends

static int subdevReadCaps(Subdev *sd) {
	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_QUERYCAP, &sd->cap)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_QUERYCAP): %d, %s", sd->fd, errno, strerror(errno));
		return -1;
	}

	v4l2PrintSubdevCapability(&sd->cap);

	return 0;
}

static int subdevFormatGet(Subdev *sd, int pad) {
	struct v4l2_subdev_format format = {
		.pad = pad,
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};
	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_FMT, &format)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_FMT, pad=%d): %d, %s", sd->fd, pad, errno, strerror(errno));
		return -1;
	}

	sd->pads[pad].format = format;
	v4l2PrintSubdevFormat(&format);

	return 0;
}

static int subdevSelectionGet(Subdev *sd, int pad) {
	struct v4l2_subdev_selection selection = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
		.pad = pad,
	};
	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_SELECTION, &selection)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_SELECTION, pad=%d): %d, %s", sd->fd, pad, errno, strerror(errno));
		return -1;
	}

	v4l2PrintSubdevSelection(&selection);

	return 0;
}

static int subdevFrameIntervalGet(Subdev *sd, int pad) {
	struct v4l2_subdev_frame_interval fi = {
		.pad = pad,
	};
	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_FRAME_INTERVAL, &fi)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_FRAME_INTERVAL, pad=%d): %d, %s", sd->fd, pad, errno, strerror(errno));
		return -1;
	}

	v4l2PrintFrameInterval(&fi);

	return 0;
}

static int subdevEnumFrameIntervals(Subdev *sd, int pad, uint32_t mbus_code, int w, int h) {
	LOGI("Enumerating frame intervals for pad=%d mcode=%s(%08x) size=%dx%d",
		pad, v4l2MbusFmtName(mbus_code), mbus_code, w, h);

	for (int i = 0;; ++i) {
		struct v4l2_subdev_frame_interval_enum fiv = {
			.index = i,
			.pad = pad,
			.which = V4L2_SUBDEV_FORMAT_ACTIVE,
			.code = mbus_code,
			.width = w,
			.height = h,
		};

		if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL, &fiv)) {
			if (EINVAL == errno) {
				LOGI("Pad has %d frame intervals", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("Pad has no frame intervals");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL, (pad=%d, code=%08x, %dx%d)): %d, %s",
				sd->fd, pad, mbus_code, w, h, errno, strerror(errno));
			return errno;
		}

		v4l2PrintSubdevFrameInterval(&fiv);
	}

	return 0;
}

static int subdevEnumFrameSizes(Subdev *sd, int pad, uint32_t mbus_code) {
	LOGI("Enumerating frame sizes for pad=%d mcode=%s(%08x)", pad, v4l2MbusFmtName(mbus_code), mbus_code);
	for (int i = 0;; ++i) {
		struct v4l2_subdev_frame_size_enum fsz = {
			.index = i,
			.pad = pad,
			.which = V4L2_SUBDEV_FORMAT_ACTIVE,
			.code = mbus_code,
		};

		if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_ENUM_FRAME_SIZE, &fsz)) {
			if (EINVAL == errno) {
				LOGI("Pad has %d frame sizes", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("Pad has no frame sizes");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_ENUM_FRAME_SIZE, (pad=%d, code=%08x)): %d, %s",
				sd->fd, pad, mbus_code, errno, strerror(errno));
			return errno;
		}

		v4l2PrintSubdevFrameSize(&fsz);

		subdevEnumFrameIntervals(sd, fsz.pad, fsz.code, fsz.min_width, fsz.min_height);
		if (fsz.min_width != fsz.max_width || fsz.min_height != fsz.max_height)
			subdevEnumFrameIntervals(sd, fsz.pad, fsz.code, fsz.max_width, fsz.max_height);
	}

	return 0;
}

static int subdevEnumMbusCodes(Subdev *sd, int pad) {
	LOGI("Enumerating mbus codes for pad=%d", pad);
	for (int i = 0;; ++i) {
		struct v4l2_subdev_mbus_code_enum mbc = {
			.index = i,
			.pad = pad,
			.which = V4L2_SUBDEV_FORMAT_ACTIVE,
		};

		if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_ENUM_MBUS_CODE, &mbc)) {
			if (EINVAL == errno) {
				LOGI("Pad has %d mbus codes", i);
				return 0;
			}

			if (ENOTTY == errno) {
				LOGI("Pad has no mbus codes");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_ENUM_MBUS_CODE, pad=%d): %d, %s", sd->fd, pad, errno, strerror(errno));
			return errno;
		}

		v4l2PrintSubdevMbusCode(&mbc);

		subdevEnumFrameSizes(sd, mbc.pad, mbc.code);
	}

	return 0;
}

Subdev *subdevOpen(const char *name, int pads_count) {
	Subdev sd = {0};

	sd.fd = open(name, O_RDWR | O_NONBLOCK);
	if (sd.fd < 0) {
		LOGE("Failed to open \"%s\": %d, %s", name, errno, strerror(errno));
		goto fail;
	}

	if (0 != subdevReadCaps(&sd))
		goto fail;

	//subdevReadTimings(&sd);
	for (int pad = 0; pad < pads_count; ++pad) { 
		LOGI("Pad %d:", pad);
		subdevFormatGet(&sd, pad);
		subdevSelectionGet(&sd, pad);
		subdevFrameIntervalGet(&sd, pad);
		subdevEnumMbusCodes(&sd, pad);
	}

fail:
	if (sd.fd > 0)
		close(sd.fd);
	return NULL;
}

void subdevClose(Subdev *sd) {
	if (!sd)
		return;

	if (sd->fd > 0)
		close(sd->fd);

	free(sd);
}
