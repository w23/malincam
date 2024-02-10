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
	} v;
} Buffer;

typedef enum {
	ENDPOINT_STATE_IDLE = 0,
	ENDPOINT_STATE_STREAMING,
} EndpointState;

typedef struct DeviceEndpoint {
	int dev_fd;

	uint32_t type;
	uint32_t buffer_capabilities;

	EndpointState state;

	struct v4l2_format format;

	Array /*T(struct v4l2_fmtdesc)*/ formats;

	struct Buffer *buffers;
	int buffers_count;
} DeviceEndpoint;

typedef struct Device {
	int fd;
	char *name;

	struct v4l2_capability caps;
	uint32_t this_device_caps;

	DeviceEndpoint capture, output;
} Device;

struct Device* deviceOpen(const char *devname);
void deviceClose(struct Device* dev);

//typedef int (got_buffer_func)(void *userptr, struct v4l2_buffer *buf);

typedef struct DeviceEndpointPrepareOpts {
	uint32_t memory_type;

	uint32_t buffers_count;

	uint32_t pixelformat;
	uint32_t width, height;

	//void *userptr;
	//got_buffer_func *buffer_func;

} DeviceEndpointPrepareOpts;

int deviceEndpointStart(DeviceEndpoint *ep, const DeviceEndpointPrepareOpts *opts);
int deviceEndpointStop(DeviceEndpoint *ep);

const Buffer *deviceEndpointPullBuffer(DeviceEndpoint *ep);
int deviceEndpointPushBuffer(DeviceEndpoint *ep, const Buffer *buf);

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
