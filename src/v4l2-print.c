#include "v4l2.h"
#include "common.h"

#define MCAM_V4L2_CAPS(X) \
	X(V4L2_CAP_VIDEO_CAPTURE) \
	X(V4L2_CAP_VIDEO_OUTPUT) \
	X(V4L2_CAP_VIDEO_OVERLAY) \
	X(V4L2_CAP_VBI_CAPTURE) \
	X(V4L2_CAP_VBI_OUTPUT) \
	X(V4L2_CAP_SLICED_VBI_CAPTURE) \
	X(V4L2_CAP_SLICED_VBI_OUTPUT) \
	X(V4L2_CAP_RDS_CAPTURE) \
	X(V4L2_CAP_VIDEO_OUTPUT_OVERLAY) \
	X(V4L2_CAP_HW_FREQ_SEEK) \
	X(V4L2_CAP_RDS_OUTPUT) \
	X(V4L2_CAP_VIDEO_CAPTURE_MPLANE) \
	X(V4L2_CAP_VIDEO_OUTPUT_MPLANE) \
	X(V4L2_CAP_VIDEO_M2M_MPLANE) \
	X(V4L2_CAP_VIDEO_M2M) \
	X(V4L2_CAP_TUNER) \
	X(V4L2_CAP_AUDIO) \
	X(V4L2_CAP_RADIO) \
	X(V4L2_CAP_MODULATOR) \
	X(V4L2_CAP_SDR_CAPTURE) \
	X(V4L2_CAP_EXT_PIX_FORMAT) \
	X(V4L2_CAP_SDR_OUTPUT) \
	X(V4L2_CAP_META_CAPTURE) \
	X(V4L2_CAP_READWRITE) \
	X(V4L2_CAP_STREAMING) \
	X(V4L2_CAP_META_OUTPUT) \
	X(V4L2_CAP_TOUCH) \
	X(V4L2_CAP_IO_MC) \
	X(V4L2_CAP_DEVICE_CAPS) \

void v4l2PrintCapabilityBits(uint32_t caps) {
#define X(cap) if (caps & cap) LOGI("  %s", #cap);
	MCAM_V4L2_CAPS(X)
#undef X
}

#define MCAM_V4L2_BUF_CAPS(X) \
	X(V4L2_BUF_CAP_SUPPORTS_MMAP) \
	X(V4L2_BUF_CAP_SUPPORTS_USERPTR) \
	X(V4L2_BUF_CAP_SUPPORTS_DMABUF) \
	X(V4L2_BUF_CAP_SUPPORTS_REQUESTS) \
	X(V4L2_BUF_CAP_SUPPORTS_ORPHANED_BUFS) \
	X(V4L2_BUF_CAP_SUPPORTS_M2M_HOLD_CAPTURE_BUF) \
	X(V4L2_BUF_CAP_SUPPORTS_MMAP_CACHE_HINTS) \

void v4l2PrintBufferCapabilityBits(uint32_t caps) {
#define X(cap) if (caps & cap) LOGI("  %s", #cap);
	MCAM_V4L2_BUF_CAPS(X)
#undef X
}

void v4l2PrintCapability(const struct v4l2_capability* caps) {
	LOGI("caps->driver = %s", caps->driver);
	LOGI("caps->card = %s", caps->card);
	LOGI("caps->bus_info = %s", caps->bus_info);
#define FROM_KERNEL_VERSION(v) (((v)>>16)&0xff), (((v)>>8)&0xff), ((v)&0xff)
	LOGI("caps->version = %d.%d.%d", FROM_KERNEL_VERSION(caps->version));
	LOGI("caps->capabilities = %08x:", caps->capabilities);
	v4l2PrintCapabilityBits(caps->capabilities);
	
	if (caps->capabilities & V4L2_CAP_DEVICE_CAPS) {
		LOGI("caps->device_caps = %08x:", caps->device_caps);
		v4l2PrintCapabilityBits(caps->device_caps);
	}
}

const char *v4l2InputTypeName(uint32_t type) {
	switch (type) {
		case V4L2_INPUT_TYPE_TUNER: return "V4L2_INPUT_TYPE_TUNER";
		case V4L2_INPUT_TYPE_CAMERA: return "V4L2_INPUT_TYPE_CAMERA";
		case V4L2_INPUT_TYPE_TOUCH: return "V4L2_INPUT_TYPE_TOUCH";
		default: return "UNKNOWN";
	}
}

const char *v4l2TunerTypeName(uint32_t type) {
	switch (type) {
		case V4L2_TUNER_RADIO: return "V4L2_TUNER_RADIO";
		case V4L2_TUNER_ANALOG_TV: return "V4L2_TUNER_ANALOG_TV";
		case V4L2_TUNER_DIGITAL_TV: return "V4L2_TUNER_DIGITAL_TV";
		case V4L2_TUNER_SDR: return "V4L2_TUNER_SDR";
		case V4L2_TUNER_RF: return "V4L2_TUNER_RF";
		default: return "UNKNOWN";
	}
}

