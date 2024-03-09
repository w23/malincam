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

typedef struct UvcGadget {
	Node node;

	Device *gadget;

	struct {
		uint32_t control_selector;
		int is_streaming;
	} state;
} UvcGadget;

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
	
	return &gadget->node;

fail:
	deviceClose(dev);
	return NULL;
}

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

static int processEventSetupStreaming(UvcGadget *uvc, int request, int control_selector, struct uvc_request_data *response) {
	UNUSED(uvc);
	UNUSED(response);
	LOGE("%s(req=%s, cs=%d)", __func__, requestName(request), control_selector);

	struct uvc_streaming_control *const stream_ctrl = (void*)&response->data;

	switch (request) {
		case UVC_GET_LEN:
			// Reply with uvc_streaming_control structure size
			// TODO why? spec ref?
			response->data[0] = 0x00;
			response->data[1] = sizeof(struct uvc_streaming_control);
			response->length = 2;
			break;

		case UVC_GET_INFO:
			// TODO spec ref?
			response->data[0] = UVC_CONTROL_CAP_GET | UVC_CONTROL_CAP_SET;
			response->length = 1;
			break;

		case UVC_SET_CUR:
			LOGE("%s: UVC_SET_CUR not implemented (cs=%d)", __func__, control_selector);
			uvc->state.control_selector = control_selector;
			response->length = sizeof(struct uvc_streaming_control);
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
			response->length = sizeof(struct uvc_streaming_control);
			break;

		case UVC_GET_RES:
			// TODO why?
			memset(stream_ctrl, 0, sizeof(*stream_ctrl));
			response->length = sizeof(struct uvc_streaming_control);
			break;
	}

	return 0;
}

static int processEventSetupClass(UvcGadget *uvc, const struct usb_ctrlrequest *ctrl, struct uvc_request_data *response) {
	const int interface = ctrl->wIndex & 0xff;
	const int control_selector = ctrl->wValue >> 8;

// TODO these come from:
// /sys/kernel/config/usb_gadget/g1/functions/uvc.0/streaming/bInterfaceNumber
// /sys/kernel/config/usb_gadget/g1/functions/uvc.0/control/bInterfaceNumber
// and shouldn't be hardcoded, probably
#define INTERFACE_CONTROL 0
#define INTERFACE_STREAMING 1

	switch (interface) {
		case INTERFACE_CONTROL:
			{
				const int interface = ctrl->wIndex >> 8;
				LOGE("%s: control interface is not implemented (interface=%d, bRequest=%s, control_selector=%d, length=%d)",
					__func__, interface, requestName(ctrl->bRequest), control_selector, ctrl->wLength);
				return 0;
			}

		case INTERFACE_STREAMING:
			{
				return processEventSetupStreaming(uvc, ctrl->bRequest, control_selector, response);
			}
		default:
			LOGE("%s: unexpected interface %d", __func__, interface);
	}

	return 0;
}

static int processEventSetup(UvcGadget *uvc, const struct usb_ctrlrequest *ctrl) {
	struct uvc_request_data response = {0};
	response.length = -EL2HLT; // TODO what is this magic value?

	switch (ctrl->bRequestType & USB_TYPE_MASK) {
		case USB_TYPE_STANDARD:
			// Do nothing
			LOGI("USB_TYPE_STANDARD -- nop");
			break;

		case USB_TYPE_CLASS:
			if (0 != processEventSetupClass(uvc, ctrl, &response)) {
				LOGE("%s: error processing USB_TYPE_CLASS event", __func__);
			}
			break;
	}

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
	LOGE("%s: not implemented (length=%d), control_selector=%d", __func__, data->length, uvc->state.control_selector);

	if (uvc->state.control_selector == UVC_VS_COMMIT_CONTROL) {
		const struct uvc_streaming_control *const ctrl = (const void*)&data->data;
		LOGI("%s: commit bFormatIndex=%d bFrameIndex=%d", __func__, ctrl->bFormatIndex, ctrl->bFrameIndex);
	}

	return 0;
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
		LOGI("%s: UVC_EVENT_SETUP (bRequestType=%02x, bRequest=%02x, wValue=%04x, wIndex=%04x, wLength=%d)",
			uvc->node.name,
			uvc_event->req.bRequestType,
			uvc_event->req.bRequest,
			uvc_event->req.wValue,
			uvc_event->req.wIndex,
			uvc_event->req.wLength);
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
