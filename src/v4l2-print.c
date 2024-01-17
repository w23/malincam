#include "v4l2.h"
#include "v4l2-subdev.h"

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
			LOGE("Unimplemented buffer format type");
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
			LOGI("  buf.m.offset = %d", buf->m.offset);
			LOGI("  buf.m.userptr = %08lx", buf->m.userptr);
			LOGI("  buf.m.fd = %d", buf->m.fd);
			//struct v4l2_plane *planes; // TODO
			break;
		case V4L2_MEMORY_USERPTR:
		case V4L2_MEMORY_DMABUF:
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
	LOGI("    fse.type = %s", v4l2FrmSizeTypeName(fse->type));
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
	LOGI("fmt.pad = %d", format->pad);
	LOGI("fmt.which = %d", format->which); // TODO
	v4l2PrintMbusFramefmt(&format->format);
}

#define MCAM_SEL_TGT_LIST(X) \
	X(V4L2_SEL_TGT_CROP) \
	X(V4L2_SEL_TGT_CROP_DEFAULT) \
	X(V4L2_SEL_TGT_CROP_BOUNDS) \
	X(V4L2_SEL_TGT_NATIVE_SIZE) \
	X(V4L2_SEL_TGT_COMPOSE) \
	X(V4L2_SEL_TGT_COMPOSE_DEFAULT) \
	X(V4L2_SEL_TGT_COMPOSE_BOUNDS) \
	X(V4L2_SEL_TGT_COMPOSE_PADDED) \

void v4l2PrintSelTgt(uint32_t bits) {
#define X(bit) if (bits & bit) LOGI("  %s", #bit);
	MCAM_SEL_TGT_LIST(X)
#undef X
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
	LOGI("sel.target = %08x", sel->target);
	v4l2PrintSelTgt(sel->target);
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
	LOGI("  fiv.pad = %d", fiv->pad);
	LOGI("  fiv.index = %d", fiv->index);
	LOGI("  fiv.code = %s (%08x)", v4l2MbusFmtName(fiv->code), fiv->code);
	LOGI("  fiv.which = %d", fiv->which);
	LOGI("  fiv.width = %d", fiv->width);
	LOGI("  fiv.height = %d", fiv->height);
	v4l2PrintInterval("  fiv.interval = ", fiv->interval);
}