#define MCAM_LIST_IN_ST(X) \
	X(V4L2_IN_ST_NO_POWER) \
	X(V4L2_IN_ST_NO_SIGNAL) \
	X(V4L2_IN_ST_NO_COLOR) \
	X(V4L2_IN_ST_HFLIP) \
	X(V4L2_IN_ST_VFLIP) \
	X(V4L2_IN_ST_NO_H_LOCK) \
	X(V4L2_IN_ST_COLOR_KILL) \
	X(V4L2_IN_ST_NO_V_LOCK) \
	X(V4L2_IN_ST_NO_STD_LOCK) \
	X(V4L2_IN_ST_NO_SYNC) \
	X(V4L2_IN_ST_NO_EQU) \
	X(V4L2_IN_ST_NO_CARRIER) \
	X(V4L2_IN_ST_MACROVISION) \
	X(V4L2_IN_ST_NO_ACCESS) \
	X(V4L2_IN_ST_VTR) \
	X(V4L2_IN_CAP_DV_TIMINGS) \
	X(V4L2_IN_CAP_DV_TIMINGS) \
	X(V4L2_IN_CAP_STD) \
	X(V4L2_IN_CAP_NATIVE_SIZE) \

void v4l2PrintInputStatusBits(uint32_t bits) {
#define X(bit) if (bits & bit) LOGI("  %s", #bit);
	MCAM_LIST_IN_ST(X)
#undef X
}

#define MCAM_LIST_IN_CAP(X) \
	X(V4L2_IN_CAP_DV_TIMINGS) \
	X(V4L2_IN_CAP_DV_TIMINGS) \
	X(V4L2_IN_CAP_STD) \
	X(V4L2_IN_CAP_NATIVE_SIZE) \

void v4l2PrintInputCapabilityBits(uint32_t bits) {
#define X(bit) if (bits & bit) LOGI("  %s", #bit);
	MCAM_LIST_IN_CAP(X)
#undef X
}

void v4l2PrintInput(const struct v4l2_input* input) {
	LOGI("  input.index = %d", input->index);
	LOGI("  input.name = %s", input->name);
	LOGI("  input.type = %s (%d)", v4l2InputTypeName(input->type), input->type);
	LOGI("  input.audioset = %08x", input->audioset);
	LOGI("  input.tuner = %s (%d)", v4l2TunerTypeName(input->tuner), input->tuner);
	LOGI("  input.std = %llx", (long long)input->std); // TODO
	LOGI("  input.status = %d:", input->status);
	v4l2PrintInputStatusBits(input->status);
	LOGI("  input.capabilities = %08x:", input->capabilities);
	v4l2PrintInputCapabilityBits(input->capabilities);
}

const char *v4l2BufTypeName(uint32_t type) {
	switch (type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE: return "V4L2_BUF_TYPE_VIDEO_CAPTURE";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT: return "V4L2_BUF_TYPE_VIDEO_OUTPUT";
		case V4L2_BUF_TYPE_VIDEO_OVERLAY: return "V4L2_BUF_TYPE_VIDEO_OVERLAY";
		case V4L2_BUF_TYPE_VBI_CAPTURE: return "V4L2_BUF_TYPE_VBI_CAPTURE";
		case V4L2_BUF_TYPE_VBI_OUTPUT: return "V4L2_BUF_TYPE_VBI_OUTPUT";
		case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE: return "V4L2_BUF_TYPE_SLICED_VBI_CAPTURE";
		case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT: return "V4L2_BUF_TYPE_SLICED_VBI_OUTPUT";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY: return "V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY";
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE: return "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE";
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE: return "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE";
		case V4L2_BUF_TYPE_SDR_CAPTURE: return "V4L2_BUF_TYPE_SDR_CAPTURE";
		case V4L2_BUF_TYPE_SDR_OUTPUT: return "V4L2_BUF_TYPE_SDR_OUTPUT";
		case V4L2_BUF_TYPE_META_CAPTURE: return "V4L2_BUF_TYPE_META_CAPTURE";
		case V4L2_BUF_TYPE_META_OUTPUT: return "V4L2_BUF_TYPE_META_OUTPUT";
		default: return "UNKNOWN";
	}
}

void v4l2PrintFormatDesc(const struct v4l2_fmtdesc* fmt) {
	LOGI("  fmt.flags = %08x", fmt->flags);
	LOGI("  fmt.description = %s", fmt->description);
	LOGI("  fmt.pixelformat = %08x", fmt->pixelformat);
}

void v4l2PrintFormat(const struct v4l2_format* fmt) {
#define _(v) (v)
#define FIELDS(X) \
	X(width, "%u", _) \
	X(height, "%u", _) \
	X(field, "%08x", _) \
	X(colorspace, "%08x", _) \
	X(num_planes, "%08x", _) \
	X(flags, "%08x", _) \
	X(quantization, "%08x", _) \
	X(xfer_func, "%08x", _) \

#define X(name, ffmt, func) \
	LOGI("fmt.pix_mp." # name " = " ffmt, func(fmt->fmt.pix_mp.name));
	FIELDS(X)
#undef X
#undef _

	for (int i = 0; i < fmt->fmt.pix_mp.num_planes; ++i) {
		LOGI("fmt.pix_mp.plane_fmt[%d].sizeimage = %d", i, fmt->fmt.pix_mp.plane_fmt[i].sizeimage);
		LOGI("fmt.pix_mp.plane_fmt[%d].bytesperline = %d", i, fmt->fmt.pix_mp.plane_fmt[i].bytesperline);
	}
}

void v4l2PrintRequestBuffers(const struct v4l2_requestbuffers* req) {
	LOGI("req.count = %d", req->count);
	v4l2PrintBufferCapabilityBits(req->capabilities);
	LOGI("req.flags = %08x", req->flags);
}
