#pragma once

#include <linux/videodev2.h>

#include <stdint.h> // uint32_t et al.

#include "array.h"

typedef struct Buffer {
	struct v4l2_buffer buffer;
	union {
		struct {
			void *ptr;
		} mmap;
	} v;
} Buffer;

typedef struct Endpoint {
	uint32_t type;
	uint32_t buffer_capabilities;
	struct v4l2_format format;
	Array /*T(struct v4l2_fmtdesc)*/ formats;
} Endpoint;

typedef struct DeviceV4L2 {
	int fd;
	char *name;

	struct v4l2_capability caps;
	uint32_t this_device_caps;

	Array endpoints;

	struct v4l2_format fmt;

	struct Buffer *buffers;
	int buffers_count;
} DeviceV4L2;

struct DeviceV4L2* devV4L2Open(const char *devname);
void devV4L2Close(struct DeviceV4L2* dev);

typedef int (got_buffer_func)(void *userptr, struct v4l2_buffer *buf);

typedef struct V4L2PrepareOpts {
	uint32_t buffer_type;
	uint32_t buffers_count;
	uint32_t memory_type;

	uint32_t width, height;

	//void *userptr;
	//got_buffer_func *buffer_func;

} V4L2PrepareOpts;

int devV4L2Start(struct DeviceV4L2 *dev, const V4L2PrepareOpts *opts);
void devV4L2Stop(struct DeviceV4L2 *dev);

// TODO Stream interface?

int devV4L2PushBuffer(struct DeviceV4L2 *dev, const Buffer *buf);
const Buffer *devV4L2PullBuffer(struct DeviceV4L2 *dev);

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
