#include "pump.h"
#include "common.h"

#include <stdlib.h>

static int passDmabuf(const Buffer *src, Buffer *dst, int planes_count) {
	// TODO support single plane, etc
	ASSERT(IS_TYPE_MPLANE(src->buffer.type));
	ASSERT(IS_TYPE_MPLANE(dst->buffer.type));

	for (int i = 0; i < planes_count; ++i) {
		dst->buffer.m.planes[i].length = dst->buffer.m.planes[i].length;
		dst->buffer.m.planes[i].bytesused = dst->buffer.m.planes[i].bytesused;
		dst->buffer.m.planes[i].m.fd = dst->buffer.m.planes[i].m.fd;
	}

	return 0;
}

Pump *pumpCreate(DeviceStream *src, DeviceStream *dst) {
	if (src->buffer_memory != BUFFER_MEMORY_DMABUF_EXPORT) {
		LOGE("Pump only supports DMABUF export as a source");
		return NULL;
	}

	if (dst->buffer_memory != BUFFER_MEMORY_DMABUF_IMPORT) {
		LOGE("Pump only supports DMABUF import as a destination");
		return NULL;
	}

	// FIXME verify plane compatibility
	if (!IS_TYPE_MPLANE(src->type) || !IS_TYPE_MPLANE(dst->type)) {
		LOGE("For now both src and dst should be MPLANE");
		return NULL;
	}

	if (src->format.fmt.pix_mp.num_planes != dst->format.fmt.pix_mp.num_planes) {
		LOGE("Incompatible number of planes: src=%d dst=%d",
			src->format.fmt.pix_mp.num_planes, dst->format.fmt.pix_mp.num_planes);
		return NULL;
	}

	Pump *const pump = malloc(sizeof(*pump));
	pump->src.st = src;
	pump->dst.st = dst;

	pump->src.next_in_queue = -1;
	pump->dst.acquired_to_source = malloc(sizeof(int) * pump->dst.st->buffers_count);
	for (int i = 0; i < pump->dst.st->buffers_count; ++i) 
		pump->dst.acquired_to_source[i] = -1;

	queueInit(&pump->dst.available, sizeof(int), pump->dst.st->buffers_count);

	pump->buffer_pass_func = &passDmabuf;
	pump->planes_count = src->format.fmt.pix_mp.num_planes;
	return pump;
}

void pumpDestroy(Pump *pump) {
	if (!pump)
		return;

	// TODO verify drained

	queueFinalize(&pump->dst.available);
	free(pump->dst.acquired_to_source);
	free(pump);
}

int pumpPump(Pump *pump /* TODO, uint32_t hint*/) {
	// 1. Pull any encoded frames from the destination
	for (;;) {
		const Buffer *const buf = deviceStreamPullBuffer(pump->dst.st);
		if (!buf)
			break;

		// Mark the corresponding source buffer as complete
		const int source_index = pump->dst.acquired_to_source[buf->buffer.index];
		ASSERT(source_index >= 0);
		const int result = deviceStreamPushBuffer(pump->src.st, pump->src.st->buffers + source_index);
		pump->dst.acquired_to_source[buf->buffer.index] = -1;

		if (result != 0) {
			LOGE("Unable to return source buffer[%d] back", source_index);
			return result;
		}
	}

	// 2. Pull any new buffers from the source
	for (;;) {
		const Buffer *const buf = deviceStreamPullBuffer(pump->src.st);
		if (!buf)
			break;

		if (pump->src.next_in_queue >= 0) {
			LOGI("Skipping buffer[%d]", pump->src.next_in_queue);
			const int result = deviceStreamPushBuffer(pump->src.st, pump->src.st->buffers + pump->src.next_in_queue);
			if (result != 0) {
				LOGE("Unable to return source buffer[%d] back", pump->src.next_in_queue);
				return result;
			}
		}

		// Replace
		pump->src.next_in_queue = buf->buffer.index;
	}

	// 3. Pass source buffers to destination
	if (queueGetSize(&pump->dst.available) <= 0 || pump->src.next_in_queue < 0) {
		// Nothing to pass
		return 0;
	}

	const Buffer *const sbuf = pump->src.st->buffers + pump->src.next_in_queue;
	const int dst_index = *(int*)queuePeek(&pump->dst.available);
	Buffer *const dbuf = pump->dst.st->buffers + dst_index;
	int result = pump->buffer_pass_func(sbuf, dbuf, pump->planes_count);
	if (result != 0) {
		LOGE("Unable to pass source to destination buffer");
		return result;
	}

	result = deviceStreamPushBuffer(pump->dst.st, dbuf);
	if (result != 0) {
		LOGE("Unable to pass buffer to dst");
		return result;
	}

	ASSERT(pump->dst.acquired_to_source[dst_index] == -1);
	pump->dst.acquired_to_source[dst_index] = pump->src.next_in_queue;
	pump->src.next_in_queue = -1;
	queuePop(&pump->dst.available);
	return 0;
}
