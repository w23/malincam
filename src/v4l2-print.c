#include "device.h"
#include "subdev.h"

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

#define MCAM_LIST_FMT_FLAGS(X) \
	X(V4L2_FMT_FLAG_COMPRESSED) \
	X(V4L2_FMT_FLAG_EMULATED) \
	X(V4L2_FMT_FLAG_CONTINUOUS_BYTESTREAM) \
	X(V4L2_FMT_FLAG_DYN_RESOLUTION) \
	X(V4L2_FMT_FLAG_ENC_CAP_FRAME_INTERVAL) \
	X(V4L2_FMT_FLAG_CSC_COLORSPACE) \
	X(V4L2_FMT_FLAG_CSC_XFER_FUNC) \
	X(V4L2_FMT_FLAG_CSC_YCBCR_ENC) \
	X(V4L2_FMT_FLAG_CSC_YCBCR_ENC) \
	X(V4L2_FMT_FLAG_CSC_QUANTIZATION) \

void v4l2PrintFormatFlags(uint32_t flags) {
#define X(bit) if (flags & bit) LOGI("    %s", #bit);
	MCAM_LIST_FMT_FLAGS(X)
#undef X
}

const char *v4l2PixFmtName(uint32_t fmt) {
	switch (fmt) {
		case V4L2_PIX_FMT_RGB332: return "V4L2_PIX_FMT_RGB332";
		case V4L2_PIX_FMT_RGB444: return "V4L2_PIX_FMT_RGB444";
		case V4L2_PIX_FMT_ARGB444: return "V4L2_PIX_FMT_ARGB444";
		case V4L2_PIX_FMT_XRGB444: return "V4L2_PIX_FMT_XRGB444";
		case V4L2_PIX_FMT_RGBA444: return "V4L2_PIX_FMT_RGBA444";
		case V4L2_PIX_FMT_RGBX444: return "V4L2_PIX_FMT_RGBX444";
		case V4L2_PIX_FMT_ABGR444: return "V4L2_PIX_FMT_ABGR444";
		case V4L2_PIX_FMT_XBGR444: return "V4L2_PIX_FMT_XBGR444";
		case V4L2_PIX_FMT_BGRA444: return "V4L2_PIX_FMT_BGRA444";
		case V4L2_PIX_FMT_BGRX444: return "V4L2_PIX_FMT_BGRX444";
		case V4L2_PIX_FMT_RGB555: return "V4L2_PIX_FMT_RGB555";
		case V4L2_PIX_FMT_ARGB555: return "V4L2_PIX_FMT_ARGB555";
		case V4L2_PIX_FMT_XRGB555: return "V4L2_PIX_FMT_XRGB555";
		case V4L2_PIX_FMT_RGBA555: return "V4L2_PIX_FMT_RGBA555";
		case V4L2_PIX_FMT_RGBX555: return "V4L2_PIX_FMT_RGBX555";
		case V4L2_PIX_FMT_ABGR555: return "V4L2_PIX_FMT_ABGR555";
		case V4L2_PIX_FMT_XBGR555: return "V4L2_PIX_FMT_XBGR555";
		case V4L2_PIX_FMT_BGRA555: return "V4L2_PIX_FMT_BGRA555";
		case V4L2_PIX_FMT_BGRX555: return "V4L2_PIX_FMT_BGRX555";
		case V4L2_PIX_FMT_RGB565: return "V4L2_PIX_FMT_RGB565";
		case V4L2_PIX_FMT_RGB555X: return "V4L2_PIX_FMT_RGB555X";
		case V4L2_PIX_FMT_ARGB555X: return "V4L2_PIX_FMT_ARGB555X";
		case V4L2_PIX_FMT_XRGB555X: return "V4L2_PIX_FMT_XRGB555X";
		case V4L2_PIX_FMT_RGB565X: return "V4L2_PIX_FMT_RGB565X";
		case V4L2_PIX_FMT_BGR666: return "V4L2_PIX_FMT_BGR666";
		case V4L2_PIX_FMT_BGR24: return "V4L2_PIX_FMT_BGR24";
		case V4L2_PIX_FMT_RGB24: return "V4L2_PIX_FMT_RGB24";
		case V4L2_PIX_FMT_BGR32: return "V4L2_PIX_FMT_BGR32";
		case V4L2_PIX_FMT_ABGR32: return "V4L2_PIX_FMT_ABGR32";
		case V4L2_PIX_FMT_XBGR32: return "V4L2_PIX_FMT_XBGR32";
		case V4L2_PIX_FMT_BGRA32: return "V4L2_PIX_FMT_BGRA32";
		case V4L2_PIX_FMT_BGRX32: return "V4L2_PIX_FMT_BGRX32";
		case V4L2_PIX_FMT_RGB32: return "V4L2_PIX_FMT_RGB32";
		case V4L2_PIX_FMT_RGBA32: return "V4L2_PIX_FMT_RGBA32";
		case V4L2_PIX_FMT_RGBX32: return "V4L2_PIX_FMT_RGBX32";
		case V4L2_PIX_FMT_ARGB32: return "V4L2_PIX_FMT_ARGB32";
		case V4L2_PIX_FMT_XRGB32: return "V4L2_PIX_FMT_XRGB32";
		case V4L2_PIX_FMT_GREY: return "V4L2_PIX_FMT_GREY";
		case V4L2_PIX_FMT_Y4: return "V4L2_PIX_FMT_Y4";
		case V4L2_PIX_FMT_Y6: return "V4L2_PIX_FMT_Y6";
		case V4L2_PIX_FMT_Y10: return "V4L2_PIX_FMT_Y10";
		case V4L2_PIX_FMT_Y12: return "V4L2_PIX_FMT_Y12";
		case V4L2_PIX_FMT_Y14: return "V4L2_PIX_FMT_Y14";
		case V4L2_PIX_FMT_Y16: return "V4L2_PIX_FMT_Y16";
		case V4L2_PIX_FMT_Y16_BE: return "V4L2_PIX_FMT_Y16_BE";
		case V4L2_PIX_FMT_Y10BPACK: return "V4L2_PIX_FMT_Y10BPACK";
		case V4L2_PIX_FMT_Y10P: return "V4L2_PIX_FMT_Y10P";
		case V4L2_PIX_FMT_IPU3_Y10: return "V4L2_PIX_FMT_IPU3_Y10";
		case V4L2_PIX_FMT_PAL8: return "V4L2_PIX_FMT_PAL8";
		case V4L2_PIX_FMT_UV8: return "V4L2_PIX_FMT_UV8";
		case V4L2_PIX_FMT_YUYV: return "V4L2_PIX_FMT_YUYV";
		case V4L2_PIX_FMT_YYUV: return "V4L2_PIX_FMT_YYUV";
		case V4L2_PIX_FMT_YVYU: return "V4L2_PIX_FMT_YVYU";
		case V4L2_PIX_FMT_UYVY: return "V4L2_PIX_FMT_UYVY";
		case V4L2_PIX_FMT_VYUY: return "V4L2_PIX_FMT_VYUY";
		case V4L2_PIX_FMT_Y41P: return "V4L2_PIX_FMT_Y41P";
		case V4L2_PIX_FMT_YUV444: return "V4L2_PIX_FMT_YUV444";
		case V4L2_PIX_FMT_YUV555: return "V4L2_PIX_FMT_YUV555";
		case V4L2_PIX_FMT_YUV565: return "V4L2_PIX_FMT_YUV565";
		case V4L2_PIX_FMT_YUV24: return "V4L2_PIX_FMT_YUV24";
		case V4L2_PIX_FMT_YUV32: return "V4L2_PIX_FMT_YUV32";
		case V4L2_PIX_FMT_AYUV32: return "V4L2_PIX_FMT_AYUV32";
		case V4L2_PIX_FMT_XYUV32: return "V4L2_PIX_FMT_XYUV32";
		case V4L2_PIX_FMT_VUYA32: return "V4L2_PIX_FMT_VUYA32";
		case V4L2_PIX_FMT_VUYX32: return "V4L2_PIX_FMT_VUYX32";
		case V4L2_PIX_FMT_YUVA32: return "V4L2_PIX_FMT_YUVA32";
		case V4L2_PIX_FMT_YUVX32: return "V4L2_PIX_FMT_YUVX32";
		case V4L2_PIX_FMT_M420: return "V4L2_PIX_FMT_M420";
		case V4L2_PIX_FMT_NV12: return "V4L2_PIX_FMT_NV12";
		case V4L2_PIX_FMT_NV21: return "V4L2_PIX_FMT_NV21";
		case V4L2_PIX_FMT_NV16: return "V4L2_PIX_FMT_NV16";
		case V4L2_PIX_FMT_NV61: return "V4L2_PIX_FMT_NV61";
		case V4L2_PIX_FMT_NV24: return "V4L2_PIX_FMT_NV24";
		case V4L2_PIX_FMT_NV42: return "V4L2_PIX_FMT_NV42";
		case V4L2_PIX_FMT_P010: return "V4L2_PIX_FMT_P010";
		case V4L2_PIX_FMT_NV12M: return "V4L2_PIX_FMT_NV12M";
		case V4L2_PIX_FMT_NV21M: return "V4L2_PIX_FMT_NV21M";
		case V4L2_PIX_FMT_NV16M: return "V4L2_PIX_FMT_NV16M";
		case V4L2_PIX_FMT_NV61M: return "V4L2_PIX_FMT_NV61M";
		case V4L2_PIX_FMT_YUV410: return "V4L2_PIX_FMT_YUV410";
		case V4L2_PIX_FMT_YVU410: return "V4L2_PIX_FMT_YVU410";
		case V4L2_PIX_FMT_YUV411P: return "V4L2_PIX_FMT_YUV411P";
		case V4L2_PIX_FMT_YUV420: return "V4L2_PIX_FMT_YUV420";
		case V4L2_PIX_FMT_YVU420: return "V4L2_PIX_FMT_YVU420";
		case V4L2_PIX_FMT_YUV422P: return "V4L2_PIX_FMT_YUV422P";
		case V4L2_PIX_FMT_YUV420M: return "V4L2_PIX_FMT_YUV420M";
		case V4L2_PIX_FMT_YVU420M: return "V4L2_PIX_FMT_YVU420M";
		case V4L2_PIX_FMT_YUV422M: return "V4L2_PIX_FMT_YUV422M";
		case V4L2_PIX_FMT_YVU422M: return "V4L2_PIX_FMT_YVU422M";
		case V4L2_PIX_FMT_YUV444M: return "V4L2_PIX_FMT_YUV444M";
		case V4L2_PIX_FMT_YVU444M: return "V4L2_PIX_FMT_YVU444M";
		case V4L2_PIX_FMT_NV12_4L4: return "V4L2_PIX_FMT_NV12_4L4";
		case V4L2_PIX_FMT_NV12_16L16: return "V4L2_PIX_FMT_NV12_16L16";
		case V4L2_PIX_FMT_NV12_32L32: return "V4L2_PIX_FMT_NV12_32L32";
		case V4L2_PIX_FMT_P010_4L4: return "V4L2_PIX_FMT_P010_4L4";
		case V4L2_PIX_FMT_NV12MT: return "V4L2_PIX_FMT_NV12MT";
		case V4L2_PIX_FMT_NV12MT_16X16: return "V4L2_PIX_FMT_NV12MT_16X16";
		case V4L2_PIX_FMT_NV12M_8L128: return "V4L2_PIX_FMT_NV12M_8L128";
		case V4L2_PIX_FMT_NV12M_10BE_8L128: return "V4L2_PIX_FMT_NV12M_10BE_8L128";
		case V4L2_PIX_FMT_SBGGR8: return "V4L2_PIX_FMT_SBGGR8";
		case V4L2_PIX_FMT_SGBRG8: return "V4L2_PIX_FMT_SGBRG8";
		case V4L2_PIX_FMT_SGRBG8: return "V4L2_PIX_FMT_SGRBG8";
		case V4L2_PIX_FMT_SRGGB8: return "V4L2_PIX_FMT_SRGGB8";
		case V4L2_PIX_FMT_SBGGR10: return "V4L2_PIX_FMT_SBGGR10";
		case V4L2_PIX_FMT_SGBRG10: return "V4L2_PIX_FMT_SGBRG10";
		case V4L2_PIX_FMT_SGRBG10: return "V4L2_PIX_FMT_SGRBG10";
		case V4L2_PIX_FMT_SRGGB10: return "V4L2_PIX_FMT_SRGGB10";
		case V4L2_PIX_FMT_SBGGR10P: return "V4L2_PIX_FMT_SBGGR10P";
		case V4L2_PIX_FMT_SGBRG10P: return "V4L2_PIX_FMT_SGBRG10P";
		case V4L2_PIX_FMT_SGRBG10P: return "V4L2_PIX_FMT_SGRBG10P";
		case V4L2_PIX_FMT_SRGGB10P: return "V4L2_PIX_FMT_SRGGB10P";
		case V4L2_PIX_FMT_SBGGR10ALAW8: return "V4L2_PIX_FMT_SBGGR10ALAW8";
		case V4L2_PIX_FMT_SGBRG10ALAW8: return "V4L2_PIX_FMT_SGBRG10ALAW8";
		case V4L2_PIX_FMT_SGRBG10ALAW8: return "V4L2_PIX_FMT_SGRBG10ALAW8";
		case V4L2_PIX_FMT_SRGGB10ALAW8: return "V4L2_PIX_FMT_SRGGB10ALAW8";
		case V4L2_PIX_FMT_SBGGR10DPCM8: return "V4L2_PIX_FMT_SBGGR10DPCM8";
		case V4L2_PIX_FMT_SGBRG10DPCM8: return "V4L2_PIX_FMT_SGBRG10DPCM8";
		case V4L2_PIX_FMT_SGRBG10DPCM8: return "V4L2_PIX_FMT_SGRBG10DPCM8";
		case V4L2_PIX_FMT_SRGGB10DPCM8: return "V4L2_PIX_FMT_SRGGB10DPCM8";
		case V4L2_PIX_FMT_SBGGR12: return "V4L2_PIX_FMT_SBGGR12";
		case V4L2_PIX_FMT_SGBRG12: return "V4L2_PIX_FMT_SGBRG12";
		case V4L2_PIX_FMT_SGRBG12: return "V4L2_PIX_FMT_SGRBG12";
		case V4L2_PIX_FMT_SRGGB12: return "V4L2_PIX_FMT_SRGGB12";
		case V4L2_PIX_FMT_SBGGR12P: return "V4L2_PIX_FMT_SBGGR12P";
		case V4L2_PIX_FMT_SGBRG12P: return "V4L2_PIX_FMT_SGBRG12P";
		case V4L2_PIX_FMT_SGRBG12P: return "V4L2_PIX_FMT_SGRBG12P";
		case V4L2_PIX_FMT_SRGGB12P: return "V4L2_PIX_FMT_SRGGB12P";
		case V4L2_PIX_FMT_SBGGR14: return "V4L2_PIX_FMT_SBGGR14";
		case V4L2_PIX_FMT_SGBRG14: return "V4L2_PIX_FMT_SGBRG14";
		case V4L2_PIX_FMT_SGRBG14: return "V4L2_PIX_FMT_SGRBG14";
		case V4L2_PIX_FMT_SRGGB14: return "V4L2_PIX_FMT_SRGGB14";
		case V4L2_PIX_FMT_SBGGR14P: return "V4L2_PIX_FMT_SBGGR14P";
		case V4L2_PIX_FMT_SGBRG14P: return "V4L2_PIX_FMT_SGBRG14P";
		case V4L2_PIX_FMT_SGRBG14P: return "V4L2_PIX_FMT_SGRBG14P";
		case V4L2_PIX_FMT_SRGGB14P: return "V4L2_PIX_FMT_SRGGB14P";
		case V4L2_PIX_FMT_SBGGR16: return "V4L2_PIX_FMT_SBGGR16";
		case V4L2_PIX_FMT_SGBRG16: return "V4L2_PIX_FMT_SGBRG16";
		case V4L2_PIX_FMT_SGRBG16: return "V4L2_PIX_FMT_SGRBG16";
		case V4L2_PIX_FMT_SRGGB16: return "V4L2_PIX_FMT_SRGGB16";
		case V4L2_PIX_FMT_HSV24: return "V4L2_PIX_FMT_HSV24";
		case V4L2_PIX_FMT_HSV32: return "V4L2_PIX_FMT_HSV32";
		case V4L2_PIX_FMT_MJPEG: return "V4L2_PIX_FMT_MJPEG";
		case V4L2_PIX_FMT_JPEG: return "V4L2_PIX_FMT_JPEG";
		case V4L2_PIX_FMT_DV: return "V4L2_PIX_FMT_DV";
		case V4L2_PIX_FMT_MPEG: return "V4L2_PIX_FMT_MPEG";
		case V4L2_PIX_FMT_H264: return "V4L2_PIX_FMT_H264";
		case V4L2_PIX_FMT_H264_NO_SC: return "V4L2_PIX_FMT_H264_NO_SC";
		case V4L2_PIX_FMT_H264_MVC: return "V4L2_PIX_FMT_H264_MVC";
		case V4L2_PIX_FMT_H263: return "V4L2_PIX_FMT_H263";
		case V4L2_PIX_FMT_MPEG1: return "V4L2_PIX_FMT_MPEG1";
		case V4L2_PIX_FMT_MPEG2: return "V4L2_PIX_FMT_MPEG2";
		case V4L2_PIX_FMT_MPEG2_SLICE: return "V4L2_PIX_FMT_MPEG2_SLICE";
		case V4L2_PIX_FMT_MPEG4: return "V4L2_PIX_FMT_MPEG4";
		case V4L2_PIX_FMT_XVID: return "V4L2_PIX_FMT_XVID";
		case V4L2_PIX_FMT_VC1_ANNEX_G: return "V4L2_PIX_FMT_VC1_ANNEX_G";
		case V4L2_PIX_FMT_VC1_ANNEX_L: return "V4L2_PIX_FMT_VC1_ANNEX_L";
		case V4L2_PIX_FMT_VP8: return "V4L2_PIX_FMT_VP8";
		case V4L2_PIX_FMT_VP8_FRAME: return "V4L2_PIX_FMT_VP8_FRAME";
		case V4L2_PIX_FMT_VP9: return "V4L2_PIX_FMT_VP9";
		case V4L2_PIX_FMT_VP9_FRAME: return "V4L2_PIX_FMT_VP9_FRAME";
		case V4L2_PIX_FMT_HEVC: return "V4L2_PIX_FMT_HEVC";
		case V4L2_PIX_FMT_FWHT: return "V4L2_PIX_FMT_FWHT";
		case V4L2_PIX_FMT_FWHT_STATELESS: return "V4L2_PIX_FMT_FWHT_STATELESS";
		case V4L2_PIX_FMT_H264_SLICE: return "V4L2_PIX_FMT_H264_SLICE";
		case V4L2_PIX_FMT_HEVC_SLICE: return "V4L2_PIX_FMT_HEVC_SLICE";
		case V4L2_PIX_FMT_CPIA1: return "V4L2_PIX_FMT_CPIA1";
		case V4L2_PIX_FMT_WNVA: return "V4L2_PIX_FMT_WNVA";
		case V4L2_PIX_FMT_SN9C10X: return "V4L2_PIX_FMT_SN9C10X";
		case V4L2_PIX_FMT_SN9C20X_I420: return "V4L2_PIX_FMT_SN9C20X_I420";
		case V4L2_PIX_FMT_PWC1: return "V4L2_PIX_FMT_PWC1";
		case V4L2_PIX_FMT_PWC2: return "V4L2_PIX_FMT_PWC2";
		case V4L2_PIX_FMT_ET61X251: return "V4L2_PIX_FMT_ET61X251";
		case V4L2_PIX_FMT_SPCA501: return "V4L2_PIX_FMT_SPCA501";
		case V4L2_PIX_FMT_SPCA505: return "V4L2_PIX_FMT_SPCA505";
		case V4L2_PIX_FMT_SPCA508: return "V4L2_PIX_FMT_SPCA508";
		case V4L2_PIX_FMT_SPCA561: return "V4L2_PIX_FMT_SPCA561";
		case V4L2_PIX_FMT_PAC207: return "V4L2_PIX_FMT_PAC207";
		case V4L2_PIX_FMT_MR97310A: return "V4L2_PIX_FMT_MR97310A";
		case V4L2_PIX_FMT_JL2005BCD: return "V4L2_PIX_FMT_JL2005BCD";
		case V4L2_PIX_FMT_SN9C2028: return "V4L2_PIX_FMT_SN9C2028";
		case V4L2_PIX_FMT_SQ905C: return "V4L2_PIX_FMT_SQ905C";
		case V4L2_PIX_FMT_PJPG: return "V4L2_PIX_FMT_PJPG";
		case V4L2_PIX_FMT_OV511: return "V4L2_PIX_FMT_OV511";
		case V4L2_PIX_FMT_OV518: return "V4L2_PIX_FMT_OV518";
		case V4L2_PIX_FMT_STV0680: return "V4L2_PIX_FMT_STV0680";
		case V4L2_PIX_FMT_TM6000: return "V4L2_PIX_FMT_TM6000";
		case V4L2_PIX_FMT_CIT_YYVYUY: return "V4L2_PIX_FMT_CIT_YYVYUY";
		case V4L2_PIX_FMT_KONICA420: return "V4L2_PIX_FMT_KONICA420";
		case V4L2_PIX_FMT_JPGL: return "V4L2_PIX_FMT_JPGL";
		case V4L2_PIX_FMT_SE401: return "V4L2_PIX_FMT_SE401";
		case V4L2_PIX_FMT_S5C_UYVY_JPG: return "V4L2_PIX_FMT_S5C_UYVY_JPG";
		case V4L2_PIX_FMT_Y8I: return "V4L2_PIX_FMT_Y8I";
		case V4L2_PIX_FMT_Y12I: return "V4L2_PIX_FMT_Y12I";
		case V4L2_PIX_FMT_Z16: return "V4L2_PIX_FMT_Z16";
		case V4L2_PIX_FMT_MT21C: return "V4L2_PIX_FMT_MT21C";
		case V4L2_PIX_FMT_MM21: return "V4L2_PIX_FMT_MM21";
		case V4L2_PIX_FMT_INZI: return "V4L2_PIX_FMT_INZI";
		case V4L2_PIX_FMT_CNF4: return "V4L2_PIX_FMT_CNF4";
		case V4L2_PIX_FMT_HI240: return "V4L2_PIX_FMT_HI240";
		case V4L2_PIX_FMT_QC08C: return "V4L2_PIX_FMT_QC08C";
		case V4L2_PIX_FMT_QC10C: return "V4L2_PIX_FMT_QC10C";
		case V4L2_PIX_FMT_IPU3_SBGGR10: return "V4L2_PIX_FMT_IPU3_SBGGR10";
		case V4L2_PIX_FMT_IPU3_SGBRG10: return "V4L2_PIX_FMT_IPU3_SGBRG10";
		case V4L2_PIX_FMT_IPU3_SGRBG10: return "V4L2_PIX_FMT_IPU3_SGRBG10";
		case V4L2_PIX_FMT_IPU3_SRGGB10: return "V4L2_PIX_FMT_IPU3_SRGGB10";
		case V4L2_SDR_FMT_CU8: return "V4L2_SDR_FMT_CU8";
		case V4L2_SDR_FMT_CU16LE: return "V4L2_SDR_FMT_CU16LE";
		case V4L2_SDR_FMT_CS8: return "V4L2_SDR_FMT_CS8";
		case V4L2_SDR_FMT_CS14LE: return "V4L2_SDR_FMT_CS14LE";
		case V4L2_SDR_FMT_RU12LE: return "V4L2_SDR_FMT_RU12LE";
		case V4L2_SDR_FMT_PCU16BE: return "V4L2_SDR_FMT_PCU16BE";
		case V4L2_SDR_FMT_PCU18BE: return "V4L2_SDR_FMT_PCU18BE";
		case V4L2_SDR_FMT_PCU20BE: return "V4L2_SDR_FMT_PCU20BE";
		case V4L2_TCH_FMT_DELTA_TD16: return "V4L2_TCH_FMT_DELTA_TD16";
		case V4L2_TCH_FMT_DELTA_TD08: return "V4L2_TCH_FMT_DELTA_TD08";
		case V4L2_TCH_FMT_TU16: return "V4L2_TCH_FMT_TU16";
		case V4L2_TCH_FMT_TU08: return "V4L2_TCH_FMT_TU08";
		case V4L2_META_FMT_VSP1_HGO: return "V4L2_META_FMT_VSP1_HGO";
		case V4L2_META_FMT_VSP1_HGT: return "V4L2_META_FMT_VSP1_HGT";
		case V4L2_META_FMT_UVC: return "V4L2_META_FMT_UVC";
		case V4L2_META_FMT_D4XX: return "V4L2_META_FMT_D4XX";
		case V4L2_META_FMT_VIVID: return "V4L2_META_FMT_VIVID";
		case V4L2_META_FMT_RK_ISP1_PARAMS: return "V4L2_META_FMT_RK_ISP1_PARAMS";
		case V4L2_META_FMT_RK_ISP1_STAT_3A: return "V4L2_META_FMT_RK_ISP1_STAT_3A";
		case V4L2_PIX_FMT_PRIV_MAGIC: return "V4L2_PIX_FMT_PRIV_MAGIC";
		default: return "UNKNOWN";
	}
}

