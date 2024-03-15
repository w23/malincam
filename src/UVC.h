#pragma once

#include <stdint.h>

struct Node;

typedef int (uvc_event_streamon_f)(int stream_on);

// TODO
//typedef int (uvc_event_ctrl_get_f)(struct Node*, uint32_t ctrl_id, int64_t *out_value);
//typedef int (uvc_event_ctrl_set_f)(struct Node*, uint32_t ctrl_id, int64_t value);

typedef struct {
	const char *dev_name;

	// TODO
	// - list of all supported modes (format, resolution, frametime)
	// - list of all supported ctrls, with Node reference

	uvc_event_streamon_f *event_streamon;

} UvcOpenArgs;

// TODO *create* uvc-gadget device programmatically from a set of known modes, ctrls, etc

struct Node *uvcOpen(UvcOpenArgs args);

int uvcProcessEvents(struct Node *uvc_node);
