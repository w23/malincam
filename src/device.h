#pragma once

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
V4l2Controls v4l2ControlsCreate(int fd);
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

void v4l2PrintCapabilityBits(uint32_t caps);
void v4l2PrintBufferCapabilityBits(uint32_t caps);
void v4l2PrintCapability(const struct v4l2_capability* caps);
const char *v4l2InputTypeName(uint32_t type);
void v4l2PrintInput(const struct v4l2_input* input);
const char *v4l2BufTypeName(uint32_t type);
void v4l2PrintFormatDesc(const struct v4l2_fmtdesc* fmt);
void v4l2PrintFormat(const struct v4l2_format* fmt);
void v4l2PrintRequestBuffers(const struct v4l2_requestbuffers* req);
const char *v4l2MemoryTypeName(enum v4l2_memory type);
void v4l2PrintBuffer(const struct v4l2_buffer *buf);
void v4l2PrintFormatFlags(uint32_t flags);
const char *v4l2PixFmtName(uint32_t fmt);
void v4l2PrintFrmSizeEnum(const struct v4l2_frmsizeenum *fse);
const char *v4l2MbusFmtName(uint32_t format);
void v4l2PrintSelection(const struct v4l2_selection* sel);
const char* v4l2SelTgtName(uint32_t target);
const char* v4l2CtrlIdName(uint32_t ctrl_id);
const char* v4l2CtrlTypeName(uint32_t ctrl_type);
void v4l2PrintControlFlags(uint32_t flags);