const char *v4l2ColorspaceName(enum v4l2_colorspace colorspace) {
	switch (colorspace) {
		case V4L2_COLORSPACE_DEFAULT: return "V4L2_COLORSPACE_DEFAULT";
		case V4L2_COLORSPACE_SMPTE170M: return "V4L2_COLORSPACE_SMPTE170M";
		case V4L2_COLORSPACE_SMPTE240M: return "V4L2_COLORSPACE_SMPTE240M";
		case V4L2_COLORSPACE_REC709: return "V4L2_COLORSPACE_REC709";
		case V4L2_COLORSPACE_BT878: return "V4L2_COLORSPACE_BT878";
		case V4L2_COLORSPACE_470_SYSTEM_M: return "V4L2_COLORSPACE_470_SYSTEM_M";
		case V4L2_COLORSPACE_470_SYSTEM_BG: return "V4L2_COLORSPACE_470_SYSTEM_BG";
		case V4L2_COLORSPACE_JPEG: return "V4L2_COLORSPACE_JPEG";
		case V4L2_COLORSPACE_SRGB: return "V4L2_COLORSPACE_SRGB";
		case V4L2_COLORSPACE_OPRGB: return "V4L2_COLORSPACE_OPRGB";
		case V4L2_COLORSPACE_BT2020: return "V4L2_COLORSPACE_BT2020";
		case V4L2_COLORSPACE_RAW: return "V4L2_COLORSPACE_RAW";
		case V4L2_COLORSPACE_DCI_P3: return "V4L2_COLORSPACE_DCI_P3";
	}

	return "UNKNOWN";
}

void v4l2PrintFormatDesc(const struct v4l2_fmtdesc* fmt) {
	LOGI("  fmt.index = %d", fmt->index);
	LOGI("  fmt.type = %s", v4l2BufTypeName(fmt->type));
	LOGI("  fmt.flags = %08x:", fmt->flags);
	v4l2PrintFormatFlags(fmt->flags);
	LOGI("  fmt.description = %s", fmt->description);
	LOGI("  fmt.pixelformat = %s (%08x)", v4l2PixFmtName(fmt->pixelformat), fmt->pixelformat);
	LOGI("  fmt.mbus_code = %d", fmt->mbus_code);
}

void v4l2PrintPixFormat(const struct v4l2_pix_format* pix) {
	LOGI("  pix.width = %d", pix->width);
	LOGI("  pix.height = %d", pix->height);
	LOGI("  pix.pixelformat = %s", v4l2PixFmtName(pix->pixelformat));
	LOGI("  pix.field = %x", pix->field); // TODO
	LOGI("  pix.bytesperline = %d", pix->bytesperline);
	LOGI("  pix.sizeimage = %d", pix->sizeimage);
	LOGI("  pix.colorspace = %s", v4l2ColorspaceName(pix->colorspace));
	LOGI("  pix.priv = %08x", pix->priv); // TODO
	LOGI("  pix.flags = %08x", pix->flags); // TODO
		// TODO __u32			ycbcr_enc;
		// TODO __u32			hsv_enc;
	LOGI("  pix.quantization = %08x", pix->quantization); // TODO /* enum v4l2_quantization */
	LOGI("  pix.xfer_func = %08x", pix->xfer_func); // TODO	/* enum v4l2_xfer_func */
}

