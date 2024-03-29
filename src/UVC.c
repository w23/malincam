#include "UVC.h"

#include "Node.h"
#include "device.h"
#include "common.h"

#include <linux/usb/g_uvc.h>
#include <linux/usb/video.h>
#include <sys/ioctl.h>

#include <stdlib.h> // free
#include <errno.h>
#include <string.h>

// 4.2.1.2 of USB UVC 1.5 spec
#define UVC_REQ_ERROR_NO_ERROR 0x00
#define UVC_REQ_ERROR_NOT_READY 0x01
#define UVC_REQ_ERROR_WRONG_STATE 0x02
#define UVC_REQ_ERROR_POWER 0x03
#define UVC_REQ_ERROR_OUT_OF_RANGE 0x04
#define UVC_REQ_ERROR_INVALID_UNIT 0x05
#define UVC_REQ_ERROR_INVALID_CONTROL 0x06
#define UVC_REQ_ERROR_INVALID_REQUEST 0x07
#define UVC_REQ_ERROR_INVALUD_VALUE 0x08
#define UVC_REQ_ERROR_UNKNOWN 0xFF

// These are hardcoded in uvc-gadget, see drivers/usb/gadget/function/f_uvc.c
// Alternatively, configfs could be parsed instead lol:
// /sys/kernel/config/usb_gadget/g1/functions/uvc.0/control/bInterfaceNumber
// /sys/kernel/config/usb_gadget/g1/functions/uvc.0/streaming/bInterfaceNumber
#define UVC_INTF_VIDEO_CONTROL			0
#define UVC_INTF_VIDEO_STREAMING		1

// Units/terminals structure and IDs are also hardcoded,
// see `uvc_alloc_inst()` function in the same file
#define UVC_VC_ENT_INTERFACE 0
#define UVC_VC_ENT_CAMERA_TERMINAL_ID 1
#define UVC_VC_ENT_PROCESSING_UNIT_ID 2
#define UVC_VC_ENT_OUTPUT_TERMINAL_ID 3

#define UVC_VS_ENT_INTERFACE 0

static const char *requestName(int request) {
	switch (request) {
		case UVC_GET_LEN: return "UVC_GET_LEN";
		case UVC_GET_INFO: return "UVC_GET_INFO";
		case UVC_SET_CUR: return "UVC_SET_CUR";
		case UVC_GET_CUR: return "UVC_GET_CUR";
		case UVC_GET_MIN: return "UVC_GET_MIN";
		case UVC_GET_DEF: return "UVC_GET_DEF";
		case UVC_GET_MAX: return "UVC_GET_MAX";
		case UVC_GET_RES: return "UVC_GET_RES";
	}
	return "UNKNOWN";
}

static const char *usbSpeedName(enum usb_device_speed speed) {
	switch (speed) {
		case USB_SPEED_UNKNOWN: return "USB_SPEED_UNKNOWN";
		case USB_SPEED_LOW: return "USB_SPEED_LOW";
		case USB_SPEED_FULL: return "USB_SPEED_FULL";
		case USB_SPEED_HIGH: return "USB_SPEED_HIGH";
		case USB_SPEED_WIRELESS: return "USB_SPEED_WIRELESS";
		case USB_SPEED_SUPER: return "USB_SPEED_SUPER";
		case USB_SPEED_SUPER_PLUS: return "USB_SPEED_SUPER_PLUS";
	}

	return "UNKNOWN";
}

static const char *usbUvcIntefaceName(int interface) {
	switch (interface) {
		case UVC_INTF_VIDEO_CONTROL: return "UVC_INTF_VIDEO_CONTROL";
		case UVC_INTF_VIDEO_STREAMING: return "UVC_INTF_VIDEO_STREAMING";
		default: return "UNKNOWN";
	}
}

static const char *usbUvcEntityName(int interface, int entity_id) {
	switch(interface) {
		case UVC_INTF_VIDEO_CONTROL:
			switch (entity_id) {
				case UVC_VC_ENT_INTERFACE: return "UVC_VC_ENT_INTERFACE";
				case UVC_VC_ENT_CAMERA_TERMINAL_ID: return "UVC_VC_ENT_CAMERA_TERMINAL_ID";
				case UVC_VC_ENT_PROCESSING_UNIT_ID: return "UVC_VC_ENT_PROCESSING_UNIT_ID";
				case UVC_VC_ENT_OUTPUT_TERMINAL_ID: return "UVC_VC_ENT_OUTPUT_TERMINAL_ID";
				default: return "UNKNOWN";
			}
		case UVC_INTF_VIDEO_STREAMING:
			switch (entity_id) {
				case 0: return "UVC_VS_ENT_INTERFACE";
				default: return "UNKNOWN";
			}
		default: return "UNKNOWN";
	}
}

