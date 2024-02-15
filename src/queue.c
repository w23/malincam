#include "queue.h"

#include <memory.h> // memcpy
#include <errno.h>
#include <stddef.h> // NULL

#define QUEUE_AT(q, i) \
	(void*)(((char*)(q)->data.data) + (q)->data.item_size * (i))

#define QUEUE_AT_CONST(q, i) \
	(const void*)(((const char*)(q)->data.data) + (q)->data.item_size * (i))

void queueInit(Queue *queue, int item_size, int max_count) {
	queue->data.size = 0;
	queue->data.capacity = 0;
	queue->data.item_size = item_size;
	queue->data.data = NULL;
	arrayReserve(&queue->data, max_count);

	queue->front = 0;
}

void queueFinalize(Queue *queue) {
	arrayDestroy(&queue->data);
}

// Returns number of items in the queue 
// Returns < 0 on error (i.e. no space)
int queuePush(Queue *queue, const void *item) {
	if (queue->data.size == queue->data.capacity)
		return ENOMEM;

	const int slot = (queue->front + queue->data.size) % queue->data.capacity;
	void *const dst = QUEUE_AT(queue, slot);
	memcpy(dst, item, queue->data.item_size);

	queue->data.size++;
	return queue->data.size;
}

// Get next item in the queue
// Pointer is valid until next queuePush() or queueFinalize()
const void *queuePop(Queue *queue) {
	if (queue->data.size == 0)
		return NULL;

	const void *const item = QUEUE_AT_CONST(queue, queue->front);
	queue->front = (queue->front + 1) % queue->data.capacity;
	return item;
}

const void *queuePeek(const Queue *queue) {
	if (queue->data.size == 0)
		return NULL;

	return QUEUE_AT_CONST(queue, queue->front);
}
