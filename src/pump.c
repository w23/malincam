#include "pump.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>

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
	dst->buffer.bytesused = src->buffer.bytesused;

	return 0;
}

static int passMmapMPToUserptrSP(const Buffer *src, Buffer *dst, int planes_count) {
	ASSERT(planes_count == 1);

	dst->buffer.length = src->buffer.m.planes[0].length;
	// FIXME this is impossible dst->buffer.data_offset = src->buffer.m.planes[0].data_offset;
	// FIXME detect and complain
	dst->buffer.bytesused = src->buffer.m.planes[0].bytesused;
	dst->buffer.m.userptr = (unsigned long)src->mapped[0];

	return 0;
}

static int passMmapMPToMmapSP(const Buffer *src, Buffer *dst, int planes_count) {
	ASSERT(planes_count == 1);

	// FIXME this is impossible dst->buffer.data_offset = src->buffer.m.planes[0].data_offset;
	// FIXME detect and complain
	dst->buffer.bytesused = src->buffer.m.planes[0].bytesused;

	if (dst->buffer.length < dst->buffer.bytesused) {
		LOGE("%s: buffer size mismatch, src:", __func__);
		v4l2PrintBuffer(&src->buffer);
		LOGE("dst:");
		v4l2PrintBuffer(&dst->buffer);
	}
	ASSERT(dst->buffer.length >= dst->buffer.bytesused);

	memcpy(dst->mapped[0], src->mapped[0], dst->buffer.bytesused);

	return 0;
}

typedef struct {
	buffer_memory_e src_mem, dst_mem;
	int src_mp, dst_mp;
	buffer_pass_func *func;
} PumpBufferPassTableEntry;

static const PumpBufferPassTableEntry pass_func_table[] = {
	{
		.src_mem = BUFFER_MEMORY_DMABUF_EXPORT,
		.dst_mem = BUFFER_MEMORY_DMABUF_IMPORT,
		.src_mp = 1,
		.dst_mp = 1,
		.func = passDmabufMP,
	},
	{
		.src_mem = BUFFER_MEMORY_DMABUF_EXPORT,
		.dst_mem = BUFFER_MEMORY_DMABUF_IMPORT,
		.src_mp = 0,
		.dst_mp = 0,
		.func = passDmabufSP,
	},
	{
		.src_mem = BUFFER_MEMORY_DMABUF_EXPORT,
		.dst_mem = BUFFER_MEMORY_DMABUF_IMPORT,
		.src_mp = 1,
		.dst_mp = 0,
		.func = passDmabufMPtoSP,
	},
	{
		.src_mem = BUFFER_MEMORY_DMABUF_EXPORT,
		.dst_mem = BUFFER_MEMORY_DMABUF_IMPORT,
		.src_mp = 0,
		.dst_mp = 1,
		.func = passDmabufSPtoMP,
	},
	{
		.src_mem = BUFFER_MEMORY_MMAP,
		.dst_mem = BUFFER_MEMORY_USERPTR,
		.src_mp = 0,
		.dst_mp = 0,
		.func = passMmapToUserptrSP,
	},
	{
		.src_mem = BUFFER_MEMORY_MMAP,
		.dst_mem = BUFFER_MEMORY_USERPTR,
		.src_mp = 1,
		.dst_mp = 0,
		.func = passMmapMPToUserptrSP
	},
	{
		.src_mem = BUFFER_MEMORY_MMAP,
		.dst_mem = BUFFER_MEMORY_MMAP,
		.src_mp = 1,
		.dst_mp = 0,
		.func = passMmapMPToMmapSP
	},
};

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

	for (int i = 0; i < (int)COUNTOF(pass_func_table); ++i) {
		const PumpBufferPassTableEntry *const te = pass_func_table + i;
		if (src->buffer_memory != te->src_mem)
			continue;
		if (dst->buffer_memory != te->dst_mem)
			continue;
		if (src_mp != te->src_mp)
			continue;
		if (dst_mp != te->dst_mp)
			continue;

		return te->func;
	}

	LOGE("Pump doesn't support passing mem=%d:%s to mem=%d:%s",
		src->buffer_memory,
		src_mp ? "MP" : "SP",
		dst->buffer_memory,
		dst_mp ? "MP" : "SP");

	return NULL;
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

		/*
		LOGI("Pulled buffer from fd=%d ptr=%p (next=%d):", pump->src.st->dev_fd, (void*)buf, pump->src.next_in_queue);
		v4l2PrintBuffer(&buf->buffer);
		*/

		if (pump->src.next_in_queue >= 0) {
			LOGI("Skipping buffer[%d]", pump->src.next_in_queue);
			//v4l2PrintBuffer(&buf->buffer);
			const int result = deviceStreamPushBuffer(pump->src.st, pump->src.st->buffers + pump->src.next_in_queue);
			//v4l2PrintBuffer(&buf->buffer);
			if (result != 0) {
				LOGE("Unable to return source buffer[%d] back", pump->src.next_in_queue);
				return result;
			}
		}

		// Replace
		pump->src.next_in_queue = buf->buffer.index;

		/*
		{
			const Buffer *const sbuf = pump->src.st->buffers + pump->src.next_in_queue;
			LOGI("next_in_queue=%d from fd=%d ptr=%p:", pump->src.next_in_queue, pump->src.st->dev_fd, (void*)sbuf);
			v4l2PrintBuffer(&sbuf->buffer);
		}
		*/
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