static const char *usbUvcControlName(int interface, int entity_id, int control_selector) {
	switch(interface) {
		case UVC_INTF_VIDEO_CONTROL:
			switch (entity_id) {
				case UVC_VC_ENT_INTERFACE:
					switch (control_selector) {
						case UVC_VC_CONTROL_UNDEFINED: return "UVC_VC_CONTROL_UNDEFINED";
						case UVC_VC_VIDEO_POWER_MODE_CONTROL: return "UVC_VC_VIDEO_POWER_MODE_CONTROL";
						case UVC_VC_REQUEST_ERROR_CODE_CONTROL: return "UVC_VC_REQUEST_ERROR_CODE_CONTROL";
						default: return "UNKNOWN";
					}
				case UVC_VC_ENT_CAMERA_TERMINAL_ID:
					switch (control_selector) {
						case UVC_CT_CONTROL_UNDEFINED: return "UVC_CT_CONTROL_UNDEFINED";
						case UVC_CT_SCANNING_MODE_CONTROL: return "UVC_CT_SCANNING_MODE_CONTROL";
						case UVC_CT_AE_MODE_CONTROL: return "UVC_CT_AE_MODE_CONTROL";
						case UVC_CT_AE_PRIORITY_CONTROL: return "UVC_CT_AE_PRIORITY_CONTROL";
						case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL: return "UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL";
						case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL: return "UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL";
						case UVC_CT_FOCUS_ABSOLUTE_CONTROL: return "UVC_CT_FOCUS_ABSOLUTE_CONTROL";
						case UVC_CT_FOCUS_RELATIVE_CONTROL: return "UVC_CT_FOCUS_RELATIVE_CONTROL";
						case UVC_CT_FOCUS_AUTO_CONTROL: return "UVC_CT_FOCUS_AUTO_CONTROL";
						case UVC_CT_IRIS_ABSOLUTE_CONTROL: return "UVC_CT_IRIS_ABSOLUTE_CONTROL";
						case UVC_CT_IRIS_RELATIVE_CONTROL: return "UVC_CT_IRIS_RELATIVE_CONTROL";
						case UVC_CT_ZOOM_ABSOLUTE_CONTROL: return "UVC_CT_ZOOM_ABSOLUTE_CONTROL";
						case UVC_CT_ZOOM_RELATIVE_CONTROL: return "UVC_CT_ZOOM_RELATIVE_CONTROL";
						case UVC_CT_PANTILT_ABSOLUTE_CONTROL: return "UVC_CT_PANTILT_ABSOLUTE_CONTROL";
						case UVC_CT_PANTILT_RELATIVE_CONTROL: return "UVC_CT_PANTILT_RELATIVE_CONTROL";
						case UVC_CT_ROLL_ABSOLUTE_CONTROL: return "UVC_CT_ROLL_ABSOLUTE_CONTROL";
						case UVC_CT_ROLL_RELATIVE_CONTROL: return "UVC_CT_ROLL_RELATIVE_CONTROL";
						case UVC_CT_PRIVACY_CONTROL: return "UVC_CT_PRIVACY_CONTROL";
						default: return "UNKNOWN";
					}
				case UVC_VC_ENT_PROCESSING_UNIT_ID:
					switch (control_selector) {
						case UVC_PU_CONTROL_UNDEFINED: return "UVC_PU_CONTROL_UNDEFINED";
						case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL: return "UVC_PU_BACKLIGHT_COMPENSATION_CONTROL";
						case UVC_PU_BRIGHTNESS_CONTROL: return "UVC_PU_BRIGHTNESS_CONTROL";
						case UVC_PU_CONTRAST_CONTROL: return "UVC_PU_CONTRAST_CONTROL";
						case UVC_PU_GAIN_CONTROL: return "UVC_PU_GAIN_CONTROL";
						case UVC_PU_POWER_LINE_FREQUENCY_CONTROL: return "UVC_PU_POWER_LINE_FREQUENCY_CONTROL";
						case UVC_PU_HUE_CONTROL: return "UVC_PU_HUE_CONTROL";
						case UVC_PU_SATURATION_CONTROL: return "UVC_PU_SATURATION_CONTROL";
						case UVC_PU_SHARPNESS_CONTROL: return "UVC_PU_SHARPNESS_CONTROL";
						case UVC_PU_GAMMA_CONTROL: return "UVC_PU_GAMMA_CONTROL";
						case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL: return "UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL";
						case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL: return "UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL";
						case UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL: return "UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL";
						case UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL: return "UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL";
						case UVC_PU_DIGITAL_MULTIPLIER_CONTROL: return "UVC_PU_DIGITAL_MULTIPLIER_CONTROL";
						case UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL: return "UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL";
						case UVC_PU_HUE_AUTO_CONTROL: return "UVC_PU_HUE_AUTO_CONTROL";
						case UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL: return "UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL";
						case UVC_PU_ANALOG_LOCK_STATUS_CONTROL: return "UVC_PU_ANALOG_LOCK_STATUS_CONTROL";
						default: return "UNKNOWN";
					}
				case UVC_VC_ENT_OUTPUT_TERMINAL_ID:
					switch (control_selector) {
						default: return "UNKNOWN";
					}
				default: return "UNKNOWN";
			}
		case UVC_INTF_VIDEO_STREAMING:
			switch (entity_id) {
				case 0:
					switch (control_selector) {
						case UVC_VS_CONTROL_UNDEFINED: return "UVC_VS_CONTROL_UNDEFINED";
						case UVC_VS_PROBE_CONTROL: return "UVC_VS_PROBE_CONTROL";
						case UVC_VS_COMMIT_CONTROL: return "UVC_VS_COMMIT_CONTROL";
						case UVC_VS_STILL_PROBE_CONTROL: return "UVC_VS_STILL_PROBE_CONTROL";
						case UVC_VS_STILL_COMMIT_CONTROL: return "UVC_VS_STILL_COMMIT_CONTROL";
						case UVC_VS_STILL_IMAGE_TRIGGER_CONTROL: return "UVC_VS_STILL_IMAGE_TRIGGER_CONTROL";
						case UVC_VS_STREAM_ERROR_CODE_CONTROL: return "UVC_VS_STREAM_ERROR_CODE_CONTROL";
						case UVC_VS_GENERATE_KEY_FRAME_CONTROL: return "UVC_VS_GENERATE_KEY_FRAME_CONTROL";
						case UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL: return "UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL";
						case UVC_VS_SYNC_DELAY_CONTROL: return "UVC_VS_SYNC_DELAY_CONTROL";
						default: return "UNKNOWN";
					}
				default: return "UNKNOWN";
			}
		default: return "UNKNOWN";
	}
}

