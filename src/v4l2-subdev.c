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
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_FMT, pad=%d): %d, %s", pad, sd->fd, errno, strerror(errno));
		return -1;
	}

	v4l2PrintSubdevFormat(&format);

	return 0;
}

static int subdevSelectionGet(Subdev *sd, int pad) {
	struct v4l2_subdev_selection selection = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
		.pad = pad,
	};
	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_SELECTION, &selection)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_SELECTION, pad=%d): %d, %s", pad, sd->fd, errno, strerror(errno));
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
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_FRAME_INTERVAL, pad=%d): %d, %s", pad, sd->fd, errno, strerror(errno));
		return -1;
	}

	v4l2PrintFrameInterval(&fi);

	return 0;
}

static int subdevEnumMbusCodes(Subdev *sd, int pad) {
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

#define VIDIOC_SUBDEV_ENUM_MBUS_CODE		_IOWR('V',  2, struct v4l2_subdev_mbus_code_enum)
#define VIDIOC_SUBDEV_ENUM_FRAME_SIZE		_IOWR('V', 74, struct v4l2_subdev_frame_size_enum)
#define VIDIOC_SUBDEV_ENUM_FRAME_INTERVAL	_IOWR('V', 75, struct v4l2_subdev_frame_interval_enum)
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
