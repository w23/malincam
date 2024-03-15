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
