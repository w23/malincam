#pragma once

#include <linux/v4l2-subdev.h>

typedef struct Subdev {
	int fd;

	struct v4l2_subdev_capability cap;
	//struct v4l2_dv_timings_cap timings_cap;

	struct {
		struct v4l2_subdev_format format;
		struct v4l2_subdev_selection selection;
	} pads[1];
} Subdev;

//Subdev *subdevOpen(const char *name);
Subdev *subdevOpen(const char *name, int pads_count);
void subdevClose(Subdev *sd);

void v4l2PrintSubdevCapability(const struct v4l2_subdev_capability *cap);
void v4l2PrintSubdevFormat(const struct v4l2_subdev_format *format);
void v4l2PrintSubdevSelection(const struct v4l2_subdev_selection *sel);
void v4l2PrintFrameInterval(const struct v4l2_subdev_frame_interval *fi);
void v4l2PrintSubdevMbusCode(const struct v4l2_subdev_mbus_code_enum *mbc);
