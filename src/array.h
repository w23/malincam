#pragma once

/* TODO
#define ArrayT(T) struct { \
	T *data; \
	int size, capacity; \
}
*/

typedef struct Array {
	void *data;
	int size, capacity;
	int item_size;
} Array;

#define arrayInit(array, T) do { \
	*(array) = (Array){.data = NULL, .size = 0, .capacity = 0, .item_size = sizeof(T)}; \
} while (0)

static inline void arrayResize(Array *array) { array->size = 0; }
void arrayReserve(Array *array, int capacity);
void arrayAppend(Array *array, const void *item);
void arrayDestroy(Array *array);

// TODO void arrayFreeDtor(Array *array, dtor_f, void *user);

#define arrayAt(arr, T, index) (((T*)((arr)->data)) + index)
#define arrayAtConst(arr, T, index) (((const T*)((arr)->data)) + index)

#ifdef ARRAY_H_IMPLEMENT

#include <stdlib.h>
#include <memory.h>

void arrayReserve(Array *array, int capacity) {
	if (capacity <= array->capacity)
		return;

	void *new_data = malloc(array->item_size * capacity);
	if (array->data) {
		memcpy(new_data, array->data, array->item_size * array->size);
		free(array->data);
	}

	array->data = new_data;
	array->capacity = capacity;
}

void arrayAppend(Array *array, const void *item) {
	if (array->size == array->capacity) {
		int new_capacity = array->capacity ? array->capacity * 3 / 2 : (16 + array->item_size- 1) / array->item_size;
		arrayReserve(array, new_capacity > array->capacity ? new_capacity : array->capacity + 1);
	}

	void *dst = (char*)array->data + array->item_size * array->size;
	memcpy(dst, item, array->item_size);
	array->size++;
}

void arrayDestroy(Array *array) {
	if (array->data)
		free(array->data);
	array->size = array->capacity = 0;
}

#endif // ifdef ARRAY_H_IMPLEMENT
