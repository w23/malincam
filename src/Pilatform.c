#include "Pilatform.h"

#include "Node.h"
#include "device.h"
#include "subdev.h"

#include "common.h"

#include <stdlib.h>
#include <memory.h>

// 120fps sensor settings for HQ camera
#define SENSOR_WIDTH 1332
#define SENSOR_HEIGHT 990
#define SENSOR_BUSFMT MEDIA_BUS_FMT_SRGGB10_1X10
//#define SENSOR_BUSFMT MEDIA_BUS_FMT_SGRBG10_1X10

// TODO detect flipping
#define CAMERA_PIXFMT V4L2_PIX_FMT_SBGGR10P
//#define CAMERA_PIXFMT V4L2_PIX_FMT_SRGGB10P
//#define CAMERA_PIXFMT V4L2_PIX_FMT_SGRBG10P

//#define CROP_TO_720P
#define CROP_TO_976
#ifdef CROP_TO_720P
#define ISP_CROP_WIDTH 1280
#define ISP_CROP_HEIGHT 720
#elif defined(CROP_TO_976)
// Needed for using JPEG encoder
#define ISP_CROP_WIDTH 1332
#define ISP_CROP_HEIGHT 976
#else
#define ISP_CROP_WIDTH SENSOR_WIDTH
#define ISP_CROP_HEIGHT SENSOR_HEIGHT
#endif

#define ISP_OUTPUT_PIXFMT V4L2_PIX_FMT_YUV420

typedef struct {
	Node node;

	Subdev *sensor;
	Device *camera;
} PiCamera;

static void cameraDtor(Node *node) {
	if (!node)
		return;

	PiCamera *cam = (PiCamera*)node;
	deviceClose(cam->camera);
	subdevClose(cam->sensor);
	free(cam);
}

struct Node *piOpenCamera(void) {
	static const char* const sensor_node = "/dev/v4l-subdev0";
	static const char* const camera_node = "/dev/video0";

	Device *camera = NULL;

	// 1. Open and set up sensor subdevice
	Subdev *const sensor = subdevOpen(sensor_node, 2);
	if (!sensor) {
		LOGE("Failed to open sensor subdev %s", sensor_node);
		return NULL;
	}

	// Flip sensor and bayer format if needed
	// TODO there should be a better way to handle this, this is too ad-hoc here.
	// Probably the sensor itself should report its orientation and support flipping if it's capable.
	// TODO should it expose the layout-flipped bayer MBUS code? Or should it expose capable formats? ...
	{
		V4l2Control *const sensor_rotation = v4l2ControlGet(&sensor->controls, V4L2_CID_CAMERA_SENSOR_ROTATION);
		if (sensor_rotation) {
			LOGI("Sensor rotation=%d", (int)sensor_rotation->value);
			switch (sensor_rotation->value) {
				case 0:
					// TODO check no flip?
					break;

				case 180:
					{
						v4l2ControlSetById(&sensor->controls, V4L2_CID_HFLIP, 1);
						v4l2ControlSetById(&sensor->controls, V4L2_CID_VFLIP, 1);

						/* TODO ...
						V4l2Control *const hflip = v4l2ControlGet(&sensor->controls, V4L2_CID_HFLIP);
						if (!hflip) {
							LOGE("Sensor rotation=%d, but HFLIP is not supported", (int)sensor_rotation->value);
							break;
						}

						if (hflip->value) {
							LOGI("Sensor is already HFLIPped");
							break;
						}

						const int result = v4l2ControlSet(&sensor->controls, hflip, 1);
						if (result != 0) {
							LOGE("Sensor rotation=%d HFLIP=%d failed: %s %d",
								(int)sensor_rotation->value, 1, strerror(result), result);
							break;
						}

						LOGI("Sensor rotation=%d HFLIP=%d modify_layout=%d",
							(int)sensor_rotation->value, 1, !!(hflip->query.flags & V4L2_CTRL_FLAG_MODIFY_LAYOUT));
						*/

						break;
					}

				// TODO case 90: break;
				// TODO case 270: break;
				default:
					LOGE("Unsupported sensor rotation=%d", (int)sensor_rotation->value);
					break;
			}
		}
	}