static void usbPrintSetupPacket(const char *prefix, const struct usb_ctrlrequest *req) {
	const char transfer_direction = (req->bRequestType & USB_DIR_IN) ? '>' : '<';

	char type = '?';
	switch (req->bRequestType & USB_TYPE_MASK) {
		case USB_TYPE_STANDARD: type = 'S'; break;
		case USB_TYPE_CLASS: type = 'C'; break;
		case USB_TYPE_VENDOR: type = 'V'; break;
	}

	char recipient = '?';
	switch (req->bRequestType & USB_RECIP_MASK) {
		case USB_RECIP_DEVICE: recipient = 'D'; break;
		case USB_RECIP_INTERFACE: recipient = 'I'; break;
		case USB_RECIP_ENDPOINT: recipient = 'E'; break;
		case USB_RECIP_OTHER: recipient = 'O'; break;
		case USB_RECIP_PORT: recipient = 'P'; break;
		case USB_RECIP_RPIPE: recipient = 'R'; break;
	}

	const int if_or_endpoint = req->wIndex & 0xff;
	const int entity_id = req->wIndex >> 8;

	LOGI("%sbRequestType=%c%c%c(%02x) bRequest=%s(%02x) wIndex=[ent=%d if=%d](%04x) wValue=%04x wLength=%d",
		prefix,
		transfer_direction, type, recipient, req->bRequestType,
		requestName(req->bRequest), req->bRequest,
		entity_id, if_or_endpoint, req->wIndex,
		req->wValue,
		req->wLength);
}

typedef union {
	uint32_t tag;
	struct {
		uint32_t interface : 8;
		uint32_t entity_id : 8;
		uint32_t control_selector : 8;
		uint32_t unused_ : 8;
	} c;
} UsbDispatchTag;

#define MAKE_DISPATCH_TAG(interface_, entity, control) \
	{ \
		.c = { \
			.interface = interface_, \
			.entity_id = entity, \
			.control_selector = control, \
			.unused_ = 0, \
		}, \
	}

static UsbDispatchTag makeDispatchTag(u8 interface, u8 entity, u8 control_selector) {
	return (UsbDispatchTag)MAKE_DISPATCH_TAG(interface, entity, control_selector);
}

typedef struct {
	UsbDispatchTag dispatch;
	const struct usb_ctrlrequest *req;
	struct uvc_request_data *response;
} UsbUvcControlDispatchArgs;

