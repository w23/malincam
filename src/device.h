#pragma once

#include "V4l2Control.h"
#include "array.h"

#include <linux/videodev2.h>
#include <stdint.h> // uint32_t et al.

typedef struct Buffer {
	struct v4l2_buffer buffer;
	struct v4l2_plane planes[VIDEO_MAX_PLANES];
	void *mapped[VIDEO_MAX_PLANES];
	int dmabuf_fd[VIDEO_MAX_PLANES];
} Buffer;

typedef enum {
	BUFFER_MEMORY_NONE,
	BUFFER_MEMORY_MMAP,
	BUFFER_MEMORY_USERPTR,
	BUFFER_MEMORY_DMABUF_EXPORT,
	BUFFER_MEMORY_DMABUF_IMPORT,
} buffer_memory_e;

typedef enum {
	STREAM_STATE_IDLE = 0,
	STREAM_STATE_PREPARED,
	STREAM_STATE_STREAMING,
} StreamState;

typedef struct DeviceStream {
	int dev_fd;

	enum v4l2_buf_type type;
	uint32_t buffer_capabilities;

	StreamState state;

	struct v4l2_format format;
	struct v4l2_rect crop, compose;

	Array /*T(struct v4l2_fmtdesc)*/ formats;

	buffer_memory_e buffer_memory;
	int buffers_count;

	struct Buffer *buffers;
} DeviceStream;

#define IS_TYPE_MPLANE(type) \
	(((type) == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)||((type) == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE))

#define IS_TYPE_CAPTURE(type) \
	(((type) == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)||((type) == V4L2_BUF_TYPE_VIDEO_CAPTURE))

#define IS_STREAM_MPLANE(st) IS_TYPE_MPLANE((st)->type)
#define IS_STREAM_CAPTURE(st) IS_TYPE_CAPTURE((st)->type)

#define STREAM_PLANES_COUNT(st) \
	(IS_STREAM_MPLANE(st) ? (st)->format.fmt.pix_mp.num_planes : 1)

typedef struct Device {
	int fd;
	char *name;

	struct v4l2_capability caps;
	uint32_t this_device_caps;

	V4l2Controls controls;

	DeviceStream capture, output;
} Device;

Device* deviceOpen(const char *devname);
void deviceClose(Device* dev);

int deviceEventSubscribe(Device *dev, uint32_t event);

// Returns <0 on error, =0 on no events, =1 on event
int deviceEventGet(Device *dev, struct v4l2_event *out);

// TODO split into:
// - set format
// - streamon (buffers); streamoff should destroy buffers
typedef struct {
	buffer_memory_e buffer_memory;
	uint32_t buffers_count;

	uint32_t pixelformat;
	uint32_t width, height;

	uint32_t crop_width, crop_height;
} DeviceStreamPrepareOpts;

// @mbus_code is optional, set to 0 if not known
int deviceStreamQueryFormats(DeviceStream *st, int mbus_code);

int deviceStreamPrepare(DeviceStream *st, const DeviceStreamPrepareOpts *opts);
int deviceStreamStart(DeviceStream *st);
int deviceStreamStop(DeviceStream *st);

const Buffer *deviceStreamPullBuffer(DeviceStream *st);
int deviceStreamPushBuffer(DeviceStream *st, const Buffer *buf);
