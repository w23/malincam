#pragma once

#include "array.h"

// FIFO queue, implemented as a ring buffer
typedef struct Queue {
	Array data;
	int front;
} Queue;

void queueInit(Queue *queue, int item_size, int max_count);
void queueFinalize(Queue *queue);

// Get number of items in the queue
static inline int queueGetSize(const Queue *queue) { return queue->data.size; }

// Get number of free slots in the queue
static inline int queueGetFree(const Queue *queue) {
	return queue->data.capacity - queue->data.size;
}

// Returns number of items in the queue 
// Returns < 0 on error (i.e. no space)
int queuePush(Queue *queue, const void *item);

// Get next item in the queue
// Pointer is valid until next queuePush() or queueFinalize()
const void *queuePop(Queue *queue);
const void *queuePeek(const Queue *queue);