static UsbUvcControlDispatchArgs usbUvcControlDispatchArgs(const struct usb_ctrlrequest *req, struct uvc_request_data *response) {
	return (UsbUvcControlDispatchArgs){
		.dispatch = makeDispatchTag(
			 /* interface = */ req->wIndex & 0xff,
			 /* entity_id = */ req->wIndex >> 8,
			 /* control_s = */ req->wValue >> 8
		),
		.req = req,
		.response = response,
	};
}

struct UvcGadget;
struct UsbUvcControl;

// Returns UVC_REQ_*
typedef int (UsbUvcControlGetFunc)(struct UvcGadget *uvc, UsbUvcControlDispatchArgs args);

// Called on UVC_GET_INFO
typedef u8 (UsbUvcControlGetInfoFunc)(struct UvcGadget *uvc, UsbUvcControlDispatchArgs args, u8 info_default);

// Called on data phase of UVC_SET_CUR
typedef int (UsbUvcControlSetDataFunc)(struct UvcGadget *uvc, const struct UsbUvcControl *control, const struct uvc_request_data *data);

typedef struct UsbUvcControl {
	UsbDispatchTag dispatch;

	// UVC_GET_INFO default response
	uint32_t info_caps;

	// UVC_GET_LEN
	uint16_t len;

	UsbUvcControlGetInfoFunc *get_info;
	UsbUvcControlGetFunc *get;
	UsbUvcControlSetDataFunc *set_data;
} UsbUvcControl;


typedef struct {
	const UsbUvcControl *controls;
	int controls_count;
} UsbUvcDispatch;

static const UsbUvcControl *usbUvcDispatchFindControlByTag(const UsbUvcDispatch *dispatch, uint32_t dispatch_tag) {
	for (int i = 0; i < dispatch->controls_count; ++i) {
		const UsbUvcControl *const ctrl = dispatch->controls + i;
		if (ctrl->dispatch.tag == dispatch_tag)
			return ctrl;
	}
	return NULL;
}

typedef struct UvcGadget {
	Node node;

	uvc_event_streamon_f *event_streamon;

	Device *gadget;

	struct {
		struct {
			//UvcVcPuBrigtnessValue value;
			//UvcVcPuBrigtnessMeta meta;
			V4l2Control *v4l2;
		} vc_pu_brightness;
	} controls;

	struct {
		// TODO UsbUvcDispatch dispatch;

		// Last request error code as per 4.2.1.2 of UVC 1.5 spec
		// VC_REQUEST_ERROR_CODE_CONTROL
		uint8_t bRequestErrorCode;

		// Set by SET_CUR
		const UsbUvcControl *data_phase_control;
	} usb;
} UvcGadget;

#define USB_UVC_DISPATCH_NO_CONTROL -1
// Values >= 0 are UVC_REQ_ERROR_*
static int usbUvcDispatchRequest(const UsbUvcDispatch *dispatch, struct UvcGadget *uvc, UsbUvcControlDispatchArgs args) {
	const UsbUvcControl *const ctrl = usbUvcDispatchFindControlByTag(dispatch, args.dispatch.tag);
	if (!ctrl)
		return USB_UVC_DISPATCH_NO_CONTROL;

	switch (args.req->bRequest) {
		case UVC_GET_LEN:
			args.response->data[0] = ctrl->len >> 8;
			args.response->data[1] = ctrl->len & 0xff;
			args.response->length = 2;
			return UVC_REQ_ERROR_NO_ERROR;

		case UVC_GET_INFO:
			{
				const u8 info_caps = ctrl->get_info ? ctrl->get_info(uvc, args, ctrl->info_caps) : ctrl->info_caps;
				args.response->data[0] = info_caps;
				args.response->length = 1;
			}
			return UVC_REQ_ERROR_NO_ERROR;

		case UVC_SET_CUR:
			if (0 == (ctrl->info_caps & UVC_CONTROL_CAP_SET)) {
				LOGE("%s: interface=%d entity=%d control=%d doesn't support SET requests",
					__func__, args.dispatch.c.interface, args.dispatch.c.entity_id, args.dispatch.c.control_selector);
				return UVC_REQ_ERROR_INVALID_REQUEST;
			}

			uvc->usb.data_phase_control = ctrl;
			args.response->length = ctrl->len;
			memset(args.response->data, 0, ctrl->len);
			return 0;

		case UVC_GET_CUR:
		case UVC_GET_MIN:
		case UVC_GET_DEF:
		case UVC_GET_MAX:
		case UVC_GET_RES:
			if (0 == (ctrl->info_caps & UVC_CONTROL_CAP_GET)) {
				LOGE("%s: interface=%d entity=%d control=%d doesn't support GET requests",
					__func__, args.dispatch.c.interface, args.dispatch.c.entity_id, args.dispatch.c.control_selector);
				return UVC_REQ_ERROR_INVALID_REQUEST;
			}
			return ctrl->get(uvc, args);

		default:
			LOGE("%s: interface=%d entity=%d control=%d invalid request=%d",
				__func__, args.dispatch.c.interface, args.dispatch.c.entity_id, args.dispatch.c.control_selector,
				args.req->bRequest);
			return UVC_REQ_ERROR_INVALID_REQUEST;
	}
}

