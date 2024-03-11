#pragma once

#include "array.h"
#include "device.h"

#include <linux/v4l2-subdev.h>
#include <stdint.h>

typedef struct SubdevPad {
	struct v4l2_subdev_format format;
	struct v4l2_rect native, crop, crop_bounds, crop_default;
} SubdevPad;

typedef struct Subdev {
	int fd;

	struct v4l2_subdev_capability cap;
	//struct v4l2_dv_timings_cap timings_cap;

	SubdevPad *pads;
	int pads_count;

	V4l2Controls controls;
} Subdev;

//Subdev *subdevOpen(const char *name);
Subdev *subdevOpen(const char *name, int pads_count);
void subdevClose(Subdev *sd);

typedef struct SubdevSet {
	int pad;
	uint32_t mbus_code;
	uint32_t width, height;
	struct v4l2_rect /* TODO in*/out_crop;
} SubdevSet;

// Sets set.rect and mbus_code that it could set
int subdevSet(Subdev *sd, SubdevSet *set);

void v4l2PrintSubdevCapability(const struct v4l2_subdev_capability *cap);
void v4l2PrintSubdevFormat(const struct v4l2_subdev_format *format);
void v4l2PrintSubdevSelection(const struct v4l2_subdev_selection *sel);
void v4l2PrintFrameInterval(const struct v4l2_subdev_frame_interval *fi);
void v4l2PrintSubdevMbusCode(const struct v4l2_subdev_mbus_code_enum *mbc);
const char *v4l2MbusFmtName(uint32_t format);
void v4l2PrintSubdevFrameSize(const struct v4l2_subdev_frame_size_enum *fsz);
void v4l2PrintSubdevFrameInterval(const struct v4l2_subdev_frame_interval_enum *fiv);
const char* v4l2SelTgtName(uint32_t target);
void v4l2PrintSelFlags(uint32_t bits);