	// TODO ...
	v4l2ControlSetById(&sensor->controls, V4L2_CID_ANALOGUE_GAIN, 896);

	SubdevSet ss = {
		.pad = 0,

		// TODO where to crop?
		.mbus_code = SENSOR_BUSFMT,
		.width = SENSOR_WIDTH,
		.height = SENSOR_HEIGHT,

		// Lower fps
		//.mbus_code = MEDIA_BUS_FMT_SRGGB12_1X12,
		//.width = 2664,
		//.height = 1980,
	};
	if (0 != subdevSet(sensor, &ss)) {
		LOGE("Failed to set up subdev");
		goto fail;
	}

	// 2. Open camera device
	camera = deviceOpen(camera_node);
	if (!camera) {
		LOGE("Failed to open camera device");
		goto fail;
	}

	if (0 != deviceStreamQueryFormats(&camera->capture, ss.mbus_code)) {
		LOGE("Failed to query camera:capture stream formats");
		goto fail;
	}

	const DeviceStreamPrepareOpts camera_capture_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_EXPORT,

		.pixelformat = CAMERA_PIXFMT,
		.width = SENSOR_WIDTH,
		.height = SENSOR_HEIGHT,
	};

	if (0 != deviceStreamPrepare(&camera->capture, &camera_capture_opts)) {
		LOGE("Unable to prepare camera:capture stream");
		goto fail;
	}

	PiCamera *node = calloc(sizeof(PiCamera), 1);

	node->node.name = "camera";
	node->node.dtorFunc = cameraDtor;
	node->camera = camera;
	node->sensor = sensor;

	node->node.output = &camera->capture;

	return &node->node;

fail:
	if (sensor)
		subdevClose(sensor);

	if (camera)
		deviceClose(camera);

	return NULL;
}

typedef struct {
	Node node;

	Device *capture;
	Device *output;
} PiIsp;

static void ispDtor(Node *node) {
	if (!node)
		return;

	PiIsp *isp = (PiIsp*)node;
	deviceClose(isp->capture);
	deviceClose(isp->output);
	free(isp);
}

struct Node *piOpenISP(void) {
#define DEBAYER_ISP_OUT_DEV "/dev/video13"
#define DEBAYER_ISP_CAP_DEV "/dev/video14"
	Device *isp_out = NULL;
	Device *isp_cap = NULL;

	// 3. Open Bayer to YUV encoder
	isp_out = deviceOpen(DEBAYER_ISP_OUT_DEV);
	if (!isp_out) {
		LOGE("Failed to open isp_out device");
		goto fail;
	}

	if (0 != deviceStreamQueryFormats(&isp_out->output, 0)) {
		LOGE("Failed to query isp_out:output stream formats");
		goto fail;
	}

	// TODO ...
	v4l2ControlSetById(&isp_out->controls, V4L2_CID_RED_BALANCE, 3285);
	v4l2ControlSetById(&isp_out->controls, V4L2_CID_BLUE_BALANCE, 1618);
	v4l2ControlSetById(&isp_out->controls, V4L2_CID_DIGITAL_GAIN, 1000);

	const DeviceStreamPrepareOpts isp_output_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_IMPORT,

		.pixelformat = CAMERA_PIXFMT,
		.width = SENSOR_WIDTH,
		.height = SENSOR_HEIGHT,