static int uvcHandleVcInterfaceErrorCodeControl(UvcGadget *uvc, UsbUvcControlDispatchArgs args) {
	UNUSED(uvc);
	if (args.req->bRequest == UVC_GET_CUR) {
		args.response->length = 1;
		args.response->data[0] = uvc->usb.bRequestErrorCode;
		return UVC_REQ_ERROR_NO_ERROR;
	}

	LOGE("%s: unexpected request %s(%d)", __func__,
		requestName(args.req->bRequest), args.req->bRequest);
	return UVC_REQ_ERROR_INVALID_REQUEST;
}

static int uvcHandleVsInterfaceProbeCommitControl(UvcGadget *uvc, UsbUvcControlDispatchArgs args) {
	UNUSED(uvc);

	struct uvc_streaming_control *const stream_ctrl = (void*)&args.response->data;
	args.response->length = sizeof(struct uvc_streaming_control);

	switch (args.req->bRequest) {
		// TODO these are not the same when there are multiple resolutions and framerates
		case UVC_GET_CUR:
		case UVC_GET_MIN:
		case UVC_GET_DEF:
		case UVC_GET_MAX:
			*stream_ctrl = (struct uvc_streaming_control) {
				.bmHint = 1, // TODO why?
				.bFormatIndex = 1, // TODO format index [1..N]
				.bFrameIndex = 1, // TODO frame index [1..N]
				.dwFrameInterval = 83333, // TODO not fixed
				//.wKeyFrameRate = // TODO not set?
				//.wPFrameRate = // TODO not set?
				//.wCompQuality = // TODO not set?
				//.wCompWindowSize = // TODO not set?
				//.wDelay = // TODO not set?
				.dwMaxVideoFrameSize = 1332 * 976 * 2, // TODO based on real w, h, format

				// Use 1024, otherwise `No fast enough alt setting for requested bandwidth` will happen in dmesg
				// TODO further reading: https://www.thegoodpenguin.co.uk/blog/multiple-uvc-cameras-on-linux/
				// TODO compute real bandwidth based on max frame size and framerate
				.dwMaxPayloadTransferSize = 1024, // TODO why?
				//.dwMaxPayloadTransferSize = 3072, // TODO why?

				//.dwClockFrequency = // TODO not set?
				.bmFramingInfo = 3, // TODO why?
				.bPreferedVersion = 1, // TODO best format?
				.bMinVersion = 1, // TODO first format
				.bMaxVersion = 1, // TODO last format
			};
			break;

		case UVC_GET_RES:
			// TODO why?
			memset(stream_ctrl, 0, sizeof(*stream_ctrl));
			break;
	}

	return 0;
}

static int uvcVsInterfaceProbeCommit(struct UvcGadget *uvc, const struct UsbUvcControl *control, const struct uvc_request_data *data) {
	UNUSED(uvc);
	const struct uvc_streaming_control *const ctrl = (const void*)&data->data;
	LOGI("%s: %s bFormatIndex=%d bFrameIndex=%d", __func__,
		control->dispatch.c.control_selector == UVC_VS_PROBE_CONTROL ? "probe" : "commit",
		ctrl->bFormatIndex, ctrl->bFrameIndex);
	return 0;
}

typedef struct {
	s16 wBrightness;
} UvcVcPuBrigtnessValue;

static u8 uvcVcPuBrightnessGetInfo(struct UvcGadget *uvc, UsbUvcControlDispatchArgs args, u8 info_default) {
	UNUSED(args);
	UNUSED(info_default);
	return (uvc->controls.vc_pu_brightness.v4l2) 
		? UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET
		: 0;
}

static int uvcVcPuBrightnessGet(UvcGadget *uvc, UsbUvcControlDispatchArgs args) {
	V4l2Control *const v4l2 = uvc->controls.vc_pu_brightness.v4l2;
	if (!v4l2)
		return UVC_REQ_ERROR_INVALID_CONTROL;

	UvcVcPuBrigtnessValue *const value = (void*)args.response->data;
	args.response->length = sizeof(UvcVcPuBrigtnessValue);

	switch (args.req->bRequest) {
		case UVC_GET_CUR:
			value->wBrightness = v4l2->value;
			break;
		case UVC_GET_MIN:
			value->wBrightness = v4l2->query.minimum;
			break;
		case UVC_GET_DEF:
			value->wBrightness = v4l2->query.default_value;
			break;
		case UVC_GET_MAX:
			value->wBrightness = v4l2->query.minimum;
			break;
		case UVC_GET_RES:
			value->wBrightness = 1;
			break;
	}
	
	return 0;
}

