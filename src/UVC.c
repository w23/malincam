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

typedef struct {
	int interface;
	int entity_id;
	int control_selector;
	const struct usb_ctrlrequest *req;
	struct uvc_request_data *response;
} UsbUvcControlDispatchArgs;

UsbUvcControlDispatchArgs usbUvcControlDispatchArgs(const struct usb_ctrlrequest *req, struct uvc_request_data *response) {
	return (UsbUvcControlDispatchArgs){
		.interface = req->wIndex & 0xff,
		.entity_id = req->wIndex >> 8,
		.control_selector = req->wValue >> 8,
		.req = req,
		.response = response,
	};
}

struct UvcGadget;

// Returns UVC_REQ_*
typedef int (UsbUvcControlHandleFunc)(struct UvcGadget *uvc, UsbUvcControlDispatchArgs args);

typedef struct {
	int control_selector;

	// UVC_GET_INFO response
	// TODO dynamic state bits
	uint32_t info_caps;

	// UVC_GET_LEN
	uint16_t len;

	UsbUvcControlHandleFunc *handle;
/* TODO?
	UsbUvcControlHandleFunc *set_cur;
	UsbUvcControlHandleFunc *get_cur;
	UsbUvcControlHandleFunc *get_min;
	UsbUvcControlHandleFunc *get_max;
	UsbUvcControlHandleFunc *get_res;
	UsbUvcControlHandleFunc *get_def;
*/
} UsbUvcControl;

typedef struct {
	int entity_id;

	int controls_count;
	const UsbUvcControl *controls;
} UsbUvcEntity;

typedef struct {
	int interface;

	int entities_count;
	const UsbUvcEntity *entities;
} UsbUvcInterface;

typedef struct {
	const UsbUvcInterface *interfaces;
	int interfaces_count;
} UsbUvcDispatch;

static const UsbUvcInterface *usbUvcDispatchFindInterface(const UsbUvcDispatch *dispatch, int interface) {
	for (int i = 0; i < dispatch->interfaces_count; ++i) {
		const UsbUvcInterface *const intf = dispatch->interfaces + i;
		if (intf->interface == interface)
			return intf;
	}

	return NULL;
}

static const UsbUvcEntity *usbUvcDispatchFindEntity(const UsbUvcInterface *intf, int entity_id) {
	for (int j = 0; j < intf->entities_count; ++j) {
		const UsbUvcEntity *const ent = intf->entities + j;
		if (ent->entity_id == entity_id)
			return ent;
	}

	return NULL;
}

static const UsbUvcControl *usbUvcDispatchFindControl(const UsbUvcEntity *ent, int control_selector) {
	for (int k = 0; k < ent->controls_count; ++k) {
		const UsbUvcControl *const ctrl = ent->controls + k;
		if (ctrl->control_selector == control_selector)
			return ctrl;
	}

	return NULL;
}

#define USB_UVC_DISPATCH_NO_INTERFACE -1
#define USB_UVC_DISPATCH_NO_ENTITY -2
#define USB_UVC_DISPATCH_NO_CONTROL -3
// Values >= 0 are UVC_REQ_ERROR_*
static int usbUvcDispatchRequest(const UsbUvcDispatch *dispatch, struct UvcGadget *uvc, UsbUvcControlDispatchArgs args) {

	const UsbUvcInterface *const intf = usbUvcDispatchFindInterface(dispatch, args.interface);
	if (!intf)
		return USB_UVC_DISPATCH_NO_INTERFACE;

	const UsbUvcEntity *const ent = usbUvcDispatchFindEntity(intf, args.entity_id);
	if (!ent)
		return USB_UVC_DISPATCH_NO_ENTITY;

	const UsbUvcControl *const ctrl = usbUvcDispatchFindControl(ent, args.control_selector);
	if (!ctrl)
		return USB_UVC_DISPATCH_NO_CONTROL;

	switch (args.req->bRequest) {
		case UVC_GET_LEN:
			args.response->data[0] = ctrl->len >> 8;
			args.response->data[1] = ctrl->len & 0xff;
			args.response->length = 2;
			return UVC_REQ_ERROR_NO_ERROR;

		case UVC_GET_INFO:
			args.response->data[0] = ctrl->info_caps;
			args.response->length = 1;
			return UVC_REQ_ERROR_NO_ERROR;

		case UVC_SET_CUR:
			if (0 == (ctrl->info_caps & UVC_CONTROL_CAP_SET)) {
				LOGE("%s: interface=%d entity=%d control=%d doesn't support SET requests",
					__func__, args.interface, args.entity_id, args.control_selector);
				return UVC_REQ_ERROR_INVALID_REQUEST;
			}
			return ctrl->handle(uvc, args);

		case UVC_GET_CUR:
		case UVC_GET_MIN:
		case UVC_GET_DEF:
		case UVC_GET_MAX:
		case UVC_GET_RES:
			if (0 == (ctrl->info_caps & UVC_CONTROL_CAP_GET)) {
				LOGE("%s: interface=%d entity=%d control=%d doesn't support GET requests",
					__func__, args.interface, args.entity_id, args.control_selector);
				return UVC_REQ_ERROR_INVALID_REQUEST;
			}
			return ctrl->handle(uvc, args);

		default:
			LOGE("%s: interface=%d entity=%d control=%d invalid request=%d",
				__func__, args.interface, args.entity_id, args.control_selector,
				args.req->bRequest);
			return UVC_REQ_ERROR_INVALID_REQUEST;
	}
}