		.crop_width = ISP_CROP_WIDTH,
		.crop_height = ISP_CROP_HEIGHT,
	};

	if (0 != deviceStreamPrepare(&isp_out->output, &isp_output_opts)) {
		LOGE("Unable to prepare isp_out:output stream");
		goto fail;
	}

	isp_cap = deviceOpen(DEBAYER_ISP_CAP_DEV);
	if (!isp_cap) {
		LOGE("Failed to open isp_cap device");
		goto fail;
	}

	if (0 != deviceStreamQueryFormats(&isp_cap->capture, 0)) {
		LOGE("Failed to query iso_cap:capture stream formats");
		goto fail;
	}

	const DeviceStreamPrepareOpts isp_capture_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_EXPORT,
		.pixelformat = ISP_OUTPUT_PIXFMT,
		.width = isp_out->output.compose.width,
		.height = isp_out->output.compose.height,
		/* .width = ss.width, */
		/* .height = ss.height, */
	};

	if (0 != deviceStreamPrepare(&isp_cap->capture, &isp_capture_opts)) {
		LOGE("Unable to prepare isp_cap:capture stream");
		goto fail;
	}

	PiIsp *node = (PiIsp*)calloc(sizeof(PiIsp), 1);
	node->node.name = "isp";
	node->node.output = &isp_cap->capture;
	node->node.input = &isp_out->output;
	node->node.dtorFunc = ispDtor;

	node->output = isp_out;
	node->capture = isp_cap;

	return &node->node;

fail:
	if (isp_out)
		deviceClose(isp_out);

	if (isp_cap)
		deviceClose(isp_cap);

	return NULL;
}

typedef struct {
	Node node;

	Device *encoder;
} PiEncoder;

static void encoderDtor(Node *node) {
	if (!node)
		return;

	PiEncoder *encoder = (PiEncoder*)node;
	deviceClose(encoder->encoder);
	free(encoder);
}

static struct Node *piOpenEncoderImpl(const char *name, uint32_t pixfmt, const char* device_node) {

	// 4. Open YUV to MJPEG encoder
	// /dev/video11
	Device *const encoder = deviceOpen(device_node);
	if (!encoder) {
		LOGE("Failed to open %s device", name);
		goto fail;
	}

	if (0 != deviceStreamQueryFormats(&encoder->output, 0)) {
		LOGE("Failed to query %s:output stream formats", name);
		goto fail;
	}

	DeviceStreamPrepareOpts encoder_output_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_IMPORT,
		.pixelformat = ISP_OUTPUT_PIXFMT,
		.width = ISP_CROP_WIDTH,
		.height = ISP_CROP_HEIGHT,
	};

	if (0 != deviceStreamPrepare(&encoder->output, &encoder_output_opts)) {
		LOGE("Unable to prepare %s output stream", name);
		goto fail;
	}

	if (0 != deviceStreamQueryFormats(&encoder->capture, 0)) {
		LOGE("Failed to query %s:capture stream formats", name);
		goto fail;
	}

	const DeviceStreamPrepareOpts encoder_capture_opts = {
		.buffers_count = 3,
		//.buffer_memory = BUFFER_MEMORY_MMAP,
		.buffer_memory = BUFFER_MEMORY_DMABUF_EXPORT,
		.pixelformat = pixfmt,
		.width = encoder_output_opts.width,
		.height = encoder_output_opts.height,
		/* .width = ss.width, */
		/* .height = ss.height, */
	};

	if (0 != deviceStreamPrepare(&encoder->capture, &encoder_capture_opts)) {
		LOGE("Unable to prepare encoder capture stream");
		goto fail;
	}

	PiEncoder *enc = (PiEncoder*)calloc(sizeof(PiEncoder), 1);
	enc->node.name = name;
	enc->node.dtorFunc = encoderDtor;
	enc->node.output = &encoder->capture;
	enc->node.input = &encoder->output;

	enc->encoder = encoder;

	return &enc->node;

fail:
	if (encoder)
		deviceClose(encoder);

	return NULL;
}

struct Node *piOpenEncoder(enum PiEncoderType type) {
#define ENCODER_DEV "/dev/video11"
#define JPEG_ENCODER_DEV "/dev/video31"

	switch (type) {
		case PiEncoderMJPEG:
			return piOpenEncoderImpl("encoder_mjpeg", V4L2_PIX_FMT_MJPEG, ENCODER_DEV);
		case PiEncoderH264:
			return piOpenEncoderImpl("encoder_h264", V4L2_PIX_FMT_H264, ENCODER_DEV);
		case PiEncoderJPEG:
			return piOpenEncoderImpl("encoder_jpeg", V4L2_PIX_FMT_JPEG, JPEG_ENCODER_DEV);
		default:
			LOGE("Invalid encoder type %d", type);
			return NULL;
	}
}