static int uvcVcPuBrightnessSet(struct UvcGadget *uvc, const struct UsbUvcControl *control, const struct uvc_request_data *data) {
	UNUSED(control);

	V4l2Control *const v4l2 = uvc->controls.vc_pu_brightness.v4l2;
	ASSERT(v4l2);

	const UvcVcPuBrigtnessValue *const value = (const void*)&data->data;
	LOGI("%s: wBrightness=%d", __func__, value->wBrightness);

	// TODO v4l2ControlSetById(

	return 0;
}

// See 4.2.2.1.2 of USB UVC 1.5 spec
#define UVC_VC_CAM_AE_MODE_MANUAL 0x01
#define UVC_VC_CAM_AE_MODE_AUTO 0x02
#define UVC_VC_CAM_AE_MODE_SHUTTER_PRIORITY 0x04
#define UVC_VC_CAM_AE_MODE_APERTURE_PRIORITY 0x08

typedef struct {
	u8 bAutoExposureMode;
} UvcVcCamAeModeValue;

static int uvcHandleVcCamAeModeGet(UvcGadget *uvc, UsbUvcControlDispatchArgs args) {
	UNUSED(uvc);
	UvcVcCamAeModeValue *const value = (void*)args.response->data;
	args.response->length = sizeof(UvcVcCamAeModeValue);

	switch (args.req->bRequest) {
		case UVC_GET_CUR:
		case UVC_GET_DEF:
		case UVC_GET_RES:
			value->bAutoExposureMode = UVC_VC_CAM_AE_MODE_AUTO;
			break;
		default:
			LOGE("%s: unexpected request=%s (%d)", __func__, requestName(args.req->bRequest), args.req->bRequest);
			return UVC_REQ_ERROR_INVALID_REQUEST;
	}
	
	return 0;
}

static const UsbUvcControl default_dispatch_table[] = {
	{
		.dispatch = MAKE_DISPATCH_TAG(UVC_INTF_VIDEO_CONTROL, UVC_VC_ENT_INTERFACE, UVC_VC_REQUEST_ERROR_CODE_CONTROL),
		.info_caps = UVC_CONTROL_CAP_GET,
		.len = 1,
		.get = uvcHandleVcInterfaceErrorCodeControl,
	},
	{
		.dispatch = MAKE_DISPATCH_TAG(UVC_INTF_VIDEO_CONTROL, UVC_VC_ENT_CAMERA_TERMINAL_ID, UVC_CT_AE_MODE_CONTROL),
		.info_caps = UVC_CONTROL_CAP_GET,
		.len = sizeof(UvcVcCamAeModeValue),
		.get = uvcHandleVcCamAeModeGet,
	},
	{
		.dispatch = MAKE_DISPATCH_TAG(UVC_INTF_VIDEO_CONTROL, UVC_VC_ENT_PROCESSING_UNIT_ID, UVC_PU_BRIGHTNESS_CONTROL),
		.info_caps = 0,
		.get_info = uvcVcPuBrightnessGetInfo,
		.len = sizeof(UvcVcPuBrigtnessValue),
		.get = uvcVcPuBrightnessGet,
		.set_data = uvcVcPuBrightnessSet,
	},
	{
		.dispatch = MAKE_DISPATCH_TAG(UVC_INTF_VIDEO_STREAMING, UVC_VS_ENT_INTERFACE, UVC_VS_PROBE_CONTROL),
		.info_caps = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET,
		.len = sizeof(struct uvc_streaming_control),
		.get = uvcHandleVsInterfaceProbeCommitControl,
		.set_data = uvcVsInterfaceProbeCommit,
	},
	{
		.dispatch = MAKE_DISPATCH_TAG(UVC_INTF_VIDEO_STREAMING, UVC_VS_ENT_INTERFACE, UVC_VS_COMMIT_CONTROL),
		.info_caps = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET,
		.len = sizeof(struct uvc_streaming_control),
		.get = uvcHandleVsInterfaceProbeCommitControl,
		.set_data = uvcVsInterfaceProbeCommit,
	},
};

static const UsbUvcDispatch default_dispatch = {
	.controls = default_dispatch_table,
	.controls_count = COUNTOF(default_dispatch_table),
};

static void uvcDtor(Node *node) {
	if (!node)
		return;

	UvcGadget *uvc = (UvcGadget*)node;
	deviceClose(uvc->gadget);
	free(uvc);
}

