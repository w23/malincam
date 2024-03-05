#pragma once

struct Node;

// TODO *create* uvc-gadget device programmatically from a set of known modes, ctrls, etc

struct Node *uvcOpen(const char *dev_name);

int uvcProcessEvents(struct Node *uvc_node);
