#pragma once

#include "device.h"
#include "queue.h"

typedef int (buffer_pass_func)(const Buffer *src, Buffer *dst, int planes_count);

typedef struct Pump {
	struct {
		DeviceStream *st;

		int next_in_queue;
		// TODO try multiple: Queue pending;
	} src;

	struct {
		DeviceStream *st;

		// Map of dst:st buffer index to corresponding src:st buffer index
		int *acquired_to_source;

		// Available empty buffer for queueing
		Queue available;
	} dst;

	buffer_pass_func *buffer_pass_func;
	int planes_count;
} Pump;

#define HINT_SOURCE (1<<0)
#define HINT_DEST (1<<1)

Pump *pumpCreate(DeviceStream *src, DeviceStream *dst);
int pumpPump(Pump *pump);// TODO, uint32_t hint);
// TODO pumpDrain()
void pumpDestroy(Pump *pump);
