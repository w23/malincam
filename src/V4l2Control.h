#pragma once

#include "array.h"

#include <linux/videodev2.h>
#include <stdint.h> // uint32_t et al.

typedef struct V4l2Control {
	struct v4l2_query_ext_ctrl query;
	//struct v4l2_ext_control value;
	int64_t value;
} V4l2Control;

typedef struct {
	int fd;
	Array controls;
} V4l2Controls;

// Enumerates controls (ext) for a given fd, device or subdevice
// Returns Array<V4l2Control>
V4l2Controls v4l2ControlsCreateFromV4l2Fd(int fd);
void v4l2ControlsDestroy(V4l2Controls *controls);

// Returns:
// - -ENOENT on no such control
// - -ERANGE on value outside of range
// - -EPERM on unsupported ctrl type
// - other values on ioctl() errors
// - 0 on success
int v4l2ControlSetById(V4l2Controls *ctrls, uint32_t ctrl_id, int64_t value);
int v4l2ControlSet(V4l2Controls *ctrls, V4l2Control *ctrl, int64_t value);

// NULL if not found
V4l2Control *v4l2ControlGet(V4l2Controls *ctrls, uint32_t ctrl);