static int subscribe(Device *dev) {
	if (0 != deviceEventSubscribe(dev, UVC_EVENT_SETUP)) {
		LOGE("Unable to subscribe to UVC_EVENT_SETUP: %d: %s", errno, strerror(errno));
		return 1;
	}

	if (0 != deviceEventSubscribe(dev, UVC_EVENT_DATA)) {
		LOGE("Unable to subscribe to UVC_EVENT_DATA: %d: %s", errno, strerror(errno));
		return 1;
	}

	if (0 != deviceEventSubscribe(dev, UVC_EVENT_STREAMON)) {
		LOGE("Unable to subscribe to UVC_EVENT_STREAMON: %d: %s", errno, strerror(errno));
		return 1;
	}

	if (0 != deviceEventSubscribe(dev, UVC_EVENT_STREAMOFF)) {
		LOGE("Unable to subscribe to UVC_EVENT_STREAMOFF: %d: %s", errno, strerror(errno));
		return 1;
	}

	return 0;
}

/* TODO
static void mapControls(UvcGadget *gadget, const Array *controls) {
#error impl
}
*/

struct Node *uvcOpen(UvcOpenArgs args) {
	Device *dev = deviceOpen(args.dev_name);
	if (!dev) {
		LOGE("%s: Failed to open device %s", __func__, args.dev_name);
		return NULL;
	}

	if (!(dev->this_device_caps & V4L2_CAP_STREAMING)) {
		LOGE("%s: device %s doesn't support streaming", __func__, args.dev_name);
		goto fail;
	}

	if (!(dev->this_device_caps & V4L2_CAP_VIDEO_OUTPUT)) {
		LOGE("%s: device %s doesn't support output", __func__, args.dev_name);
		goto fail;
	}

	if (0 != subscribe(dev)) {
		LOGE("%s: cannot subscribing device %s to UVC gadget events", __func__, args.dev_name);
		goto fail;
	}

	UvcGadget *gadget = (UvcGadget*)calloc(1, sizeof(UvcGadget));
	gadget->node.name = "uvc_gadget";
	gadget->node.dtorFunc = uvcDtor;
	gadget->node.input = &dev->output;

	gadget->event_streamon = args.event_streamon;

	gadget->gadget = dev;
	// TODO construct from controls gadget->usb.dispatch = uvc_dispatch;

	return &gadget->node;

fail:
	deviceClose(dev);
	return NULL;
}

static int processEventSetup(UvcGadget *uvc, const struct usb_ctrlrequest *req) {
	struct uvc_request_data response = {0};
	uvc->usb.data_phase_control = NULL;

	// TODO what is this magic value? STALL?
	// grepping kernel sources says that any negative value would mean stall,
	// see `uvc_send_response()` in `uvc_v4l2.c`
	response.length = -EL2HLT;

	if ((req->bRequestType & USB_TYPE_MASK) != USB_TYPE_CLASS) {
		LOGE("Unexpected setup packet request type %02x, USB_TYPE_CLASS=%02x expected",
			req->bRequestType & USB_TYPE_MASK, USB_TYPE_CLASS);

		uvc->usb.bRequestErrorCode = UVC_REQ_ERROR_INVALID_REQUEST;
	}

	const UsbUvcControlDispatchArgs args = usbUvcControlDispatchArgs(req, &response);

	LOGI("%s: interface=%s(%d) entity=%s(%d) control=%s(%d)", __func__,
		usbUvcIntefaceName(args.dispatch.c.interface), args.dispatch.c.interface,
		usbUvcEntityName(args.dispatch.c.interface, args.dispatch.c.entity_id), args.dispatch.c.entity_id,
		usbUvcControlName(args.dispatch.c.interface, args.dispatch.c.entity_id, args.dispatch.c.control_selector), args.dispatch.c.control_selector);

	// TODO controls dispatch const int result = usbUvcDispatchRequest(&uvc->usb.dispatch, uvc, args);
	const int result = usbUvcDispatchRequest(&default_dispatch, uvc, args);
	switch (result) {
		case USB_UVC_DISPATCH_NO_CONTROL:
			LOGE("%s: interface=%s(%d) entity=%s(%d) control=%s(%d) not found", __func__,
				usbUvcIntefaceName(args.dispatch.c.interface), args.dispatch.c.interface,
				usbUvcEntityName(args.dispatch.c.interface, args.dispatch.c.entity_id), args.dispatch.c.entity_id,
				usbUvcControlName(args.dispatch.c.interface, args.dispatch.c.entity_id, args.dispatch.c.control_selector), args.dispatch.c.control_selector);
			uvc->usb.bRequestErrorCode = UVC_REQ_ERROR_INVALID_REQUEST;
			response.length = -1; // STALL
			break;
		default:
			uvc->usb.bRequestErrorCode = result;
			if (result != UVC_REQ_ERROR_NO_ERROR) {
				LOGE("%s: interface=%s(%d) entity=%s(%d) control=%s(%d) processing request error=%02x", __func__,
					usbUvcIntefaceName(args.dispatch.c.interface), args.dispatch.c.interface,
					usbUvcEntityName(args.dispatch.c.interface, args.dispatch.c.entity_id), args.dispatch.c.entity_id,
					usbUvcControlName(args.dispatch.c.interface, args.dispatch.c.entity_id, args.dispatch.c.control_selector), args.dispatch.c.control_selector,
					result);
				response.length = -1; // STALL
			}
			break;
	}

	// Setup packet *always* needs a data out phase
	// FIXME does it? Even on UVC_SET_CUR?
	LOGI("%s: sending response length=%d", __func__, response.length);
	if (0 != ioctl(uvc->gadget->fd, UVCIOC_SEND_RESPONSE, &response)) {
		const int err = errno;
		LOGE("%s: failed to UVCIOIC_SEND_RESPONSE: %d: %s", __func__, err, strerror(err));
		return -err;
	}

	return 0;
}