typedef struct UvcGadget {
	Node node;

	Device *gadget;

	struct {
		int is_streaming;
	} state;

	struct {
		UsbUvcDispatch dispatch;

		// Last request error code as per 4.2.1.2 of UVC 1.5 spec
		// VC_REQUEST_ERROR_CODE_CONTROL
		uint8_t bRequestErrorCode;

		// Set by VS / INTERFACE / PROBE|COMMIT / UVC_SET_CUR
		uint32_t control_selector;
	} usb;
} UvcGadget;

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

static const UsbUvcControl uvc_vc_interface_controls[] = {
	{
		.control_selector = UVC_VC_REQUEST_ERROR_CODE_CONTROL,
		.info_caps = UVC_CONTROL_CAP_GET,
		.len = 1,
		.handle = uvcHandleVcInterfaceErrorCodeControl,
	},
};

static const UsbUvcEntity uvc_vc_entities[] = {
	{
		.entity_id = UVC_VC_ENT_INTERFACE,
		.controls = uvc_vc_interface_controls,
		.controls_count = COUNTOF(uvc_vc_interface_controls),
	},
};

static int uvcHandleVsInterfaceProbeCommitControl(UvcGadget *uvc, UsbUvcControlDispatchArgs args) {
	UNUSED(uvc);

	struct uvc_streaming_control *const stream_ctrl = (void*)&args.response->data;

	switch (args.req->bRequest) {
		case UVC_SET_CUR:
			LOGE("%s: UVC_SET_CUR not implemented (cs=%d)", __func__, args.control_selector);
			uvc->usb.control_selector = args.control_selector;
			args.response->length = sizeof(struct uvc_streaming_control);
			break;

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
			args.response->length = sizeof(struct uvc_streaming_control);
			break;

		case UVC_GET_RES:
			// TODO why?
			memset(stream_ctrl, 0, sizeof(*stream_ctrl));
			args.response->length = sizeof(struct uvc_streaming_control);
			break;
	}

	return 0;
}

static const UsbUvcControl uvc_vs_interface_controls[] = {
	{
		.control_selector = UVC_VS_PROBE_CONTROL,
		.info_caps = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET,
		.len = sizeof(struct uvc_streaming_control),
		.handle = uvcHandleVsInterfaceProbeCommitControl,
	},
	{
		.control_selector = UVC_VS_COMMIT_CONTROL,
		.info_caps = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET,
		.len = sizeof(struct uvc_streaming_control),
		.handle = uvcHandleVsInterfaceProbeCommitControl,
	},
};

static const UsbUvcEntity uvc_vs_entities[] = {
	{
		.entity_id = UVC_VS_ENT_INTERFACE,
		.controls = uvc_vs_interface_controls,
		.controls_count = COUNTOF(uvc_vs_interface_controls),
	},
};

static const UsbUvcInterface uvc_interfaces[] = {
	{
		.interface = UVC_INTF_VIDEO_CONTROL,
		.entities = uvc_vc_entities,
		.entities_count = COUNTOF(uvc_vc_entities),
	},
	{
		.interface = UVC_INTF_VIDEO_STREAMING,
		.entities = uvc_vs_entities,
		.entities_count = COUNTOF(uvc_vs_entities),
	},
};

