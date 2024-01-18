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

static int subdevSelectionGet(Subdev *sd, int pad, int target, struct v4l2_rect *out) {
	struct v4l2_subdev_selection selection = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
		.pad = pad,
		.target = target,
	};

	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_SELECTION, &selection)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_SELECTION, pad=%d): %d, %s", sd->fd, pad, errno, strerror(errno));
		return -1;
	}

	//v4l2PrintSubdevSelection(&selection);
	LOGI(" sel[pad=%d, %s] = (%d,%d) + (%dx%d)", pad, v4l2SelTgtName(target),
		selection.r.top, selection.r.left, selection.r.width, selection.r.height);
	v4l2PrintSelFlags(selection.target);

	if (out)
		*out = selection.r;

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
	//LOGI("Enumerating frame intervals for pad=%d mcode=%s(%08x) size=%dx%d",
		//pad, v4l2MbusFmtName(mbus_code), mbus_code, w, h);

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
				//LOGI("Pad has no frame intervals");
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
	//LOGI("Enumerating frame sizes for pad=%d mcode=%s(%08x)", pad, v4l2MbusFmtName(mbus_code), mbus_code);
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

		//v4l2PrintSubdevFrameSize(&fsz);
		LOGI(" %s(%#x) [%d..%d]x[%d..%d]", v4l2MbusFmtName(mbus_code), mbus_code,
		 fsz.min_width, fsz.max_width,
		 fsz.min_height, fsz.max_height);

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

	sd.pads = malloc(sizeof(*sd.pads) * pads_count);
	sd.pads_count = pads_count;

	//subdevReadTimings(&sd);
	for (int ipad = 0; ipad < pads_count; ++ipad) { 
		SubdevPad *const pad = sd.pads + ipad;
		LOGI("Pad %d:", ipad);
		subdevFormatGet(&sd, ipad);
		subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_NATIVE_SIZE, &pad->native);
		subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_CROP, &pad->crop);
		subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_CROP_DEFAULT, &pad->crop_default);
		subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_CROP_BOUNDS, &pad->crop_bounds);
		//subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_COMPOSE, NULL);
		// invalid for subdev subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_COMPOSE_DEFAULT);
		//subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_COMPOSE_BOUNDS, NULL);
		//subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_COMPOSE_ACTIVE, NULL);
		// invalid for subdev subdevSelectionGet(&sd, ipad, V4L2_SEL_TGT_COMPOSE_PADDED);
		subdevFrameIntervalGet(&sd, ipad);
		subdevEnumMbusCodes(&sd, ipad);
	}

	Subdev *out = malloc(sizeof(*out));
	*out = sd;
	return out;

fail:
	if (sd.pads)
		free(sd.pads);

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

int subdevSet(Subdev *sd, SubdevSet *set) {
	if (set->pad < 0 || set->pad >= sd->pads_count)
		return EINVAL;

	SubdevPad *const pad = sd->pads + set->pad;

	// Fill format with values and try to set it
	// TODO skip if the new format is the same
	// TODO check whether the format/resolution is explicitly supported
	//      and then pick the most suitable
	struct v4l2_subdev_format format = pad->format;
	format.which = V4L2_SUBDEV_FORMAT_TRY;
	format.format.code = set->mbus_code;
	format.format.width = set->width;
	format.format.height = set->height;

	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_S_FMT, &format)) {
		const int err = errno;
		// TODO print out old and new
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_S_FMT, pad=%d): %d, %s", sd->fd, set->pad, err, strerror(err));
		return err;
	}

	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_FMT, &format)) {
		const int err = errno;
		// TODO print out old and new
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_FMT, pad=%d): %d, %s", sd->fd, set->pad, err, strerror(err));
		return err;
	}

	LOGI("Got format %s(%#x) %dx%d", v4l2MbusFmtName(format.format.code), format.format.code,
		format.format.width, format.format.height);

	struct v4l2_subdev_selection crop = {
		.pad = set->pad,
		.which = V4L2_SUBDEV_FORMAT_TRY,
		.target = V4L2_SEL_TGT_CROP_BOUNDS,
	};

	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_SELECTION, &crop)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_SELECTION, pad=%d): %d, %s", sd->fd, set->pad, errno, strerror(errno));
		return -1;
	}

	LOGI("Got selection crop bounds = (%d,%d) + (%dx%d)",
		crop.r.top, crop.r.left, crop.r.width, crop.r.height);

	if (crop.r.width > set->width || crop.r.height > set->height) {
		LOGI("Need to crop to %dx%d", set->width, set->height);
	}

	// Actually set things
	LOGI("Setting...");

	format.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_S_FMT, &format)) {
		const int err = errno;
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_S_FMT, pad=%d): %d, %s", sd->fd, set->pad, err, strerror(err));
		return err;
	}

	crop.which = V4L2_SUBDEV_FORMAT_ACTIVE;
	crop.target = V4L2_SEL_TGT_CROP;

	if (0 != ioctl(sd->fd, VIDIOC_SUBDEV_G_SELECTION, &crop)) {
		LOGE("Failed to ioctl(%d, VIDIOC_SUBDEV_G_SELECTION, pad=%d): %d, %s", sd->fd, set->pad, errno, strerror(errno));
		return -1;
	}

	// TODO set requested crop

	set->out_crop = crop.r;

	return 0;
}