static void uvcPrepare(UvcGadget *uvc) {
	// TODO this is where we'd pick format, set it, recreate buffers, etc etc
	const DeviceStreamPrepareOpts uvc_output_opts = {
		.buffers_count = 3,
		//.buffer_memory = BUFFER_MEMORY_USERPTR,
		//.buffer_memory = BUFFER_MEMORY_MMAP,
		.buffer_memory = BUFFER_MEMORY_DMABUF_IMPORT,

		// TODO real format
		.pixelformat = V4L2_PIX_FMT_MJPEG,
		.width = 1332,
		.height = 976,
	};

	if (0 != deviceStreamPrepare(&uvc->gadget->output, &uvc_output_opts)) {
		LOGE("%s: Unable to prepare uvc-gadget output stream", __func__);
	}
}

static int processEventData(UvcGadget *uvc, const struct uvc_request_data *data) {
	int retval = 0;
	if (!uvc->usb.data_phase_control) {
		LOGE("%s: Missing data phase control, likely unsupported UVC_SET_CUR event (length=%d)", __func__, data->length);
		// This is not a fatal event
		goto exit;
	}

	if (!uvc->usb.data_phase_control->set_data) {
		LOGE("%s: Missing set_data() on control, likely unsupported UVC_SET_CUR event (length=%d)", __func__, data->length);
		// This is not a fatal event
		goto exit;
	}

	retval = uvc->usb.data_phase_control->set_data(uvc, uvc->usb.data_phase_control, data);

exit:
	uvc->usb.data_phase_control = NULL;
	return retval;
}

static int processEvent(UvcGadget *uvc, const struct v4l2_event *event) {
	const struct uvc_event *const uvc_event = (void*)&event->u.data;

	switch (event->type) {
	case UVC_EVENT_CONNECT:
		LOGI("%s: UVC_EVENT_CONNECT with speed=%s", uvc->node.name, usbSpeedName(uvc_event->speed));
		break;

	case UVC_EVENT_DISCONNECT:
		LOGI("%s: UVC_EVENT_DISCONNECT", uvc->node.name);
		break;

	case UVC_EVENT_STREAMON:
		LOGI("%s: UVC_EVENT_STREAMON", uvc->node.name);
		uvcPrepare(uvc);
		uvc->event_streamon(1);
		break;

	case UVC_EVENT_STREAMOFF:
		LOGI("%s: UVC_EVENT_STREAMOFF", uvc->node.name);
		uvc->event_streamon(0);
		break;

	case UVC_EVENT_SETUP:
		usbPrintSetupPacket("UVC_EVENT_SETUP: ", &uvc_event->req);
		return processEventSetup(uvc, &uvc_event->req);

	case UVC_EVENT_DATA:
		LOGI("%s: UVC_EVENT_DATA length=%d", uvc->node.name, uvc_event->data.length);
		return processEventData(uvc, &uvc_event->data);

		default:
			LOGE("%s: Unexpected event %d", uvc->node.name, event->type);
	}

	return 0;
}

int uvcProcessEvents(struct Node *uvc_node) {
	UvcGadget *uvc = (UvcGadget*)uvc_node;

	int events = 0;

	for (;;) {
		struct v4l2_event event;
		const int result = deviceEventGet(uvc->gadget, &event);
		if (result == 0)
			break;

		if (result < 0) {
			LOGE("Error getting events for %s", uvc->node.name);
			return result;
		}

		++events;

		if (0 != processEvent(uvc, &event))
			return -1;
	}

	return events;
}
