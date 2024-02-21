#pragma once

// TODO struct Stream;
struct DeviceStream;
struct Node;

typedef void (NodeDtorFunc)(struct Node*);

/* TODO ?
enum NodeStart {
	NodeStart,
	NodeStop,
};
typedef int (NodeStartFunc)(struct Node*, enum NodeStart start);
*/

typedef struct Node {
	const char *name;

	NodeDtorFunc *dtorFunc;
	//NodeStartFunc *startFunc;

	// Input: stream that consumes buffers. V4L2 OUTPUT
	struct DeviceStream *input;

	// Output: stream that produces buffers. V4L2 CAPTURE
	struct DeviceStream *output;
} Node;

/* TODO
enum StreamType {
	STREAM_TYPE_INPUT,
	STREAM_TYPE_OUTPUT,
};

typedef struct Stream {
	const char *name;
	enum StreamType type;
	int is_active;

} Stream;
*/

int nodeStart(Node *node);

// TODO drain sequence?
int nodeStop(Node *node);

static inline void nodeDestroy(Node *node) {
	node->dtorFunc(node);
}
