#include "pump.h"
#include "common.h"

#include <stdlib.h>

static int passDmabufMP(const Buffer *src, Buffer *dst, int planes_count) {
	for (int i = 0; i < planes_count; ++i) {
		dst->buffer.m.planes[i].length = src->buffer.m.planes[i].length;
		dst->buffer.m.planes[i].data_offset = src->buffer.m.planes[i].data_offset;
		dst->buffer.m.planes[i].bytesused = src->buffer.m.planes[i].bytesused;
		dst->buffer.m.planes[i].m.fd = src->dmabuf_fd[i];
	}

	return 0;
}

static int passDmabufSPtoMP(const Buffer *src, Buffer *dst, int planes_count) {
	ASSERT(planes_count == 1);

	dst->buffer.m.planes[0].length = src->buffer.length;
	dst->buffer.m.planes[0].data_offset = 0;
	dst->buffer.m.planes[0].bytesused = src->buffer.bytesused;
	dst->buffer.m.planes[0].m.fd = src->dmabuf_fd[0];

	return 0;
}

static int passDmabufMPtoSP(const Buffer *src, Buffer *dst, int planes_count) {
	ASSERT(planes_count == 1);

	dst->buffer.length = src->buffer.m.planes[0].length;
	// FIXME this is impossible dst->buffer.data_offset = src->buffer.m.planes[0].data_offset;
	// FIXME detect and complain
	dst->buffer.bytesused = src->buffer.m.planes[0].bytesused;
	dst->buffer.m.fd = src->dmabuf_fd[0];

	return 0;
}

static int passDmabufSP(const Buffer *src, Buffer *dst, int planes_count) {
	ASSERT(planes_count == 1);

	dst->buffer.length = src->buffer.length;
	dst->buffer.bytesused = src->buffer.bytesused;
	dst->buffer.m.fd = src->dmabuf_fd[0];

	return 0;
}

static int passMmapToUserptrSP(const Buffer *src, Buffer *dst, int planes_count) {
	ASSERT(planes_count == 1);

	dst->buffer.m.userptr = (unsigned long)src->mapped[0];
	dst->buffer.length = src->buffer.length;

	return 0;
}

static buffer_pass_func *getPassFunc(const DeviceStream *src, const DeviceStream *dst) {
	// FIXME verify plane compatibility
	const int src_planes = STREAM_PLANES_COUNT(src);
	const int dst_planes = STREAM_PLANES_COUNT(src);
	if (src_planes != dst_planes) {
		LOGE("%s: incompatible number of planes: src=%d dst=%d", __func__, src_planes, dst_planes);
		return NULL;
	}

	const int src_mp = !!IS_STREAM_MPLANE(src);
	const int dst_mp = !!IS_STREAM_MPLANE(dst);

	// TODO table with all supported permutations
	if (src->buffer_memory == BUFFER_MEMORY_MMAP && dst->buffer_memory == BUFFER_MEMORY_USERPTR) {
		if (src_mp) {
			LOGE("Pump only supports single-plane mmap to userptr pass");
			return NULL;
		}

		return passMmapToUserptrSP;
	}

	if (src->buffer_memory != BUFFER_MEMORY_DMABUF_EXPORT) {
		LOGE("Pump only supports DMABUF export as a source");
		return NULL;
	}

	if (dst->buffer_memory != BUFFER_MEMORY_DMABUF_IMPORT) {
		LOGE("Pump only supports DMABUF import as a destination");
		return NULL;
	}
	
	buffer_pass_func *const table[] = {
		passDmabufSP,
		passDmabufSPtoMP,
		passDmabufMPtoSP,
		passDmabufMP
	};

	return table[src_mp * 2 + dst_mp];
}

Pump *pumpCreate(DeviceStream *src, DeviceStream *dst) {
	buffer_pass_func *const pass_func = getPassFunc(src, dst);
	if (!pass_func) {
		LOGE("Unable to find a suitable buffer passing func for given src and dst streams");
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
	for (int i = 0; i < pump->dst.st->buffers_count; ++i)
		queuePush(&pump->dst.available, &i);

	pump->buffer_pass_func = pass_func;
	pump->planes_count = STREAM_PLANES_COUNT(src);
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

	/*
	LOGI("FROM BUF");
	v4l2PrintBuffer(&sbuf->buffer);
	LOGI("TO BUF");
	v4l2PrintBuffer(&dbuf->buffer);
	*/

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
