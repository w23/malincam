#pragma once

#include "array.h"

#include <linux/videodev2.h>
#include <stdint.h> // uint32_t et al.

typedef struct Buffer {
	struct v4l2_buffer buffer;
	union {
		struct {
			void *ptr;
		} mmap;
		struct {
			int fd[VIDEO_MAX_PLANES];
		} dmabuf;
	} v;
} Buffer;

typedef enum {
	STREAM_STATE_IDLE = 0,
	STREAM_STATE_STREAMING,
} StreamState;

typedef struct DeviceStream {
	int dev_fd;

	enum v4l2_buf_type type;
	uint32_t buffer_capabilities;

	StreamState state;

	struct v4l2_format format;

	Array /*T(struct v4l2_fmtdesc)*/ formats;

	struct Buffer *buffers;
	int buffers_count;
} DeviceStream;

#define IS_STREAM_MPLANE(st) \
	(((st)->type&V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)||((st)->type&V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE))

#define IS_STREAM_CAPTURE(st) \
	(((st)->type&V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)||((st)->type&V4L2_BUF_TYPE_VIDEO_CAPTURE))

typedef struct Device {
	int fd;
	char *name;

	struct v4l2_capability caps;
	uint32_t this_device_caps;

	DeviceStream capture, output;
} Device;

struct Device* deviceOpen(const char *devname);
void deviceClose(struct Device* dev);

typedef struct DeviceStreamPrepareOpts {
	uint32_t memory_type;

	uint32_t buffers_count;

	uint32_t pixelformat;
	uint32_t width, height;
} DeviceStreamPrepareOpts;

// @mbus_code is optional
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