static const UsbUvcDispatch uvc_dispatch = {
	.interfaces = uvc_interfaces,
	.interfaces_count = COUNTOF(uvc_interfaces),
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

struct Node *uvcOpen(const char *dev_name) {
	Device *dev = deviceOpen(dev_name);
	if (!dev) {
		LOGE("%s: Failed to open device %s", __func__, dev_name);
		return NULL;
	}

	if (!(dev->this_device_caps & V4L2_CAP_STREAMING)) {
		LOGE("%s: device %s doesn't support streaming", __func__, dev_name);
		goto fail;
	}

	if (!(dev->this_device_caps & V4L2_CAP_VIDEO_OUTPUT)) {
		LOGE("%s: device %s doesn't support output", __func__, dev_name);
		goto fail;
	}

	if (0 != subscribe(dev)) {
		LOGE("%s: cannot subscribing device %s to UVC gadget events", __func__, dev_name);
		goto fail;
	}

	UvcGadget *gadget = (UvcGadget*)calloc(1, sizeof(UvcGadget));
	gadget->node.name = "uvc_gadget";
	gadget->node.dtorFunc = uvcDtor;
	gadget->node.input = &dev->output;

	gadget->gadget = dev;
	gadget->usb.dispatch = uvc_dispatch;

	return &gadget->node;

fail:
	deviceClose(dev);
	return NULL;
}

static int processEventSetup(UvcGadget *uvc, const struct usb_ctrlrequest *req) {
	struct uvc_request_data response = {0};

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
		usbUvcIntefaceName(args.interface), args.interface,
		usbUvcEntityName(args.interface, args.entity_id), args.entity_id,
		usbUvcControlName(args.interface, args.entity_id, args.control_selector), args.control_selector);

	const int result = usbUvcDispatchRequest(&uvc->usb.dispatch, uvc, args);
	switch (result) {
		case USB_UVC_DISPATCH_NO_INTERFACE:
			LOGE("%s: interface=%s(%d) not found", __func__,
				usbUvcIntefaceName(args.interface), args.interface);
			uvc->usb.bRequestErrorCode = UVC_REQ_ERROR_INVALID_REQUEST;
			response.length = -1; // STALL
			break;
		case USB_UVC_DISPATCH_NO_ENTITY:
			LOGE("%s: interface=%s(%d) entity=%s(%d) not found", __func__,
				usbUvcIntefaceName(args.interface), args.interface,
				usbUvcEntityName(args.interface, args.entity_id), args.entity_id);
			uvc->usb.bRequestErrorCode = UVC_REQ_ERROR_INVALID_REQUEST;
			response.length = -1; // STALL
			break;
		case USB_UVC_DISPATCH_NO_CONTROL:
			LOGE("%s: interface=%s(%d) entity=%s(%d) control=%s(%d) not found", __func__,
				usbUvcIntefaceName(args.interface), args.interface,
				usbUvcEntityName(args.interface, args.entity_id), args.entity_id,
				usbUvcControlName(args.interface, args.entity_id, args.control_selector), args.control_selector);
			uvc->usb.bRequestErrorCode = UVC_REQ_ERROR_INVALID_REQUEST;
			response.length = -1; // STALL
			break;
		default:
			uvc->usb.bRequestErrorCode = result;
			if (result != UVC_REQ_ERROR_NO_ERROR) {
				LOGE("%s: interface=%s(%d) entity=%s(%d) control=%s(%d) processing request error=%02x", __func__,
					usbUvcIntefaceName(args.interface), args.interface,
					usbUvcEntityName(args.interface, args.entity_id), args.entity_id,
					usbUvcControlName(args.interface, args.entity_id, args.control_selector), args.control_selector,
					result);
				response.length = -1; // STALL
			}
			break;
	}

	// Setup packet *always* needs a data out phase
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

		.pixelformat = V4L2_PIX_FMT_MJPEG,
		.width = 1332,
		.height = 976,
	};

	if (0 != deviceStreamPrepare(&uvc->gadget->output, &uvc_output_opts)) {
		LOGE("%s: Unable to prepare uvc-gadget output stream", __func__);
	}
}

static int processEventData(UvcGadget *uvc, const struct uvc_request_data *data) {
	UNUSED(uvc);
	UNUSED(data);
	LOGE("%s: not implemented (length=%d), control_selector=%d", __func__, data->length, uvc->usb.control_selector);

	if (uvc->usb.control_selector == UVC_VS_COMMIT_CONTROL) {
		const struct uvc_streaming_control *const ctrl = (const void*)&data->data;
		LOGI("%s: commit bFormatIndex=%d bFrameIndex=%d", __func__, ctrl->bFormatIndex, ctrl->bFrameIndex);
	}

	return 0;
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
		uvc->state.is_streaming = 1;
		// TODO start streaming
		break;

	case UVC_EVENT_STREAMOFF:
		LOGI("%s: UVC_EVENT_STREAMOFF", uvc->node.name);
		// TODO stop streaming
		uvc->state.is_streaming = 0;
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

int uvcIsStreaming(struct Node *uvc_node) {
	UvcGadget *uvc = (UvcGadget*)uvc_node;
	return uvc->state.is_streaming;
}
