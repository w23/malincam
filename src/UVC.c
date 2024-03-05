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

	const DeviceStreamPrepareOpts uvc_output_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_IMPORT,

		.pixelformat = V4L2_PIX_FMT_MJPEG,
		.width = 1332,
		.height = 976,
	};

	if (0 != deviceStreamPrepare(&dev->output, &uvc_output_opts)) {
		LOGE("%s: Unable to prepare uvc-gadget output stream", __func__);
		goto fail;
	}

	UvcGadget *gadget = (UvcGadget*)calloc(1, sizeof(UvcGadget));
	gadget->node.name = "uvc_gadget";
	gadget->node.dtorFunc = uvcDtor;
	gadget->node.output = &dev->output;

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

static int processEventStreaming(UvcGadget *uvc, int request, int control_selector, struct uvc_request_data *response) {
	UNUSED(uvc);
	UNUSED(response);
	LOGE("%s: not implemented (req=%s, cs=%d)", __func__, requestName(request), control_selector);

	switch (request) {
		case UVC_GET_LEN:
			break;
		case UVC_GET_INFO:
			break;

		case UVC_SET_CUR:
			break;
		case UVC_GET_CUR:
			break;

		case UVC_GET_MIN:
		case UVC_GET_DEF:
		case UVC_GET_MAX:
			break;

		case UVC_GET_RES:
			break;
	}

	return 0;
}

static int processEventClass(UvcGadget *uvc, const struct usb_ctrlrequest *ctrl, struct uvc_request_data *response) {
	const int interface = ctrl->wIndex & 0xff;

// TODO these come from:
// /sys/kernel/config/usb_gadget/g1/functions/uvc.0/streaming/bInterfaceNumber
// /sys/kernel/config/usb_gadget/g1/functions/uvc.0/control/bInterfaceNumber
// and shouldn't be hardcoded, probably
#define INTERFACE_CONTROL 0
#define INTERFACE_STREAMING 1

	switch (interface) {
		case INTERFACE_CONTROL:
			LOGE("%s: control interface is not implemented", __func__);
			return 0;
		case INTERFACE_STREAMING:
			{
				const int control_selector = ctrl->wValue >> 8;
				return processEventStreaming(uvc, ctrl->bRequest, control_selector, response);
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
			if (0 != processEventClass(uvc, ctrl, &response)) {
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

static int processEventData(UvcGadget *uvc, const struct uvc_request_data *data) {
	UNUSED(uvc);
	UNUSED(data);
	LOGE("%s: not implemented", __func__);
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
		// TODO start streaming
		break;

	case UVC_EVENT_STREAMOFF:
		LOGI("%s: UVC_EVENT_STREAMOFF", uvc->node.name);
		// TODO stop streaming
		break;

	case UVC_EVENT_SETUP:
		LOGI("%s: UVC_EVENT_SETUP (rtype=%02x, type=%02x, val=%04x, ind=%d, len=%d)", uvc->node.name,
			(int)uvc_event->req.bRequestType,
			(int)uvc_event->req.bRequest,
			(int)uvc_event->req.wValue,
			(int)uvc_event->req.wIndex,
			(int)uvc_event->req.wLength);
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

	for (;;) {
		struct v4l2_event event;
		const int result = deviceEventGet(uvc->gadget, &event);
		if (result == 0)
			break;

		if (result < 0) {
			LOGE("Error getting events for %s", uvc->node.name);
			return result;
		}

		if (0 != processEvent(uvc, &event))
			return -1;
	}

	return 0;
}