void v4l2PrintPixFormatMPlane(const struct v4l2_pix_format_mplane* pix_mp) {
#define _(v) (v)
#define FIELDS(X) \
	X(width, "%u", _) \
	X(height, "%u", _) \
	X(field, "%08x", _) \
	X(colorspace, "%s", v4l2ColorspaceName) \
	X(num_planes, "%08x", _) \
	X(flags, "%08x", _) \
	X(quantization, "%08x", _) \
	X(xfer_func, "%08x", _) \

#define X(name, ffmt, func) \
	LOGI("  pix_mp." # name " = " ffmt, func(pix_mp->name));
	FIELDS(X)
#undef X
#undef _
#undef FIELDS

	for (int i = 0; i < pix_mp->num_planes; ++i) {
		LOGI("  pix_mp.plane_fmt[%d].sizeimage = %d", i, pix_mp->plane_fmt[i].sizeimage);
		LOGI("  pix_mp.plane_fmt[%d].bytesperline = %d", i, pix_mp->plane_fmt[i].bytesperline);
	}
}

void v4l2PrintFormat(const struct v4l2_format* fmt) {
	LOGI("  fmt.type = %s", v4l2BufTypeName(fmt->type));
	switch (fmt->type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
			v4l2PrintPixFormat(&fmt->fmt.pix);
			break;
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
			v4l2PrintPixFormatMPlane(&fmt->fmt.pix_mp);
			break;
		default:
			LOGE("Unimplemented buffer format type %s(%x)", v4l2BufTypeName(fmt->type), fmt->type);
	}
}

void v4l2PrintRequestBuffers(const struct v4l2_requestbuffers* req) {
	LOGI("req.count = %d", req->count);
	LOGI("req.capabilities = %08x:", req->capabilities);
	v4l2PrintBufferCapabilityBits(req->capabilities);
	LOGI("req.flags = %08x", req->flags);
}

const char *v4l2MemoryTypeName(enum v4l2_memory type) {
	switch (type) {
		case V4L2_MEMORY_MMAP: return "V4L2_MEMORY_MMAP";
		case V4L2_MEMORY_USERPTR: return "V4L2_MEMORY_USERPTR";
		case V4L2_MEMORY_OVERLAY: return "V4L2_MEMORY_OVERLAY";
		case V4L2_MEMORY_DMABUF: return "V4L2_MEMORY_DMABUF";
		default: return "UNKNOWN";
	}
}

void v4l2PrintBuffer(const struct v4l2_buffer *buf) {
#define _(v) (v)
#define FIELDS(X) \
	X(index, "%d", _) \
	X(type, "%s", v4l2BufTypeName) \
	X(bytesused, "%d", _) \
	X(flags, "%08x", _) \
	X(field, "%08x", _) \
	X(sequence, "%d", _) \
	X(memory, "%s", v4l2MemoryTypeName) \
	X(length, "%d", _) \

#define X(name, ffmt, func) \
	LOGI("  buf." #name " = " ffmt, func(buf->name));
	FIELDS(X)
#undef X
#undef FIELDS
#undef _

	switch (buf->memory) {
		case V4L2_MEMORY_MMAP:
			if (IS_TYPE_MPLANE(buf->type)) {
				if (buf->m.planes) {
					for (int i = 0; i < (int)buf->length; ++i) {
						LOGI("  buf.m.planes[%d].bytesused = %d", i, buf->m.planes[i].bytesused);
						LOGI("  buf.m.planes[%d].length = %d", i, buf->m.planes[i].length);
						LOGI("  buf.m.planes[%d].m.mem_offset = %d", i, buf->m.planes[i].m.mem_offset);
						LOGI("  buf.m.planes[%d].data_offset = %d", i, buf->m.planes[i].data_offset);
					}
				} else {
					LOGI("  buf.m.planes = NULL");
				}
			} else {
				LOGI("  buf.m.offset = %d", buf->m.offset);
			}
			break;
		case V4L2_MEMORY_USERPTR:
			if (IS_TYPE_MPLANE(buf->type)) {
				if (buf->m.planes) {
					for (int i = 0; i < (int)buf->length; ++i) {
						LOGI("  buf.m.planes[%d].bytesused = %d", i, buf->m.planes[i].bytesused);
						LOGI("  buf.m.planes[%d].length = %d", i, buf->m.planes[i].length);
						LOGI("  buf.m.planes[%d].m.userptr = %p", i, (void*) buf->m.planes[i].m.userptr);
						LOGI("  buf.m.planes[%d].data_offset = %d", i, buf->m.planes[i].data_offset);
					}
				} else {
					LOGI("  buf.m.planes = NULL");
				}
			} else {
				LOGI("  buf.m.userptr = %p", (void*)buf->m.userptr);
			}
			break;

		case V4L2_MEMORY_DMABUF:
			if (IS_TYPE_MPLANE(buf->type)) {
				if (buf->m.planes) {
					for (int i = 0; i < (int)buf->length; ++i) {
						LOGI("  buf.m.planes[%d].bytesused = %d", i, buf->m.planes[i].bytesused);
						LOGI("  buf.m.planes[%d].length = %d", i, buf->m.planes[i].length);
						LOGI("  buf.m.planes[%d].m.fd = %d", i, buf->m.planes[i].m.fd);
						LOGI("  buf.m.planes[%d].data_offset = %d", i, buf->m.planes[i].data_offset);
					}
				} else {
					LOGI("  buf.m.planes = NULL");
				}
			} else {
				LOGI("  buf.m.fd = %d", buf->m.fd);
			}
			break;
		case V4L2_MEMORY_OVERLAY:
			break;
	}

	// TODO
	//struct timeval		timestamp;
	//struct v4l2_timecode	timecode;
}

const char *v4l2FrmSizeTypeName(enum v4l2_frmsizetypes type) {
	switch (type) {
		case V4L2_FRMSIZE_TYPE_DISCRETE: return "V4L2_FRMSIZE_TYPE_DISCRETE";
		case V4L2_FRMSIZE_TYPE_CONTINUOUS: return "V4L2_FRMSIZE_TYPE_CONTINUOUS";
		case V4L2_FRMSIZE_TYPE_STEPWISE: return "V4L2_FRMSIZE_TYPE_STEPWISE";
	}
	return "UNKNOWN";
}

void v4l2PrintFrmSizeDiscrete(const struct v4l2_frmsize_discrete *fsd) {
	LOGI("    fsd.discrete = %dx%d", fsd->width, fsd->height);
}

void v4l2PrintFrmSizeStepwise(const struct v4l2_frmsize_stepwise *fss) {
	LOGI("    fss.stepwise = [%d..+%dx..%d] x [%d..+%dx..%d]",
		fss->min_width, fss->step_width, fss->max_width,
		fss->min_height, fss->step_height, fss->max_height);
}

void v4l2PrintFrmSizeEnum(const struct v4l2_frmsizeenum *fse) {
	//LOGI("    fse.type = %s", v4l2FrmSizeTypeName(fse->type));
	switch (fse->type) {
		case V4L2_FRMSIZE_TYPE_DISCRETE:
			LOGI("    fse.index = %d", fse->index);
			v4l2PrintFrmSizeDiscrete(&fse->discrete);
			break;
		case V4L2_FRMSIZE_TYPE_CONTINUOUS:
		case V4L2_FRMSIZE_TYPE_STEPWISE:
			v4l2PrintFrmSizeStepwise(&fse->stepwise);
			break;
	}
}

void v4l2PrintSubdevCapability(const struct v4l2_subdev_capability *cap) {
	LOGI("cap.version = %d.%d.%d", FROM_KERNEL_VERSION(cap->version));
	LOGI("cap.capabilities = %08x", cap->capabilities); // TODO
}

#define PRINT_FIELD(prefix, s, name, ffmt, func) \
	LOGI(prefix #name " = " ffmt, func(s->name));

const char *v4l2MbusFmtName(uint32_t format) {
	switch (format) {
		case MEDIA_BUS_FMT_FIXED: return "MEDIA_BUS_FMT_FIXED";
		case MEDIA_BUS_FMT_RGB444_1X12: return "MEDIA_BUS_FMT_RGB444_1X12";
		case MEDIA_BUS_FMT_RGB444_2X8_PADHI_BE: return "MEDIA_BUS_FMT_RGB444_2X8_PADHI_BE";
		case MEDIA_BUS_FMT_RGB444_2X8_PADHI_LE: return "MEDIA_BUS_FMT_RGB444_2X8_PADHI_LE";
		case MEDIA_BUS_FMT_RGB555_2X8_PADHI_BE: return "MEDIA_BUS_FMT_RGB555_2X8_PADHI_BE";
		case MEDIA_BUS_FMT_RGB555_2X8_PADHI_LE: return "MEDIA_BUS_FMT_RGB555_2X8_PADHI_LE";
		case MEDIA_BUS_FMT_RGB565_1X16: return "MEDIA_BUS_FMT_RGB565_1X16";
		case MEDIA_BUS_FMT_BGR565_2X8_BE: return "MEDIA_BUS_FMT_BGR565_2X8_BE";
		case MEDIA_BUS_FMT_BGR565_2X8_LE: return "MEDIA_BUS_FMT_BGR565_2X8_LE";
		case MEDIA_BUS_FMT_RGB565_2X8_BE: return "MEDIA_BUS_FMT_RGB565_2X8_BE";
		case MEDIA_BUS_FMT_RGB565_2X8_LE: return "MEDIA_BUS_FMT_RGB565_2X8_LE";
		case MEDIA_BUS_FMT_RGB666_1X18: return "MEDIA_BUS_FMT_RGB666_1X18";
		case MEDIA_BUS_FMT_RBG888_1X24: return "MEDIA_BUS_FMT_RBG888_1X24";
		case MEDIA_BUS_FMT_RGB666_1X24_CPADHI: return "MEDIA_BUS_FMT_RGB666_1X24_CPADHI";
		case MEDIA_BUS_FMT_RGB666_1X7X3_SPWG: return "MEDIA_BUS_FMT_RGB666_1X7X3_SPWG";
		case MEDIA_BUS_FMT_BGR888_1X24: return "MEDIA_BUS_FMT_BGR888_1X24";
		case MEDIA_BUS_FMT_BGR888_3X8: return "MEDIA_BUS_FMT_BGR888_3X8";
		case MEDIA_BUS_FMT_GBR888_1X24: return "MEDIA_BUS_FMT_GBR888_1X24";
		case MEDIA_BUS_FMT_RGB888_1X24: return "MEDIA_BUS_FMT_RGB888_1X24";
		case MEDIA_BUS_FMT_RGB888_2X12_BE: return "MEDIA_BUS_FMT_RGB888_2X12_BE";
		case MEDIA_BUS_FMT_RGB888_2X12_LE: return "MEDIA_BUS_FMT_RGB888_2X12_LE";
		case MEDIA_BUS_FMT_RGB888_3X8: return "MEDIA_BUS_FMT_RGB888_3X8";
		case MEDIA_BUS_FMT_RGB888_3X8_DELTA: return "MEDIA_BUS_FMT_RGB888_3X8_DELTA";
		case MEDIA_BUS_FMT_RGB888_1X7X4_SPWG: return "MEDIA_BUS_FMT_RGB888_1X7X4_SPWG";
		case MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA: return "MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA";
		case MEDIA_BUS_FMT_RGB666_1X30_CPADLO: return "MEDIA_BUS_FMT_RGB666_1X30_CPADLO";
		case MEDIA_BUS_FMT_RGB888_1X30_CPADLO: return "MEDIA_BUS_FMT_RGB888_1X30_CPADLO";
		case MEDIA_BUS_FMT_ARGB8888_1X32: return "MEDIA_BUS_FMT_ARGB8888_1X32";
		case MEDIA_BUS_FMT_RGB888_1X32_PADHI: return "MEDIA_BUS_FMT_RGB888_1X32_PADHI";
		case MEDIA_BUS_FMT_RGB101010_1X30: return "MEDIA_BUS_FMT_RGB101010_1X30";
		case MEDIA_BUS_FMT_RGB666_1X36_CPADLO: return "MEDIA_BUS_FMT_RGB666_1X36_CPADLO";
		case MEDIA_BUS_FMT_RGB888_1X36_CPADLO: return "MEDIA_BUS_FMT_RGB888_1X36_CPADLO";
		case MEDIA_BUS_FMT_RGB121212_1X36: return "MEDIA_BUS_FMT_RGB121212_1X36";
		case MEDIA_BUS_FMT_RGB161616_1X48: return "MEDIA_BUS_FMT_RGB161616_1X48";
		case MEDIA_BUS_FMT_Y8_1X8: return "MEDIA_BUS_FMT_Y8_1X8";
		case MEDIA_BUS_FMT_UV8_1X8: return "MEDIA_BUS_FMT_UV8_1X8";
		case MEDIA_BUS_FMT_UYVY8_1_5X8: return "MEDIA_BUS_FMT_UYVY8_1_5X8";
		case MEDIA_BUS_FMT_VYUY8_1_5X8: return "MEDIA_BUS_FMT_VYUY8_1_5X8";
		case MEDIA_BUS_FMT_YUYV8_1_5X8: return "MEDIA_BUS_FMT_YUYV8_1_5X8";
		case MEDIA_BUS_FMT_YVYU8_1_5X8: return "MEDIA_BUS_FMT_YVYU8_1_5X8";
		case MEDIA_BUS_FMT_UYVY8_2X8: return "MEDIA_BUS_FMT_UYVY8_2X8";
		case MEDIA_BUS_FMT_VYUY8_2X8: return "MEDIA_BUS_FMT_VYUY8_2X8";
		case MEDIA_BUS_FMT_YUYV8_2X8: return "MEDIA_BUS_FMT_YUYV8_2X8";
		case MEDIA_BUS_FMT_YVYU8_2X8: return "MEDIA_BUS_FMT_YVYU8_2X8";
		case MEDIA_BUS_FMT_Y10_1X10: return "MEDIA_BUS_FMT_Y10_1X10";
		case MEDIA_BUS_FMT_Y10_2X8_PADHI_LE: return "MEDIA_BUS_FMT_Y10_2X8_PADHI_LE";
		case MEDIA_BUS_FMT_UYVY10_2X10: return "MEDIA_BUS_FMT_UYVY10_2X10";
		case MEDIA_BUS_FMT_VYUY10_2X10: return "MEDIA_BUS_FMT_VYUY10_2X10";
		case MEDIA_BUS_FMT_YUYV10_2X10: return "MEDIA_BUS_FMT_YUYV10_2X10";
		case MEDIA_BUS_FMT_YVYU10_2X10: return "MEDIA_BUS_FMT_YVYU10_2X10";
		case MEDIA_BUS_FMT_Y12_1X12: return "MEDIA_BUS_FMT_Y12_1X12";
		case MEDIA_BUS_FMT_UYVY12_2X12: return "MEDIA_BUS_FMT_UYVY12_2X12";
		case MEDIA_BUS_FMT_VYUY12_2X12: return "MEDIA_BUS_FMT_VYUY12_2X12";
		case MEDIA_BUS_FMT_YUYV12_2X12: return "MEDIA_BUS_FMT_YUYV12_2X12";
		case MEDIA_BUS_FMT_YVYU12_2X12: return "MEDIA_BUS_FMT_YVYU12_2X12";
		case MEDIA_BUS_FMT_Y14_1X14: return "MEDIA_BUS_FMT_Y14_1X14";
		case MEDIA_BUS_FMT_UYVY8_1X16: return "MEDIA_BUS_FMT_UYVY8_1X16";
		case MEDIA_BUS_FMT_VYUY8_1X16: return "MEDIA_BUS_FMT_VYUY8_1X16";
		case MEDIA_BUS_FMT_YUYV8_1X16: return "MEDIA_BUS_FMT_YUYV8_1X16";
		case MEDIA_BUS_FMT_YVYU8_1X16: return "MEDIA_BUS_FMT_YVYU8_1X16";
		case MEDIA_BUS_FMT_YDYUYDYV8_1X16: return "MEDIA_BUS_FMT_YDYUYDYV8_1X16";
		case MEDIA_BUS_FMT_UYVY10_1X20: return "MEDIA_BUS_FMT_UYVY10_1X20";
		case MEDIA_BUS_FMT_VYUY10_1X20: return "MEDIA_BUS_FMT_VYUY10_1X20";
		case MEDIA_BUS_FMT_YUYV10_1X20: return "MEDIA_BUS_FMT_YUYV10_1X20";
		case MEDIA_BUS_FMT_YVYU10_1X20: return "MEDIA_BUS_FMT_YVYU10_1X20";
		case MEDIA_BUS_FMT_VUY8_1X24: return "MEDIA_BUS_FMT_VUY8_1X24";
		case MEDIA_BUS_FMT_YUV8_1X24: return "MEDIA_BUS_FMT_YUV8_1X24";
		case MEDIA_BUS_FMT_UYYVYY8_0_5X24: return "MEDIA_BUS_FMT_UYYVYY8_0_5X24";
		case MEDIA_BUS_FMT_UYVY12_1X24: return "MEDIA_BUS_FMT_UYVY12_1X24";
		case MEDIA_BUS_FMT_VYUY12_1X24: return "MEDIA_BUS_FMT_VYUY12_1X24";
		case MEDIA_BUS_FMT_YUYV12_1X24: return "MEDIA_BUS_FMT_YUYV12_1X24";
		case MEDIA_BUS_FMT_YVYU12_1X24: return "MEDIA_BUS_FMT_YVYU12_1X24";
		case MEDIA_BUS_FMT_YUV10_1X30: return "MEDIA_BUS_FMT_YUV10_1X30";
		case MEDIA_BUS_FMT_UYYVYY10_0_5X30: return "MEDIA_BUS_FMT_UYYVYY10_0_5X30";
		case MEDIA_BUS_FMT_AYUV8_1X32: return "MEDIA_BUS_FMT_AYUV8_1X32";
		case MEDIA_BUS_FMT_UYYVYY12_0_5X36: return "MEDIA_BUS_FMT_UYYVYY12_0_5X36";
		case MEDIA_BUS_FMT_YUV12_1X36: return "MEDIA_BUS_FMT_YUV12_1X36";
		case MEDIA_BUS_FMT_YUV16_1X48: return "MEDIA_BUS_FMT_YUV16_1X48";
		case MEDIA_BUS_FMT_UYYVYY16_0_5X48: return "MEDIA_BUS_FMT_UYYVYY16_0_5X48";
		case MEDIA_BUS_FMT_SBGGR8_1X8: return "MEDIA_BUS_FMT_SBGGR8_1X8";
		case MEDIA_BUS_FMT_SGBRG8_1X8: return "MEDIA_BUS_FMT_SGBRG8_1X8";
		case MEDIA_BUS_FMT_SGRBG8_1X8: return "MEDIA_BUS_FMT_SGRBG8_1X8";
		case MEDIA_BUS_FMT_SRGGB8_1X8: return "MEDIA_BUS_FMT_SRGGB8_1X8";
		case MEDIA_BUS_FMT_SBGGR10_ALAW8_1X8: return "MEDIA_BUS_FMT_SBGGR10_ALAW8_1X8";
		case MEDIA_BUS_FMT_SGBRG10_ALAW8_1X8: return "MEDIA_BUS_FMT_SGBRG10_ALAW8_1X8";
		case MEDIA_BUS_FMT_SGRBG10_ALAW8_1X8: return "MEDIA_BUS_FMT_SGRBG10_ALAW8_1X8";
		case MEDIA_BUS_FMT_SRGGB10_ALAW8_1X8: return "MEDIA_BUS_FMT_SRGGB10_ALAW8_1X8";
		case MEDIA_BUS_FMT_SBGGR10_DPCM8_1X8: return "MEDIA_BUS_FMT_SBGGR10_DPCM8_1X8";
		case MEDIA_BUS_FMT_SGBRG10_DPCM8_1X8: return "MEDIA_BUS_FMT_SGBRG10_DPCM8_1X8";
		case MEDIA_BUS_FMT_SGRBG10_DPCM8_1X8: return "MEDIA_BUS_FMT_SGRBG10_DPCM8_1X8";
		case MEDIA_BUS_FMT_SRGGB10_DPCM8_1X8: return "MEDIA_BUS_FMT_SRGGB10_DPCM8_1X8";
		case MEDIA_BUS_FMT_SBGGR10_2X8_PADHI_BE: return "MEDIA_BUS_FMT_SBGGR10_2X8_PADHI_BE";
		case MEDIA_BUS_FMT_SBGGR10_2X8_PADHI_LE: return "MEDIA_BUS_FMT_SBGGR10_2X8_PADHI_LE";
		case MEDIA_BUS_FMT_SBGGR10_2X8_PADLO_BE: return "MEDIA_BUS_FMT_SBGGR10_2X8_PADLO_BE";
		case MEDIA_BUS_FMT_SBGGR10_2X8_PADLO_LE: return "MEDIA_BUS_FMT_SBGGR10_2X8_PADLO_LE";
		case MEDIA_BUS_FMT_SBGGR10_1X10: return "MEDIA_BUS_FMT_SBGGR10_1X10";
		case MEDIA_BUS_FMT_SGBRG10_1X10: return "MEDIA_BUS_FMT_SGBRG10_1X10";
		case MEDIA_BUS_FMT_SGRBG10_1X10: return "MEDIA_BUS_FMT_SGRBG10_1X10";
		case MEDIA_BUS_FMT_SRGGB10_1X10: return "MEDIA_BUS_FMT_SRGGB10_1X10";
		case MEDIA_BUS_FMT_SBGGR12_1X12: return "MEDIA_BUS_FMT_SBGGR12_1X12";
		case MEDIA_BUS_FMT_SGBRG12_1X12: return "MEDIA_BUS_FMT_SGBRG12_1X12";
		case MEDIA_BUS_FMT_SGRBG12_1X12: return "MEDIA_BUS_FMT_SGRBG12_1X12";
		case MEDIA_BUS_FMT_SRGGB12_1X12: return "MEDIA_BUS_FMT_SRGGB12_1X12";
		case MEDIA_BUS_FMT_SBGGR14_1X14: return "MEDIA_BUS_FMT_SBGGR14_1X14";
		case MEDIA_BUS_FMT_SGBRG14_1X14: return "MEDIA_BUS_FMT_SGBRG14_1X14";
		case MEDIA_BUS_FMT_SGRBG14_1X14: return "MEDIA_BUS_FMT_SGRBG14_1X14";
		case MEDIA_BUS_FMT_SRGGB14_1X14: return "MEDIA_BUS_FMT_SRGGB14_1X14";
		case MEDIA_BUS_FMT_SBGGR16_1X16: return "MEDIA_BUS_FMT_SBGGR16_1X16";
		case MEDIA_BUS_FMT_SGBRG16_1X16: return "MEDIA_BUS_FMT_SGBRG16_1X16";
		case MEDIA_BUS_FMT_SGRBG16_1X16: return "MEDIA_BUS_FMT_SGRBG16_1X16";
		case MEDIA_BUS_FMT_SRGGB16_1X16: return "MEDIA_BUS_FMT_SRGGB16_1X16";
		case MEDIA_BUS_FMT_JPEG_1X8: return "MEDIA_BUS_FMT_JPEG_1X8";
		case MEDIA_BUS_FMT_S5C_UYVY_JPEG_1X8: return "MEDIA_BUS_FMT_S5C_UYVY_JPEG_1X8";
		case MEDIA_BUS_FMT_AHSV8888_1X32: return "MEDIA_BUS_FMT_AHSV8888_1X32";
		case MEDIA_BUS_FMT_METADATA_FIXED: return "MEDIA_BUS_FMT_METADATA_FIXED";
	}
	return "UNKNOWN";
}

void v4l2PrintMbusFramefmt(const struct v4l2_mbus_framefmt *mf) {
#define FIELDS(prefix, s,X) \
	X(prefix,s, width, "%d", _) \
	X(prefix,s, height, "%d", _) \
	X(prefix,s, code, "%s", v4l2MbusFmtName) \
	X(prefix,s, field, "%d", _) \
	X(prefix,s, colorspace, "%s", v4l2ColorspaceName) \
	X(prefix,s, ycbcr_enc, "%d", _) \
	X(prefix,s, hsv_enc, "%d", _) \
	X(prefix,s, quantization, "%d", _) \
	X(prefix,s, xfer_func, "%d", _) \
	X(prefix,s, flags, "%08x", _) \

#define _(p) (p)
	FIELDS(" fmt.format.", mf, PRINT_FIELD)
#undef _
}

void v4l2PrintSubdevFormat(const struct v4l2_subdev_format *format) {
	LOGI(" fmt.pad = %d", format->pad);
	LOGI(" fmt.which = %d", format->which); // TODO
	v4l2PrintMbusFramefmt(&format->format);
}

const char *v4l2SelTgtName(uint32_t target) {
	switch (target) {
		case V4L2_SEL_TGT_CROP: return "V4L2_SEL_TGT_CROP";
		case V4L2_SEL_TGT_CROP_DEFAULT: return "V4L2_SEL_TGT_CROP_DEFAULT";
		case V4L2_SEL_TGT_CROP_BOUNDS: return "V4L2_SEL_TGT_CROP_BOUNDS";
		case V4L2_SEL_TGT_NATIVE_SIZE: return "V4L2_SEL_TGT_NATIVE_SIZE";
		case V4L2_SEL_TGT_COMPOSE: return "V4L2_SEL_TGT_COMPOSE";
		case V4L2_SEL_TGT_COMPOSE_DEFAULT: return "V4L2_SEL_TGT_COMPOSE_DEFAULT";
		case V4L2_SEL_TGT_COMPOSE_BOUNDS: return "V4L2_SEL_TGT_COMPOSE_BOUNDS";
		case V4L2_SEL_TGT_COMPOSE_PADDED: return "V4L2_SEL_TGT_COMPOSE_PADDED";
		default: return "UNKNOWN";
	}
}

#define MCAM_SEL_FLAG_LIST(X) \
	X(V4L2_SEL_FLAG_GE) \
	X(V4L2_SEL_FLAG_LE) \
	X(V4L2_SEL_FLAG_KEEP_CONFIG) \

void v4l2PrintSelFlags(uint32_t bits) {
#define X(bit) if (bits & bit) LOGI("  %s", #bit);
	MCAM_SEL_FLAG_LIST(X)
#undef X
}

void v4l2PrintSubdevSelection(const struct v4l2_subdev_selection *sel) {
	LOGI("sel.which = %d", sel->which); // TODO
	LOGI("sel.pad = %d", sel->pad);
	LOGI("sel.target = %s(%08x)", v4l2SelTgtName(sel->target), sel->target);
	LOGI("sel.flags = %08x", sel->flags);
	v4l2PrintSelFlags(sel->target);
	LOGI("sel.r = (%d, %d) + (%dx%d)", sel->r.top, sel->r.left, sel->r.width, sel->r.height);
}

void v4l2PrintInterval(const char *prefix, struct v4l2_fract fr) {
	LOGI("%s%d/%d (%dms, %dfps)", prefix,
		fr.numerator, fr.denominator,
		1000 * fr.denominator / fr.numerator,
		fr.denominator
	);
}

void v4l2PrintFrameInterval(const struct v4l2_subdev_frame_interval *fi) {
	LOGI("fi.pad = %d", fi->pad);
	v4l2PrintInterval("fi.interval = ", fi->interval);
}

#define MCAM_SUBDEV_MBUS_CODE_LIST(X) \
	X(V4L2_SUBDEV_MBUS_CODE_CSC_COLORSPACE) \
	X(V4L2_SUBDEV_MBUS_CODE_CSC_XFER_FUNC) \
	X(V4L2_SUBDEV_MBUS_CODE_CSC_YCBCR_ENC) \
	X(V4L2_SUBDEV_MBUS_CODE_CSC_YCBCR_ENC) \
	X(V4L2_SUBDEV_MBUS_CODE_CSC_QUANTIZATION) \

void v4l2PrintMbusCodeFlags(uint32_t bits) {
#define X(bit) if (bits & bit) LOGI("  %s", #bit);
	MCAM_SEL_FLAG_LIST(X)
#undef X
}

void v4l2PrintSubdevMbusCode(const struct v4l2_subdev_mbus_code_enum *mbc) {
	LOGI(" mbc.pad = %d", mbc->pad);
	LOGI(" mbc.index = %d", mbc->index);
	LOGI(" mbc.code = %s (%08x)", v4l2MbusFmtName(mbc->code), mbc->code);
	LOGI(" mbc.which = %d", mbc->which);
	LOGI(" mbc.flags = %08x", mbc->flags);
	v4l2PrintMbusCodeFlags(mbc->flags);
}

void v4l2PrintSubdevFrameSize(const struct v4l2_subdev_frame_size_enum *fsz) {
	LOGI("  fsz.pad = %d", fsz->pad);
	LOGI("  fsz.index = %d", fsz->index);
	LOGI("  fsz.code = %s (%08x)", v4l2MbusFmtName(fsz->code), fsz->code);
	LOGI("  fsz.which = %d", fsz->which);
	LOGI("  fsz.width = %d..%d", fsz->min_width, fsz->max_width);
	LOGI("  fsz.height = %d..%d", fsz->min_height, fsz->max_height);
}

void v4l2PrintSubdevFrameInterval(const struct v4l2_subdev_frame_interval_enum *fiv) {
	LOGI("   fiv.pad = %d", fiv->pad);
	LOGI("   fiv.index = %d", fiv->index);
	LOGI("   fiv.code = %s (%08x)", v4l2MbusFmtName(fiv->code), fiv->code);
	LOGI("   fiv.which = %d", fiv->which);
	LOGI("   fiv.width = %d", fiv->width);
	LOGI("   fiv.height = %d", fiv->height);
	v4l2PrintInterval("  fiv.interval = ", fiv->interval);
}

void v4l2PrintSelection(const struct v4l2_selection* sel) {
	LOGI("  sel.type = %s", v4l2BufTypeName(sel->type));
	LOGI("  sel.target = %s", v4l2SelTgtName(sel->target));
	LOGI("  sel.flags = %x", sel->flags);
	LOGI("  sel.r = {%d, %d, %dx%d}",
			sel->r.left, sel->r.top, sel->r.width, sel->r.height);
}

const char* v4l2CtrlIdName(uint32_t ctrl_id) {
	switch (ctrl_id) {
#ifdef V4L2_CID_BRIGHTNESS
		case V4L2_CID_BRIGHTNESS: return "V4L2_CID_BRIGHTNESS";
#endif
#ifdef V4L2_CID_CONTRAST
		case V4L2_CID_CONTRAST: return "V4L2_CID_CONTRAST";
#endif
#ifdef V4L2_CID_SATURATION
		case V4L2_CID_SATURATION: return "V4L2_CID_SATURATION";
#endif
#ifdef V4L2_CID_HUE
		case V4L2_CID_HUE: return "V4L2_CID_HUE";
#endif
#ifdef V4L2_CID_AUDIO_VOLUME
		case V4L2_CID_AUDIO_VOLUME: return "V4L2_CID_AUDIO_VOLUME";
#endif
#ifdef V4L2_CID_AUDIO_BALANCE
		case V4L2_CID_AUDIO_BALANCE: return "V4L2_CID_AUDIO_BALANCE";
#endif
#ifdef V4L2_CID_AUDIO_BASS
		case V4L2_CID_AUDIO_BASS: return "V4L2_CID_AUDIO_BASS";
#endif
#ifdef V4L2_CID_AUDIO_TREBLE
		case V4L2_CID_AUDIO_TREBLE: return "V4L2_CID_AUDIO_TREBLE";
#endif
#ifdef V4L2_CID_AUDIO_MUTE
		case V4L2_CID_AUDIO_MUTE: return "V4L2_CID_AUDIO_MUTE";
#endif
#ifdef V4L2_CID_AUDIO_LOUDNESS
		case V4L2_CID_AUDIO_LOUDNESS: return "V4L2_CID_AUDIO_LOUDNESS";
#endif
#ifdef V4L2_CID_BLACK_LEVEL
		case V4L2_CID_BLACK_LEVEL: return "V4L2_CID_BLACK_LEVEL";
#endif
#ifdef V4L2_CID_AUTO_WHITE_BALANCE
		case V4L2_CID_AUTO_WHITE_BALANCE: return "V4L2_CID_AUTO_WHITE_BALANCE";
#endif
#ifdef V4L2_CID_DO_WHITE_BALANCE
		case V4L2_CID_DO_WHITE_BALANCE: return "V4L2_CID_DO_WHITE_BALANCE";
#endif
#ifdef V4L2_CID_RED_BALANCE
		case V4L2_CID_RED_BALANCE: return "V4L2_CID_RED_BALANCE";
#endif
#ifdef V4L2_CID_BLUE_BALANCE
		case V4L2_CID_BLUE_BALANCE: return "V4L2_CID_BLUE_BALANCE";
#endif
#ifdef V4L2_CID_GAMMA
		case V4L2_CID_GAMMA: return "V4L2_CID_GAMMA";
#endif
#ifdef V4L2_CID_EXPOSURE
		case V4L2_CID_EXPOSURE: return "V4L2_CID_EXPOSURE";
#endif
#ifdef V4L2_CID_AUTOGAIN
		case V4L2_CID_AUTOGAIN: return "V4L2_CID_AUTOGAIN";
#endif
#ifdef V4L2_CID_GAIN
		case V4L2_CID_GAIN: return "V4L2_CID_GAIN";
#endif
#ifdef V4L2_CID_HFLIP
		case V4L2_CID_HFLIP: return "V4L2_CID_HFLIP";
#endif
#ifdef V4L2_CID_VFLIP
		case V4L2_CID_VFLIP: return "V4L2_CID_VFLIP";
#endif
#ifdef V4L2_CID_POWER_LINE_FREQUENCY
		case V4L2_CID_POWER_LINE_FREQUENCY: return "V4L2_CID_POWER_LINE_FREQUENCY";
#endif
#ifdef V4L2_CID_HUE_AUTO
		case V4L2_CID_HUE_AUTO: return "V4L2_CID_HUE_AUTO";
#endif
#ifdef V4L2_CID_WHITE_BALANCE_TEMPERATURE
		case V4L2_CID_WHITE_BALANCE_TEMPERATURE: return "V4L2_CID_WHITE_BALANCE_TEMPERATURE";
#endif
#ifdef V4L2_CID_SHARPNESS
		case V4L2_CID_SHARPNESS: return "V4L2_CID_SHARPNESS";
#endif
#ifdef V4L2_CID_BACKLIGHT_COMPENSATION
		case V4L2_CID_BACKLIGHT_COMPENSATION: return "V4L2_CID_BACKLIGHT_COMPENSATION";
#endif
#ifdef V4L2_CID_CHROMA_AGC
		case V4L2_CID_CHROMA_AGC: return "V4L2_CID_CHROMA_AGC";
#endif
#ifdef V4L2_CID_COLOR_KILLER
		case V4L2_CID_COLOR_KILLER: return "V4L2_CID_COLOR_KILLER";
#endif
#ifdef V4L2_CID_COLORFX
		case V4L2_CID_COLORFX: return "V4L2_CID_COLORFX";
#endif
#ifdef V4L2_CID_AUTOBRIGHTNESS
		case V4L2_CID_AUTOBRIGHTNESS: return "V4L2_CID_AUTOBRIGHTNESS";
#endif
#ifdef V4L2_CID_BAND_STOP_FILTER
		case V4L2_CID_BAND_STOP_FILTER: return "V4L2_CID_BAND_STOP_FILTER";
#endif
#ifdef V4L2_CID_ROTATE
		case V4L2_CID_ROTATE: return "V4L2_CID_ROTATE";
#endif
#ifdef V4L2_CID_BG_COLOR
		case V4L2_CID_BG_COLOR: return "V4L2_CID_BG_COLOR";
#endif
#ifdef V4L2_CID_CHROMA_GAIN
		case V4L2_CID_CHROMA_GAIN: return "V4L2_CID_CHROMA_GAIN";
#endif
#ifdef V4L2_CID_ILLUMINATORS_1
		case V4L2_CID_ILLUMINATORS_1: return "V4L2_CID_ILLUMINATORS_1";
#endif
#ifdef V4L2_CID_ILLUMINATORS_2
		case V4L2_CID_ILLUMINATORS_2: return "V4L2_CID_ILLUMINATORS_2";
#endif
#ifdef V4L2_CID_MIN_BUFFERS_FOR_CAPTURE
		case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE: return "V4L2_CID_MIN_BUFFERS_FOR_CAPTURE";
#endif
#ifdef V4L2_CID_MIN_BUFFERS_FOR_OUTPUT
		case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT: return "V4L2_CID_MIN_BUFFERS_FOR_OUTPUT";
#endif
#ifdef V4L2_CID_ALPHA_COMPONENT
		case V4L2_CID_ALPHA_COMPONENT: return "V4L2_CID_ALPHA_COMPONENT";
#endif
#ifdef V4L2_CID_COLORFX_CBCR
		case V4L2_CID_COLORFX_CBCR: return "V4L2_CID_COLORFX_CBCR";
#endif
#ifdef V4L2_CID_COLORFX_RGB
		case V4L2_CID_COLORFX_RGB: return "V4L2_CID_COLORFX_RGB";
#endif
#ifdef V4L2_CID_LASTP1
		case V4L2_CID_LASTP1: return "V4L2_CID_LASTP1";
#endif
#ifdef V4L2_CID_MPEG_STREAM_PID_PMT
		case V4L2_CID_MPEG_STREAM_PID_PMT: return "V4L2_CID_MPEG_STREAM_PID_PMT";
#endif
#ifdef V4L2_CID_MPEG_STREAM_PID_AUDIO
		case V4L2_CID_MPEG_STREAM_PID_AUDIO: return "V4L2_CID_MPEG_STREAM_PID_AUDIO";
#endif
#ifdef V4L2_CID_MPEG_STREAM_PID_VIDEO
		case V4L2_CID_MPEG_STREAM_PID_VIDEO: return "V4L2_CID_MPEG_STREAM_PID_VIDEO";
#endif
#ifdef V4L2_CID_MPEG_STREAM_PID_PCR
		case V4L2_CID_MPEG_STREAM_PID_PCR: return "V4L2_CID_MPEG_STREAM_PID_PCR";
#endif
#ifdef V4L2_CID_MPEG_STREAM_PES_ID_AUDIO
		case V4L2_CID_MPEG_STREAM_PES_ID_AUDIO: return "V4L2_CID_MPEG_STREAM_PES_ID_AUDIO";
#endif
#ifdef V4L2_CID_MPEG_STREAM_PES_ID_VIDEO
		case V4L2_CID_MPEG_STREAM_PES_ID_VIDEO: return "V4L2_CID_MPEG_STREAM_PES_ID_VIDEO";
#endif
#ifdef V4L2_CID_MPEG_STREAM_VBI_FMT
		case V4L2_CID_MPEG_STREAM_VBI_FMT: return "V4L2_CID_MPEG_STREAM_VBI_FMT";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ
		case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ: return "V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_ENCODING
		case V4L2_CID_MPEG_AUDIO_ENCODING: return "V4L2_CID_MPEG_AUDIO_ENCODING";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_L1_BITRATE
		case V4L2_CID_MPEG_AUDIO_L1_BITRATE: return "V4L2_CID_MPEG_AUDIO_L1_BITRATE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_L2_BITRATE
		case V4L2_CID_MPEG_AUDIO_L2_BITRATE: return "V4L2_CID_MPEG_AUDIO_L2_BITRATE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_L3_BITRATE
		case V4L2_CID_MPEG_AUDIO_L3_BITRATE: return "V4L2_CID_MPEG_AUDIO_L3_BITRATE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_MODE
		case V4L2_CID_MPEG_AUDIO_MODE: return "V4L2_CID_MPEG_AUDIO_MODE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_MODE_EXTENSION
		case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION: return "V4L2_CID_MPEG_AUDIO_MODE_EXTENSION";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_EMPHASIS
		case V4L2_CID_MPEG_AUDIO_EMPHASIS: return "V4L2_CID_MPEG_AUDIO_EMPHASIS";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_CRC
		case V4L2_CID_MPEG_AUDIO_CRC: return "V4L2_CID_MPEG_AUDIO_CRC";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_MUTE
		case V4L2_CID_MPEG_AUDIO_MUTE: return "V4L2_CID_MPEG_AUDIO_MUTE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_AAC_BITRATE
		case V4L2_CID_MPEG_AUDIO_AAC_BITRATE: return "V4L2_CID_MPEG_AUDIO_AAC_BITRATE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_AC3_BITRATE
		case V4L2_CID_MPEG_AUDIO_AC3_BITRATE: return "V4L2_CID_MPEG_AUDIO_AC3_BITRATE";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_DEC_PLAYBACK
		case V4L2_CID_MPEG_AUDIO_DEC_PLAYBACK: return "V4L2_CID_MPEG_AUDIO_DEC_PLAYBACK";
#endif
#ifdef V4L2_CID_MPEG_AUDIO_DEC_MULTILINGUAL_PLAYBACK
		case V4L2_CID_MPEG_AUDIO_DEC_MULTILINGUAL_PLAYBACK: return "V4L2_CID_MPEG_AUDIO_DEC_MULTILINGUAL_PLAYBACK";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_ENCODING
		case V4L2_CID_MPEG_VIDEO_ENCODING: return "V4L2_CID_MPEG_VIDEO_ENCODING";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_ASPECT
		case V4L2_CID_MPEG_VIDEO_ASPECT: return "V4L2_CID_MPEG_VIDEO_ASPECT";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_B_FRAMES
		case V4L2_CID_MPEG_VIDEO_B_FRAMES: return "V4L2_CID_MPEG_VIDEO_B_FRAMES";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_GOP_SIZE
		case V4L2_CID_MPEG_VIDEO_GOP_SIZE: return "V4L2_CID_MPEG_VIDEO_GOP_SIZE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_GOP_CLOSURE
		case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE: return "V4L2_CID_MPEG_VIDEO_GOP_CLOSURE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_PULLDOWN
		case V4L2_CID_MPEG_VIDEO_PULLDOWN: return "V4L2_CID_MPEG_VIDEO_PULLDOWN";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_BITRATE_MODE
		case V4L2_CID_MPEG_VIDEO_BITRATE_MODE: return "V4L2_CID_MPEG_VIDEO_BITRATE_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_BITRATE
		case V4L2_CID_MPEG_VIDEO_BITRATE: return "V4L2_CID_MPEG_VIDEO_BITRATE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_BITRATE_PEAK
		case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK: return "V4L2_CID_MPEG_VIDEO_BITRATE_PEAK";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_TEMPORAL_DECIMATION
		case V4L2_CID_MPEG_VIDEO_TEMPORAL_DECIMATION: return "V4L2_CID_MPEG_VIDEO_TEMPORAL_DECIMATION";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MUTE
		case V4L2_CID_MPEG_VIDEO_MUTE: return "V4L2_CID_MPEG_VIDEO_MUTE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MUTE_YUV
		case V4L2_CID_MPEG_VIDEO_MUTE_YUV: return "V4L2_CID_MPEG_VIDEO_MUTE_YUV";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DECODER_SLICE_INTERFACE
		case V4L2_CID_MPEG_VIDEO_DECODER_SLICE_INTERFACE: return "V4L2_CID_MPEG_VIDEO_DECODER_SLICE_INTERFACE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DECODER_MPEG4_DEBLOCK_FILTER
		case V4L2_CID_MPEG_VIDEO_DECODER_MPEG4_DEBLOCK_FILTER: return "V4L2_CID_MPEG_VIDEO_DECODER_MPEG4_DEBLOCK_FILTER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB
		case V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB: return "V4L2_CID_MPEG_VIDEO_CYCLIC_INTRA_REFRESH_MB";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE
		case V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE: return "V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEADER_MODE
		case V4L2_CID_MPEG_VIDEO_HEADER_MODE: return "V4L2_CID_MPEG_VIDEO_HEADER_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MAX_REF_PIC
		case V4L2_CID_MPEG_VIDEO_MAX_REF_PIC: return "V4L2_CID_MPEG_VIDEO_MAX_REF_PIC";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE
		case V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE: return "V4L2_CID_MPEG_VIDEO_MB_RC_ENABLE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES
		case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES: return "V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_BYTES";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB
		case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB: return "V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MAX_MB";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE
		case V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE: return "V4L2_CID_MPEG_VIDEO_MULTI_SLICE_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VBV_SIZE
		case V4L2_CID_MPEG_VIDEO_VBV_SIZE: return "V4L2_CID_MPEG_VIDEO_VBV_SIZE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DEC_PTS
		case V4L2_CID_MPEG_VIDEO_DEC_PTS: return "V4L2_CID_MPEG_VIDEO_DEC_PTS";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DEC_FRAME
		case V4L2_CID_MPEG_VIDEO_DEC_FRAME: return "V4L2_CID_MPEG_VIDEO_DEC_FRAME";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VBV_DELAY
		case V4L2_CID_MPEG_VIDEO_VBV_DELAY: return "V4L2_CID_MPEG_VIDEO_VBV_DELAY";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER
		case V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER: return "V4L2_CID_MPEG_VIDEO_REPEAT_SEQ_HEADER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MV_H_SEARCH_RANGE
		case V4L2_CID_MPEG_VIDEO_MV_H_SEARCH_RANGE: return "V4L2_CID_MPEG_VIDEO_MV_H_SEARCH_RANGE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE
		case V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE: return "V4L2_CID_MPEG_VIDEO_MV_V_SEARCH_RANGE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME
		case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME: return "V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID
		case V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID: return "V4L2_CID_MPEG_VIDEO_BASELAYER_PRIORITY_ID";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_AU_DELIMITER
		case V4L2_CID_MPEG_VIDEO_AU_DELIMITER: return "V4L2_CID_MPEG_VIDEO_AU_DELIMITER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_LTR_COUNT
		case V4L2_CID_MPEG_VIDEO_LTR_COUNT: return "V4L2_CID_MPEG_VIDEO_LTR_COUNT";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX
		case V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX: return "V4L2_CID_MPEG_VIDEO_FRAME_LTR_INDEX";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES
		case V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES: return "V4L2_CID_MPEG_VIDEO_USE_LTR_FRAMES";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DEC_CONCEAL_COLOR
		case V4L2_CID_MPEG_VIDEO_DEC_CONCEAL_COLOR: return "V4L2_CID_MPEG_VIDEO_DEC_CONCEAL_COLOR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD
		case V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD: return "V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE
		case V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE: return "V4L2_CID_MPEG_VIDEO_INTRA_REFRESH_PERIOD_TYPE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG2_LEVEL
		case V4L2_CID_MPEG_VIDEO_MPEG2_LEVEL: return "V4L2_CID_MPEG_VIDEO_MPEG2_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG2_PROFILE
		case V4L2_CID_MPEG_VIDEO_MPEG2_PROFILE: return "V4L2_CID_MPEG_VIDEO_MPEG2_PROFILE";
#endif
#ifdef V4L2_CID_FWHT_I_FRAME_QP
		case V4L2_CID_FWHT_I_FRAME_QP: return "V4L2_CID_FWHT_I_FRAME_QP";
#endif
#ifdef V4L2_CID_FWHT_P_FRAME_QP
		case V4L2_CID_FWHT_P_FRAME_QP: return "V4L2_CID_FWHT_P_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H263_I_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_H263_I_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_H263_I_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H263_P_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_H263_P_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_H263_P_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H263_B_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_H263_B_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_H263_B_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H263_MIN_QP
		case V4L2_CID_MPEG_VIDEO_H263_MIN_QP: return "V4L2_CID_MPEG_VIDEO_H263_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H263_MAX_QP
		case V4L2_CID_MPEG_VIDEO_H263_MAX_QP: return "V4L2_CID_MPEG_VIDEO_H263_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_H264_B_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_MIN_QP
		case V4L2_CID_MPEG_VIDEO_H264_MIN_QP: return "V4L2_CID_MPEG_VIDEO_H264_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_MAX_QP
		case V4L2_CID_MPEG_VIDEO_H264_MAX_QP: return "V4L2_CID_MPEG_VIDEO_H264_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM
		case V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM: return "V4L2_CID_MPEG_VIDEO_H264_8X8_TRANSFORM";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE
		case V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE: return "V4L2_CID_MPEG_VIDEO_H264_CPB_SIZE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE
		case V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE: return "V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_I_PERIOD
		case V4L2_CID_MPEG_VIDEO_H264_I_PERIOD: return "V4L2_CID_MPEG_VIDEO_H264_I_PERIOD";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_LEVEL
		case V4L2_CID_MPEG_VIDEO_H264_LEVEL: return "V4L2_CID_MPEG_VIDEO_H264_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA
		case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA: return "V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_ALPHA";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA
		case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA: return "V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_BETA";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE
		case V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE: return "V4L2_CID_MPEG_VIDEO_H264_LOOP_FILTER_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_PROFILE
		case V4L2_CID_MPEG_VIDEO_H264_PROFILE: return "V4L2_CID_MPEG_VIDEO_H264_PROFILE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT
		case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT: return "V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_HEIGHT";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH
		case V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH: return "V4L2_CID_MPEG_VIDEO_H264_VUI_EXT_SAR_WIDTH";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE
		case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE: return "V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_ENABLE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC
		case V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC: return "V4L2_CID_MPEG_VIDEO_H264_VUI_SAR_IDC";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_SEI_FRAME_PACKING
		case V4L2_CID_MPEG_VIDEO_H264_SEI_FRAME_PACKING: return "V4L2_CID_MPEG_VIDEO_H264_SEI_FRAME_PACKING";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_SEI_FP_CURRENT_FRAME_0
		case V4L2_CID_MPEG_VIDEO_H264_SEI_FP_CURRENT_FRAME_0: return "V4L2_CID_MPEG_VIDEO_H264_SEI_FP_CURRENT_FRAME_0";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_SEI_FP_ARRANGEMENT_TYPE
		case V4L2_CID_MPEG_VIDEO_H264_SEI_FP_ARRANGEMENT_TYPE: return "V4L2_CID_MPEG_VIDEO_H264_SEI_FP_ARRANGEMENT_TYPE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_FMO
		case V4L2_CID_MPEG_VIDEO_H264_FMO: return "V4L2_CID_MPEG_VIDEO_H264_FMO";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_FMO_MAP_TYPE
		case V4L2_CID_MPEG_VIDEO_H264_FMO_MAP_TYPE: return "V4L2_CID_MPEG_VIDEO_H264_FMO_MAP_TYPE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_FMO_SLICE_GROUP
		case V4L2_CID_MPEG_VIDEO_H264_FMO_SLICE_GROUP: return "V4L2_CID_MPEG_VIDEO_H264_FMO_SLICE_GROUP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_FMO_CHANGE_DIRECTION
		case V4L2_CID_MPEG_VIDEO_H264_FMO_CHANGE_DIRECTION: return "V4L2_CID_MPEG_VIDEO_H264_FMO_CHANGE_DIRECTION";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_FMO_CHANGE_RATE
		case V4L2_CID_MPEG_VIDEO_H264_FMO_CHANGE_RATE: return "V4L2_CID_MPEG_VIDEO_H264_FMO_CHANGE_RATE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_FMO_RUN_LENGTH
		case V4L2_CID_MPEG_VIDEO_H264_FMO_RUN_LENGTH: return "V4L2_CID_MPEG_VIDEO_H264_FMO_RUN_LENGTH";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_ASO
		case V4L2_CID_MPEG_VIDEO_H264_ASO: return "V4L2_CID_MPEG_VIDEO_H264_ASO";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_ASO_SLICE_ORDER
		case V4L2_CID_MPEG_VIDEO_H264_ASO_SLICE_ORDER: return "V4L2_CID_MPEG_VIDEO_H264_ASO_SLICE_ORDER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING
		case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING: return "V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE
		case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE: return "V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_TYPE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER
		case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER: return "V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP
		case V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP: return "V4L2_CID_MPEG_VIDEO_H264_HIERARCHICAL_CODING_LAYER_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION
		case V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION: return "V4L2_CID_MPEG_VIDEO_H264_CONSTRAINED_INTRA_PREDICTION";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET
		case V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET: return "V4L2_CID_MPEG_VIDEO_H264_CHROMA_QP_INDEX_OFFSET";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP
		case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP: return "V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP
		case V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP: return "V4L2_CID_MPEG_VIDEO_H264_I_FRAME_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP
		case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP: return "V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP
		case V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP: return "V4L2_CID_MPEG_VIDEO_H264_P_FRAME_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP
		case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP: return "V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP
		case V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP: return "V4L2_CID_MPEG_VIDEO_H264_B_FRAME_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L0_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L1_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L2_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L3_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L4_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L5_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR
		case V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR: return "V4L2_CID_MPEG_VIDEO_H264_HIER_CODING_L6_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_I_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_MPEG4_I_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_MPEG4_I_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_P_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_MPEG4_P_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_MPEG4_P_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_B_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_MPEG4_B_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_MPEG4_B_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_MIN_QP
		case V4L2_CID_MPEG_VIDEO_MPEG4_MIN_QP: return "V4L2_CID_MPEG_VIDEO_MPEG4_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_MAX_QP
		case V4L2_CID_MPEG_VIDEO_MPEG4_MAX_QP: return "V4L2_CID_MPEG_VIDEO_MPEG4_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL
		case V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL: return "V4L2_CID_MPEG_VIDEO_MPEG4_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE
		case V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE: return "V4L2_CID_MPEG_VIDEO_MPEG4_PROFILE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_MPEG4_QPEL
		case V4L2_CID_MPEG_VIDEO_MPEG4_QPEL: return "V4L2_CID_MPEG_VIDEO_MPEG4_QPEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS
		case V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS: return "V4L2_CID_MPEG_VIDEO_VPX_NUM_PARTITIONS";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_IMD_DISABLE_4X4
		case V4L2_CID_MPEG_VIDEO_VPX_IMD_DISABLE_4X4: return "V4L2_CID_MPEG_VIDEO_VPX_IMD_DISABLE_4X4";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES
		case V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES: return "V4L2_CID_MPEG_VIDEO_VPX_NUM_REF_FRAMES";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL
		case V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL: return "V4L2_CID_MPEG_VIDEO_VPX_FILTER_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS
		case V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS: return "V4L2_CID_MPEG_VIDEO_VPX_FILTER_SHARPNESS";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD
		case V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD: return "V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_REF_PERIOD";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_SEL
		case V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_SEL: return "V4L2_CID_MPEG_VIDEO_VPX_GOLDEN_FRAME_SEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_MIN_QP
		case V4L2_CID_MPEG_VIDEO_VPX_MIN_QP: return "V4L2_CID_MPEG_VIDEO_VPX_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_MAX_QP
		case V4L2_CID_MPEG_VIDEO_VPX_MAX_QP: return "V4L2_CID_MPEG_VIDEO_VPX_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_VPX_I_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_VPX_P_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VP8_PROFILE
		case V4L2_CID_MPEG_VIDEO_VP8_PROFILE: return "V4L2_CID_MPEG_VIDEO_VP8_PROFILE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VP9_PROFILE
		case V4L2_CID_MPEG_VIDEO_VP9_PROFILE: return "V4L2_CID_MPEG_VIDEO_VP9_PROFILE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_VP9_LEVEL
		case V4L2_CID_MPEG_VIDEO_VP9_LEVEL: return "V4L2_CID_MPEG_VIDEO_VP9_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_TYPE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_PROFILE
		case V4L2_CID_MPEG_VIDEO_HEVC_PROFILE: return "V4L2_CID_MPEG_VIDEO_HEVC_PROFILE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_LEVEL
		case V4L2_CID_MPEG_VIDEO_HEVC_LEVEL: return "V4L2_CID_MPEG_VIDEO_HEVC_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_FRAME_RATE_RESOLUTION
		case V4L2_CID_MPEG_VIDEO_HEVC_FRAME_RATE_RESOLUTION: return "V4L2_CID_MPEG_VIDEO_HEVC_FRAME_RATE_RESOLUTION";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_TIER
		case V4L2_CID_MPEG_VIDEO_HEVC_TIER: return "V4L2_CID_MPEG_VIDEO_HEVC_TIER";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_MAX_PARTITION_DEPTH
		case V4L2_CID_MPEG_VIDEO_HEVC_MAX_PARTITION_DEPTH: return "V4L2_CID_MPEG_VIDEO_HEVC_MAX_PARTITION_DEPTH";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE
		case V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE: return "V4L2_CID_MPEG_VIDEO_HEVC_LOOP_FILTER_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2
		case V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2: return "V4L2_CID_MPEG_VIDEO_HEVC_LF_BETA_OFFSET_DIV2";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2
		case V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2: return "V4L2_CID_MPEG_VIDEO_HEVC_LF_TC_OFFSET_DIV2";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE
		case V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE: return "V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_TYPE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD
		case V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD: return "V4L2_CID_MPEG_VIDEO_HEVC_REFRESH_PERIOD";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_LOSSLESS_CU
		case V4L2_CID_MPEG_VIDEO_HEVC_LOSSLESS_CU: return "V4L2_CID_MPEG_VIDEO_HEVC_LOSSLESS_CU";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_CONST_INTRA_PRED
		case V4L2_CID_MPEG_VIDEO_HEVC_CONST_INTRA_PRED: return "V4L2_CID_MPEG_VIDEO_HEVC_CONST_INTRA_PRED";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_WAVEFRONT
		case V4L2_CID_MPEG_VIDEO_HEVC_WAVEFRONT: return "V4L2_CID_MPEG_VIDEO_HEVC_WAVEFRONT";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_GENERAL_PB
		case V4L2_CID_MPEG_VIDEO_HEVC_GENERAL_PB: return "V4L2_CID_MPEG_VIDEO_HEVC_GENERAL_PB";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID
		case V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID: return "V4L2_CID_MPEG_VIDEO_HEVC_TEMPORAL_ID";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING
		case V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING: return "V4L2_CID_MPEG_VIDEO_HEVC_STRONG_SMOOTHING";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_MAX_NUM_MERGE_MV_MINUS1
		case V4L2_CID_MPEG_VIDEO_HEVC_MAX_NUM_MERGE_MV_MINUS1: return "V4L2_CID_MPEG_VIDEO_HEVC_MAX_NUM_MERGE_MV_MINUS1";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_INTRA_PU_SPLIT
		case V4L2_CID_MPEG_VIDEO_HEVC_INTRA_PU_SPLIT: return "V4L2_CID_MPEG_VIDEO_HEVC_INTRA_PU_SPLIT";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION
		case V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION: return "V4L2_CID_MPEG_VIDEO_HEVC_TMV_PREDICTION";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE
		case V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE: return "V4L2_CID_MPEG_VIDEO_HEVC_WITHOUT_STARTCODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD
		case V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD: return "V4L2_CID_MPEG_VIDEO_HEVC_SIZE_OF_LENGTH_FIELD";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_BR
		case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_BR: return "V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_BR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES
		case V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES: return "V4L2_CID_MPEG_VIDEO_REF_NUMBER_FOR_PFRAMES";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR
		case V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR: return "V4L2_CID_MPEG_VIDEO_PREPEND_SPSPPS_TO_IDR";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY
		case V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY: return "V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE
		case V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE: return "V4L2_CID_MPEG_VIDEO_FRAME_SKIP_MODE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_P_FRAME_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MIN_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP
		case V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP: return "V4L2_CID_MPEG_VIDEO_HEVC_B_FRAME_MAX_QP";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY
		case V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY: return "V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE
		case V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE: return "V4L2_CID_MPEG_VIDEO_DEC_DISPLAY_DELAY_ENABLE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_AV1_PROFILE
		case V4L2_CID_MPEG_VIDEO_AV1_PROFILE: return "V4L2_CID_MPEG_VIDEO_AV1_PROFILE";
#endif
#ifdef V4L2_CID_MPEG_VIDEO_AV1_LEVEL
		case V4L2_CID_MPEG_VIDEO_AV1_LEVEL: return "V4L2_CID_MPEG_VIDEO_AV1_LEVEL";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_SPATIAL_FILTER
		case V4L2_CID_MPEG_CX2341X_VIDEO_SPATIAL_FILTER: return "V4L2_CID_MPEG_CX2341X_VIDEO_SPATIAL_FILTER";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE
		case V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE: return "V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_SPATIAL_FILTER_TYPE";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_SPATIAL_FILTER_TYPE
		case V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_SPATIAL_FILTER_TYPE: return "V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_SPATIAL_FILTER_TYPE";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_TEMPORAL_FILTER_MODE
		case V4L2_CID_MPEG_CX2341X_VIDEO_TEMPORAL_FILTER_MODE: return "V4L2_CID_MPEG_CX2341X_VIDEO_TEMPORAL_FILTER_MODE";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_TEMPORAL_FILTER
		case V4L2_CID_MPEG_CX2341X_VIDEO_TEMPORAL_FILTER: return "V4L2_CID_MPEG_CX2341X_VIDEO_TEMPORAL_FILTER";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_MEDIAN_FILTER_TYPE
		case V4L2_CID_MPEG_CX2341X_VIDEO_MEDIAN_FILTER_TYPE: return "V4L2_CID_MPEG_CX2341X_VIDEO_MEDIAN_FILTER_TYPE";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM
		case V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM: return "V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_BOTTOM";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP
		case V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP: return "V4L2_CID_MPEG_CX2341X_VIDEO_LUMA_MEDIAN_FILTER_TOP";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_BOTTOM
		case V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_BOTTOM: return "V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_BOTTOM";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_TOP
		case V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_TOP: return "V4L2_CID_MPEG_CX2341X_VIDEO_CHROMA_MEDIAN_FILTER_TOP";
#endif
#ifdef V4L2_CID_MPEG_CX2341X_STREAM_INSERT_NAV_PACKETS
		case V4L2_CID_MPEG_CX2341X_STREAM_INSERT_NAV_PACKETS: return "V4L2_CID_MPEG_CX2341X_STREAM_INSERT_NAV_PACKETS";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE
		case V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE: return "V4L2_CID_MPEG_MFC51_VIDEO_DECODER_H264_DISPLAY_DELAY_ENABLE";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE
		case V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE: return "V4L2_CID_MPEG_MFC51_VIDEO_FRAME_SKIP_MODE";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE
		case V4L2_CID_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE: return "V4L2_CID_MPEG_MFC51_VIDEO_FORCE_FRAME_TYPE";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_PADDING
		case V4L2_CID_MPEG_MFC51_VIDEO_PADDING: return "V4L2_CID_MPEG_MFC51_VIDEO_PADDING";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_PADDING_YUV
		case V4L2_CID_MPEG_MFC51_VIDEO_PADDING_YUV: return "V4L2_CID_MPEG_MFC51_VIDEO_PADDING_YUV";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_RC_FIXED_TARGET_BIT
		case V4L2_CID_MPEG_MFC51_VIDEO_RC_FIXED_TARGET_BIT: return "V4L2_CID_MPEG_MFC51_VIDEO_RC_FIXED_TARGET_BIT";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_RC_REACTION_COEFF
		case V4L2_CID_MPEG_MFC51_VIDEO_RC_REACTION_COEFF: return "V4L2_CID_MPEG_MFC51_VIDEO_RC_REACTION_COEFF";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_ACTIVITY
		case V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_ACTIVITY: return "V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_ACTIVITY";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_DARK
		case V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_DARK: return "V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_DARK";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_SMOOTH
		case V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_SMOOTH: return "V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_SMOOTH";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_STATIC
		case V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_STATIC: return "V4L2_CID_MPEG_MFC51_VIDEO_H264_ADAPTIVE_RC_STATIC";
#endif
#ifdef V4L2_CID_MPEG_MFC51_VIDEO_H264_NUM_REF_PIC_FOR_P
		case V4L2_CID_MPEG_MFC51_VIDEO_H264_NUM_REF_PIC_FOR_P: return "V4L2_CID_MPEG_MFC51_VIDEO_H264_NUM_REF_PIC_FOR_P";
#endif
#ifdef V4L2_CID_EXPOSURE_AUTO
		case V4L2_CID_EXPOSURE_AUTO: return "V4L2_CID_EXPOSURE_AUTO";
#endif
#ifdef V4L2_CID_EXPOSURE_ABSOLUTE
		case V4L2_CID_EXPOSURE_ABSOLUTE: return "V4L2_CID_EXPOSURE_ABSOLUTE";
#endif
#ifdef V4L2_CID_EXPOSURE_AUTO_PRIORITY
		case V4L2_CID_EXPOSURE_AUTO_PRIORITY: return "V4L2_CID_EXPOSURE_AUTO_PRIORITY";
#endif
#ifdef V4L2_CID_PAN_RELATIVE
		case V4L2_CID_PAN_RELATIVE: return "V4L2_CID_PAN_RELATIVE";
#endif
#ifdef V4L2_CID_TILT_RELATIVE
		case V4L2_CID_TILT_RELATIVE: return "V4L2_CID_TILT_RELATIVE";
#endif
#ifdef V4L2_CID_PAN_RESET
		case V4L2_CID_PAN_RESET: return "V4L2_CID_PAN_RESET";
#endif
#ifdef V4L2_CID_TILT_RESET
		case V4L2_CID_TILT_RESET: return "V4L2_CID_TILT_RESET";
#endif
#ifdef V4L2_CID_PAN_ABSOLUTE
		case V4L2_CID_PAN_ABSOLUTE: return "V4L2_CID_PAN_ABSOLUTE";
#endif
#ifdef V4L2_CID_TILT_ABSOLUTE
		case V4L2_CID_TILT_ABSOLUTE: return "V4L2_CID_TILT_ABSOLUTE";
#endif
#ifdef V4L2_CID_FOCUS_ABSOLUTE
		case V4L2_CID_FOCUS_ABSOLUTE: return "V4L2_CID_FOCUS_ABSOLUTE";
#endif
#ifdef V4L2_CID_FOCUS_RELATIVE
		case V4L2_CID_FOCUS_RELATIVE: return "V4L2_CID_FOCUS_RELATIVE";
#endif
#ifdef V4L2_CID_FOCUS_AUTO
		case V4L2_CID_FOCUS_AUTO: return "V4L2_CID_FOCUS_AUTO";
#endif
#ifdef V4L2_CID_ZOOM_ABSOLUTE
		case V4L2_CID_ZOOM_ABSOLUTE: return "V4L2_CID_ZOOM_ABSOLUTE";
#endif
#ifdef V4L2_CID_ZOOM_RELATIVE
		case V4L2_CID_ZOOM_RELATIVE: return "V4L2_CID_ZOOM_RELATIVE";
#endif
#ifdef V4L2_CID_ZOOM_CONTINUOUS
		case V4L2_CID_ZOOM_CONTINUOUS: return "V4L2_CID_ZOOM_CONTINUOUS";
#endif
#ifdef V4L2_CID_PRIVACY
		case V4L2_CID_PRIVACY: return "V4L2_CID_PRIVACY";
#endif
#ifdef V4L2_CID_IRIS_ABSOLUTE
		case V4L2_CID_IRIS_ABSOLUTE: return "V4L2_CID_IRIS_ABSOLUTE";
#endif
#ifdef V4L2_CID_IRIS_RELATIVE
		case V4L2_CID_IRIS_RELATIVE: return "V4L2_CID_IRIS_RELATIVE";
#endif
#ifdef V4L2_CID_AUTO_EXPOSURE_BIAS
		case V4L2_CID_AUTO_EXPOSURE_BIAS: return "V4L2_CID_AUTO_EXPOSURE_BIAS";
#endif
#ifdef V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE
		case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE: return "V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE";
#endif
#ifdef V4L2_CID_WIDE_DYNAMIC_RANGE
		case V4L2_CID_WIDE_DYNAMIC_RANGE: return "V4L2_CID_WIDE_DYNAMIC_RANGE";
#endif
#ifdef V4L2_CID_IMAGE_STABILIZATION
		case V4L2_CID_IMAGE_STABILIZATION: return "V4L2_CID_IMAGE_STABILIZATION";
#endif
#ifdef V4L2_CID_ISO_SENSITIVITY
		case V4L2_CID_ISO_SENSITIVITY: return "V4L2_CID_ISO_SENSITIVITY";
#endif
#ifdef V4L2_CID_ISO_SENSITIVITY_AUTO
		case V4L2_CID_ISO_SENSITIVITY_AUTO: return "V4L2_CID_ISO_SENSITIVITY_AUTO";
#endif
#ifdef V4L2_CID_EXPOSURE_METERING
		case V4L2_CID_EXPOSURE_METERING: return "V4L2_CID_EXPOSURE_METERING";
#endif
#ifdef V4L2_CID_SCENE_MODE
		case V4L2_CID_SCENE_MODE: return "V4L2_CID_SCENE_MODE";
#endif
#ifdef V4L2_CID_3A_LOCK
		case V4L2_CID_3A_LOCK: return "V4L2_CID_3A_LOCK";
#endif
#ifdef V4L2_CID_AUTO_FOCUS_START
		case V4L2_CID_AUTO_FOCUS_START: return "V4L2_CID_AUTO_FOCUS_START";
#endif
#ifdef V4L2_CID_AUTO_FOCUS_STOP
		case V4L2_CID_AUTO_FOCUS_STOP: return "V4L2_CID_AUTO_FOCUS_STOP";
#endif
#ifdef V4L2_CID_AUTO_FOCUS_STATUS
		case V4L2_CID_AUTO_FOCUS_STATUS: return "V4L2_CID_AUTO_FOCUS_STATUS";
#endif
#ifdef V4L2_CID_AUTO_FOCUS_RANGE
		case V4L2_CID_AUTO_FOCUS_RANGE: return "V4L2_CID_AUTO_FOCUS_RANGE";
#endif
#ifdef V4L2_CID_PAN_SPEED
		case V4L2_CID_PAN_SPEED: return "V4L2_CID_PAN_SPEED";
#endif
#ifdef V4L2_CID_TILT_SPEED
		case V4L2_CID_TILT_SPEED: return "V4L2_CID_TILT_SPEED";
#endif
#ifdef V4L2_CID_CAMERA_ORIENTATION
		case V4L2_CID_CAMERA_ORIENTATION: return "V4L2_CID_CAMERA_ORIENTATION";
#endif
#ifdef V4L2_CID_CAMERA_SENSOR_ROTATION
		case V4L2_CID_CAMERA_SENSOR_ROTATION: return "V4L2_CID_CAMERA_SENSOR_ROTATION";
#endif
#ifdef V4L2_CID_HDR_SENSOR_MODE
		case V4L2_CID_HDR_SENSOR_MODE: return "V4L2_CID_HDR_SENSOR_MODE";
#endif
#ifdef V4L2_CID_RDS_TX_DEVIATION
		case V4L2_CID_RDS_TX_DEVIATION: return "V4L2_CID_RDS_TX_DEVIATION";
#endif
#ifdef V4L2_CID_RDS_TX_PI
		case V4L2_CID_RDS_TX_PI: return "V4L2_CID_RDS_TX_PI";
#endif
#ifdef V4L2_CID_RDS_TX_PTY
		case V4L2_CID_RDS_TX_PTY: return "V4L2_CID_RDS_TX_PTY";
#endif
#ifdef V4L2_CID_RDS_TX_PS_NAME
		case V4L2_CID_RDS_TX_PS_NAME: return "V4L2_CID_RDS_TX_PS_NAME";
#endif
#ifdef V4L2_CID_RDS_TX_RADIO_TEXT
		case V4L2_CID_RDS_TX_RADIO_TEXT: return "V4L2_CID_RDS_TX_RADIO_TEXT";
#endif
#ifdef V4L2_CID_RDS_TX_MONO_STEREO
		case V4L2_CID_RDS_TX_MONO_STEREO: return "V4L2_CID_RDS_TX_MONO_STEREO";
#endif
#ifdef V4L2_CID_RDS_TX_ARTIFICIAL_HEAD
		case V4L2_CID_RDS_TX_ARTIFICIAL_HEAD: return "V4L2_CID_RDS_TX_ARTIFICIAL_HEAD";
#endif
#ifdef V4L2_CID_RDS_TX_COMPRESSED
		case V4L2_CID_RDS_TX_COMPRESSED: return "V4L2_CID_RDS_TX_COMPRESSED";
#endif
#ifdef V4L2_CID_RDS_TX_DYNAMIC_PTY
		case V4L2_CID_RDS_TX_DYNAMIC_PTY: return "V4L2_CID_RDS_TX_DYNAMIC_PTY";
#endif
#ifdef V4L2_CID_RDS_TX_TRAFFIC_ANNOUNCEMENT
		case V4L2_CID_RDS_TX_TRAFFIC_ANNOUNCEMENT: return "V4L2_CID_RDS_TX_TRAFFIC_ANNOUNCEMENT";
#endif
#ifdef V4L2_CID_RDS_TX_TRAFFIC_PROGRAM
		case V4L2_CID_RDS_TX_TRAFFIC_PROGRAM: return "V4L2_CID_RDS_TX_TRAFFIC_PROGRAM";
#endif
#ifdef V4L2_CID_RDS_TX_MUSIC_SPEECH
		case V4L2_CID_RDS_TX_MUSIC_SPEECH: return "V4L2_CID_RDS_TX_MUSIC_SPEECH";
#endif
#ifdef V4L2_CID_RDS_TX_ALT_FREQS_ENABLE
		case V4L2_CID_RDS_TX_ALT_FREQS_ENABLE: return "V4L2_CID_RDS_TX_ALT_FREQS_ENABLE";
#endif
#ifdef V4L2_CID_RDS_TX_ALT_FREQS
		case V4L2_CID_RDS_TX_ALT_FREQS: return "V4L2_CID_RDS_TX_ALT_FREQS";
#endif
#ifdef V4L2_CID_AUDIO_LIMITER_ENABLED
		case V4L2_CID_AUDIO_LIMITER_ENABLED: return "V4L2_CID_AUDIO_LIMITER_ENABLED";
#endif
#ifdef V4L2_CID_AUDIO_LIMITER_RELEASE_TIME
		case V4L2_CID_AUDIO_LIMITER_RELEASE_TIME: return "V4L2_CID_AUDIO_LIMITER_RELEASE_TIME";
#endif
#ifdef V4L2_CID_AUDIO_LIMITER_DEVIATION
		case V4L2_CID_AUDIO_LIMITER_DEVIATION: return "V4L2_CID_AUDIO_LIMITER_DEVIATION";
#endif
#ifdef V4L2_CID_AUDIO_COMPRESSION_ENABLED
		case V4L2_CID_AUDIO_COMPRESSION_ENABLED: return "V4L2_CID_AUDIO_COMPRESSION_ENABLED";
#endif
#ifdef V4L2_CID_AUDIO_COMPRESSION_GAIN
		case V4L2_CID_AUDIO_COMPRESSION_GAIN: return "V4L2_CID_AUDIO_COMPRESSION_GAIN";
#endif
#ifdef V4L2_CID_AUDIO_COMPRESSION_THRESHOLD
		case V4L2_CID_AUDIO_COMPRESSION_THRESHOLD: return "V4L2_CID_AUDIO_COMPRESSION_THRESHOLD";
#endif
#ifdef V4L2_CID_AUDIO_COMPRESSION_ATTACK_TIME
		case V4L2_CID_AUDIO_COMPRESSION_ATTACK_TIME: return "V4L2_CID_AUDIO_COMPRESSION_ATTACK_TIME";
#endif
#ifdef V4L2_CID_AUDIO_COMPRESSION_RELEASE_TIME
		case V4L2_CID_AUDIO_COMPRESSION_RELEASE_TIME: return "V4L2_CID_AUDIO_COMPRESSION_RELEASE_TIME";
#endif
#ifdef V4L2_CID_PILOT_TONE_ENABLED
		case V4L2_CID_PILOT_TONE_ENABLED: return "V4L2_CID_PILOT_TONE_ENABLED";
#endif
#ifdef V4L2_CID_PILOT_TONE_DEVIATION
		case V4L2_CID_PILOT_TONE_DEVIATION: return "V4L2_CID_PILOT_TONE_DEVIATION";
#endif
#ifdef V4L2_CID_PILOT_TONE_FREQUENCY
		case V4L2_CID_PILOT_TONE_FREQUENCY: return "V4L2_CID_PILOT_TONE_FREQUENCY";
#endif
#ifdef V4L2_CID_TUNE_PREEMPHASIS
		case V4L2_CID_TUNE_PREEMPHASIS: return "V4L2_CID_TUNE_PREEMPHASIS";
#endif
#ifdef V4L2_CID_TUNE_POWER_LEVEL
		case V4L2_CID_TUNE_POWER_LEVEL: return "V4L2_CID_TUNE_POWER_LEVEL";
#endif
#ifdef V4L2_CID_TUNE_ANTENNA_CAPACITOR
		case V4L2_CID_TUNE_ANTENNA_CAPACITOR: return "V4L2_CID_TUNE_ANTENNA_CAPACITOR";
#endif
#ifdef V4L2_CID_FLASH_LED_MODE
		case V4L2_CID_FLASH_LED_MODE: return "V4L2_CID_FLASH_LED_MODE";
#endif
#ifdef V4L2_CID_FLASH_STROBE_SOURCE
		case V4L2_CID_FLASH_STROBE_SOURCE: return "V4L2_CID_FLASH_STROBE_SOURCE";
#endif
#ifdef V4L2_CID_FLASH_STROBE
		case V4L2_CID_FLASH_STROBE: return "V4L2_CID_FLASH_STROBE";
#endif
#ifdef V4L2_CID_FLASH_STROBE_STOP
		case V4L2_CID_FLASH_STROBE_STOP: return "V4L2_CID_FLASH_STROBE_STOP";
#endif
#ifdef V4L2_CID_FLASH_STROBE_STATUS
		case V4L2_CID_FLASH_STROBE_STATUS: return "V4L2_CID_FLASH_STROBE_STATUS";
#endif
#ifdef V4L2_CID_FLASH_TIMEOUT
		case V4L2_CID_FLASH_TIMEOUT: return "V4L2_CID_FLASH_TIMEOUT";
#endif
#ifdef V4L2_CID_FLASH_INTENSITY
		case V4L2_CID_FLASH_INTENSITY: return "V4L2_CID_FLASH_INTENSITY";
#endif
#ifdef V4L2_CID_FLASH_TORCH_INTENSITY
		case V4L2_CID_FLASH_TORCH_INTENSITY: return "V4L2_CID_FLASH_TORCH_INTENSITY";
#endif
#ifdef V4L2_CID_FLASH_INDICATOR_INTENSITY
		case V4L2_CID_FLASH_INDICATOR_INTENSITY: return "V4L2_CID_FLASH_INDICATOR_INTENSITY";
#endif
#ifdef V4L2_CID_FLASH_FAULT
		case V4L2_CID_FLASH_FAULT: return "V4L2_CID_FLASH_FAULT";
#endif
#ifdef V4L2_CID_FLASH_CHARGE
		case V4L2_CID_FLASH_CHARGE: return "V4L2_CID_FLASH_CHARGE";
#endif
#ifdef V4L2_CID_FLASH_READY
		case V4L2_CID_FLASH_READY: return "V4L2_CID_FLASH_READY";
#endif
#ifdef V4L2_CID_JPEG_CHROMA_SUBSAMPLING
		case V4L2_CID_JPEG_CHROMA_SUBSAMPLING: return "V4L2_CID_JPEG_CHROMA_SUBSAMPLING";
#endif
#ifdef V4L2_CID_JPEG_RESTART_INTERVAL
		case V4L2_CID_JPEG_RESTART_INTERVAL: return "V4L2_CID_JPEG_RESTART_INTERVAL";
#endif
#ifdef V4L2_CID_JPEG_COMPRESSION_QUALITY
		case V4L2_CID_JPEG_COMPRESSION_QUALITY: return "V4L2_CID_JPEG_COMPRESSION_QUALITY";
#endif
#ifdef V4L2_CID_JPEG_ACTIVE_MARKER
		case V4L2_CID_JPEG_ACTIVE_MARKER: return "V4L2_CID_JPEG_ACTIVE_MARKER";
#endif
#ifdef V4L2_CID_VBLANK
		case V4L2_CID_VBLANK: return "V4L2_CID_VBLANK";
#endif
#ifdef V4L2_CID_HBLANK
		case V4L2_CID_HBLANK: return "V4L2_CID_HBLANK";
#endif
#ifdef V4L2_CID_ANALOGUE_GAIN
		case V4L2_CID_ANALOGUE_GAIN: return "V4L2_CID_ANALOGUE_GAIN";
#endif
#ifdef V4L2_CID_TEST_PATTERN_RED
		case V4L2_CID_TEST_PATTERN_RED: return "V4L2_CID_TEST_PATTERN_RED";
#endif
#ifdef V4L2_CID_TEST_PATTERN_GREENR
		case V4L2_CID_TEST_PATTERN_GREENR: return "V4L2_CID_TEST_PATTERN_GREENR";
#endif
#ifdef V4L2_CID_TEST_PATTERN_BLUE
		case V4L2_CID_TEST_PATTERN_BLUE: return "V4L2_CID_TEST_PATTERN_BLUE";
#endif
#ifdef V4L2_CID_TEST_PATTERN_GREENB
		case V4L2_CID_TEST_PATTERN_GREENB: return "V4L2_CID_TEST_PATTERN_GREENB";
#endif
#ifdef V4L2_CID_UNIT_CELL_SIZE
		case V4L2_CID_UNIT_CELL_SIZE: return "V4L2_CID_UNIT_CELL_SIZE";
#endif
#ifdef V4L2_CID_NOTIFY_GAINS
		case V4L2_CID_NOTIFY_GAINS: return "V4L2_CID_NOTIFY_GAINS";
#endif
#ifdef V4L2_CID_LINK_FREQ
		case V4L2_CID_LINK_FREQ: return "V4L2_CID_LINK_FREQ";
#endif
#ifdef V4L2_CID_PIXEL_RATE
		case V4L2_CID_PIXEL_RATE: return "V4L2_CID_PIXEL_RATE";
#endif
#ifdef V4L2_CID_TEST_PATTERN
		case V4L2_CID_TEST_PATTERN: return "V4L2_CID_TEST_PATTERN";
#endif
#ifdef V4L2_CID_DEINTERLACING_MODE
		case V4L2_CID_DEINTERLACING_MODE: return "V4L2_CID_DEINTERLACING_MODE";
#endif
#ifdef V4L2_CID_DIGITAL_GAIN
		case V4L2_CID_DIGITAL_GAIN: return "V4L2_CID_DIGITAL_GAIN";
#endif
#ifdef V4L2_CID_DV_TX_HOTPLUG
		case V4L2_CID_DV_TX_HOTPLUG: return "V4L2_CID_DV_TX_HOTPLUG";
#endif
#ifdef V4L2_CID_DV_TX_RXSENSE
		case V4L2_CID_DV_TX_RXSENSE: return "V4L2_CID_DV_TX_RXSENSE";
#endif
#ifdef V4L2_CID_DV_TX_EDID_PRESENT
		case V4L2_CID_DV_TX_EDID_PRESENT: return "V4L2_CID_DV_TX_EDID_PRESENT";
#endif
#ifdef V4L2_CID_DV_TX_MODE
		case V4L2_CID_DV_TX_MODE: return "V4L2_CID_DV_TX_MODE";
#endif
#ifdef V4L2_CID_DV_TX_RGB_RANGE
		case V4L2_CID_DV_TX_RGB_RANGE: return "V4L2_CID_DV_TX_RGB_RANGE";
#endif
#ifdef V4L2_CID_DV_TX_IT_CONTENT_TYPE
		case V4L2_CID_DV_TX_IT_CONTENT_TYPE: return "V4L2_CID_DV_TX_IT_CONTENT_TYPE";
#endif
#ifdef V4L2_CID_DV_RX_POWER_PRESENT
		case V4L2_CID_DV_RX_POWER_PRESENT: return "V4L2_CID_DV_RX_POWER_PRESENT";
#endif
#ifdef V4L2_CID_DV_RX_RGB_RANGE
		case V4L2_CID_DV_RX_RGB_RANGE: return "V4L2_CID_DV_RX_RGB_RANGE";
#endif
#ifdef V4L2_CID_DV_RX_IT_CONTENT_TYPE
		case V4L2_CID_DV_RX_IT_CONTENT_TYPE: return "V4L2_CID_DV_RX_IT_CONTENT_TYPE";
#endif
#ifdef V4L2_CID_TUNE_DEEMPHASIS
		case V4L2_CID_TUNE_DEEMPHASIS: return "V4L2_CID_TUNE_DEEMPHASIS";
#endif
#ifdef V4L2_CID_RDS_RECEPTION
		case V4L2_CID_RDS_RECEPTION: return "V4L2_CID_RDS_RECEPTION";
#endif
#ifdef V4L2_CID_RDS_RX_PTY
		case V4L2_CID_RDS_RX_PTY: return "V4L2_CID_RDS_RX_PTY";
#endif
#ifdef V4L2_CID_RDS_RX_PS_NAME
		case V4L2_CID_RDS_RX_PS_NAME: return "V4L2_CID_RDS_RX_PS_NAME";
#endif
#ifdef V4L2_CID_RDS_RX_RADIO_TEXT
		case V4L2_CID_RDS_RX_RADIO_TEXT: return "V4L2_CID_RDS_RX_RADIO_TEXT";
#endif
#ifdef V4L2_CID_RDS_RX_TRAFFIC_ANNOUNCEMENT
		case V4L2_CID_RDS_RX_TRAFFIC_ANNOUNCEMENT: return "V4L2_CID_RDS_RX_TRAFFIC_ANNOUNCEMENT";
#endif
#ifdef V4L2_CID_RDS_RX_TRAFFIC_PROGRAM
		case V4L2_CID_RDS_RX_TRAFFIC_PROGRAM: return "V4L2_CID_RDS_RX_TRAFFIC_PROGRAM";
#endif
#ifdef V4L2_CID_RDS_RX_MUSIC_SPEECH
		case V4L2_CID_RDS_RX_MUSIC_SPEECH: return "V4L2_CID_RDS_RX_MUSIC_SPEECH";
#endif
#ifdef V4L2_CID_RF_TUNER_BANDWIDTH_AUTO
		case V4L2_CID_RF_TUNER_BANDWIDTH_AUTO: return "V4L2_CID_RF_TUNER_BANDWIDTH_AUTO";
#endif
#ifdef V4L2_CID_RF_TUNER_BANDWIDTH
		case V4L2_CID_RF_TUNER_BANDWIDTH: return "V4L2_CID_RF_TUNER_BANDWIDTH";
#endif
#ifdef V4L2_CID_RF_TUNER_RF_GAIN
		case V4L2_CID_RF_TUNER_RF_GAIN: return "V4L2_CID_RF_TUNER_RF_GAIN";
#endif
#ifdef V4L2_CID_RF_TUNER_LNA_GAIN_AUTO
		case V4L2_CID_RF_TUNER_LNA_GAIN_AUTO: return "V4L2_CID_RF_TUNER_LNA_GAIN_AUTO";
#endif
#ifdef V4L2_CID_RF_TUNER_LNA_GAIN
		case V4L2_CID_RF_TUNER_LNA_GAIN: return "V4L2_CID_RF_TUNER_LNA_GAIN";
#endif
#ifdef V4L2_CID_RF_TUNER_MIXER_GAIN_AUTO
		case V4L2_CID_RF_TUNER_MIXER_GAIN_AUTO: return "V4L2_CID_RF_TUNER_MIXER_GAIN_AUTO";
#endif
#ifdef V4L2_CID_RF_TUNER_MIXER_GAIN
		case V4L2_CID_RF_TUNER_MIXER_GAIN: return "V4L2_CID_RF_TUNER_MIXER_GAIN";
#endif
#ifdef V4L2_CID_RF_TUNER_IF_GAIN_AUTO
		case V4L2_CID_RF_TUNER_IF_GAIN_AUTO: return "V4L2_CID_RF_TUNER_IF_GAIN_AUTO";
#endif
#ifdef V4L2_CID_RF_TUNER_IF_GAIN
		case V4L2_CID_RF_TUNER_IF_GAIN: return "V4L2_CID_RF_TUNER_IF_GAIN";
#endif
#ifdef V4L2_CID_RF_TUNER_PLL_LOCK
		case V4L2_CID_RF_TUNER_PLL_LOCK: return "V4L2_CID_RF_TUNER_PLL_LOCK";
#endif
#ifdef V4L2_CID_DETECT_MD_MODE
		case V4L2_CID_DETECT_MD_MODE: return "V4L2_CID_DETECT_MD_MODE";
#endif
#ifdef V4L2_CID_DETECT_MD_GLOBAL_THRESHOLD
		case V4L2_CID_DETECT_MD_GLOBAL_THRESHOLD: return "V4L2_CID_DETECT_MD_GLOBAL_THRESHOLD";
#endif
#ifdef V4L2_CID_DETECT_MD_THRESHOLD_GRID
		case V4L2_CID_DETECT_MD_THRESHOLD_GRID: return "V4L2_CID_DETECT_MD_THRESHOLD_GRID";
#endif
#ifdef V4L2_CID_DETECT_MD_REGION_GRID
		case V4L2_CID_DETECT_MD_REGION_GRID: return "V4L2_CID_DETECT_MD_REGION_GRID";
#endif
#ifdef V4L2_CID_STATELESS_H264_DECODE_MODE
		case V4L2_CID_STATELESS_H264_DECODE_MODE: return "V4L2_CID_STATELESS_H264_DECODE_MODE";
#endif
#ifdef V4L2_CID_STATELESS_H264_START_CODE
		case V4L2_CID_STATELESS_H264_START_CODE: return "V4L2_CID_STATELESS_H264_START_CODE";
#endif
#ifdef V4L2_CID_STATELESS_H264_SPS
		case V4L2_CID_STATELESS_H264_SPS: return "V4L2_CID_STATELESS_H264_SPS";
#endif
		default: return "UNKNOWN";
	}
}

const char* v4l2CtrlTypeName(uint32_t ctrl_type) {
	switch (ctrl_type) {
		case V4L2_CTRL_TYPE_INTEGER: return "V4L2_CTRL_TYPE_INTEGER";
		case V4L2_CTRL_TYPE_BOOLEAN: return "V4L2_CTRL_TYPE_BOOLEAN";
		case V4L2_CTRL_TYPE_MENU: return "V4L2_CTRL_TYPE_MENU";
		case V4L2_CTRL_TYPE_BUTTON: return "V4L2_CTRL_TYPE_BUTTON";
		case V4L2_CTRL_TYPE_INTEGER64: return "V4L2_CTRL_TYPE_INTEGER64";
		case V4L2_CTRL_TYPE_CTRL_CLASS: return "V4L2_CTRL_TYPE_CTRL_CLASS";
		case V4L2_CTRL_TYPE_STRING: return "V4L2_CTRL_TYPE_STRING";
		case V4L2_CTRL_TYPE_BITMASK: return "V4L2_CTRL_TYPE_BITMASK";
		case V4L2_CTRL_TYPE_INTEGER_MENU: return "V4L2_CTRL_TYPE_INTEGER_MENU";
		case V4L2_CTRL_TYPE_U8: return "V4L2_CTRL_TYPE_U8";
		case V4L2_CTRL_TYPE_U16: return "V4L2_CTRL_TYPE_U16";
		case V4L2_CTRL_TYPE_U32: return "V4L2_CTRL_TYPE_U32";
		case V4L2_CTRL_TYPE_AREA: return "V4L2_CTRL_TYPE_AREA";
		case V4L2_CTRL_TYPE_HDR10_CLL_INFO: return "V4L2_CTRL_TYPE_HDR10_CLL_INFO";
		case V4L2_CTRL_TYPE_HDR10_MASTERING_DISPLAY: return "V4L2_CTRL_TYPE_HDR10_MASTERING_DISPLAY";
		case V4L2_CTRL_TYPE_H264_SPS: return "V4L2_CTRL_TYPE_H264_SPS";
		case V4L2_CTRL_TYPE_H264_PPS: return "V4L2_CTRL_TYPE_H264_PPS";
		case V4L2_CTRL_TYPE_H264_SCALING_MATRIX: return "V4L2_CTRL_TYPE_H264_SCALING_MATRIX";
		case V4L2_CTRL_TYPE_H264_SLICE_PARAMS: return "V4L2_CTRL_TYPE_H264_SLICE_PARAMS";
		case V4L2_CTRL_TYPE_H264_DECODE_PARAMS: return "V4L2_CTRL_TYPE_H264_DECODE_PARAMS";
		case V4L2_CTRL_TYPE_H264_PRED_WEIGHTS: return "V4L2_CTRL_TYPE_H264_PRED_WEIGHTS";
		case V4L2_CTRL_TYPE_FWHT_PARAMS: return "V4L2_CTRL_TYPE_FWHT_PARAMS";
		case V4L2_CTRL_TYPE_VP8_FRAME: return "V4L2_CTRL_TYPE_VP8_FRAME";
		case V4L2_CTRL_TYPE_MPEG2_QUANTISATION: return "V4L2_CTRL_TYPE_MPEG2_QUANTISATION";
		case V4L2_CTRL_TYPE_MPEG2_SEQUENCE: return "V4L2_CTRL_TYPE_MPEG2_SEQUENCE";
		case V4L2_CTRL_TYPE_MPEG2_PICTURE: return "V4L2_CTRL_TYPE_MPEG2_PICTURE";
		case V4L2_CTRL_TYPE_VP9_COMPRESSED_HDR: return "V4L2_CTRL_TYPE_VP9_COMPRESSED_HDR";
		case V4L2_CTRL_TYPE_VP9_FRAME: return "V4L2_CTRL_TYPE_VP9_FRAME";
		case V4L2_CTRL_TYPE_HEVC_SPS: return "V4L2_CTRL_TYPE_HEVC_SPS";
		case V4L2_CTRL_TYPE_HEVC_PPS: return "V4L2_CTRL_TYPE_HEVC_PPS";
		case V4L2_CTRL_TYPE_HEVC_SLICE_PARAMS: return "V4L2_CTRL_TYPE_HEVC_SLICE_PARAMS";
		case V4L2_CTRL_TYPE_HEVC_SCALING_MATRIX: return "V4L2_CTRL_TYPE_HEVC_SCALING_MATRIX";
		case V4L2_CTRL_TYPE_HEVC_DECODE_PARAMS: return "V4L2_CTRL_TYPE_HEVC_DECODE_PARAMS";
#if 0 // how to detect v4l2 version to cull these?
		case V4L2_CTRL_TYPE_AV1_SEQUENCE: return "V4L2_CTRL_TYPE_AV1_SEQUENCE";
		case V4L2_CTRL_TYPE_AV1_TILE_GROUP_ENTRY: return "V4L2_CTRL_TYPE_AV1_TILE_GROUP_ENTRY";
		case V4L2_CTRL_TYPE_AV1_FRAME: return "V4L2_CTRL_TYPE_AV1_FRAME";
		case V4L2_CTRL_TYPE_AV1_FILM_GRAIN: return "V4L2_CTRL_TYPE_AV1_FILM_GRAIN";
#endif
		default: return "UNKNOWN";
	}
}
