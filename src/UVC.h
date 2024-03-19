#pragma once

#include "V4l2Control.h"

#include <stdint.h>

struct Node;

typedef int (uvc_event_streamon_f)(int stream_on);

/*
typedef int (uvc_event_ctrl_get_f)(void* arg1, uint32_t ctrl_id, int64_t *out_value);
typedef int (uvc_event_ctrl_set_f)(void* arg1, uint32_t ctrl_id, int64_t value);

typedef struct {
	V4l2Control ctrl;
	void *arg1;
} UvcCtrl;
*/

typedef struct {
	const char *dev_name;

	// TODO
	// - list of all supported modes (format, resolution, frametime)
	// - list of all supported ctrls, with Node reference

	// Array of const V4l2Controls*
	// Moved out
	//Array *controls;

	// TODO more flexible
	struct {
		V4l2Control *brightness;
	} controls;

	uvc_event_streamon_f *event_streamon;

	//uvc_event_ctrl_get_f *event_ctrl_get;
	//uvc_event_ctrl_set_f *event_ctrl_set;

} UvcOpenArgs;

// TODO *create* uvc-gadget device programmatically from a set of known modes, ctrls, etc

struct Node *uvcOpen(UvcOpenArgs args);

int uvcProcessEvents(struct Node *uvc_node);
