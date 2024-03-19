#pragma once
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

#define UNUSED(a) ((void)(a));

#define COUNTOF(a) (sizeof(a)/sizeof(a[0]))

#define ASSERT(...) assert(__VA_ARGS__)

#define LOG(prefix, fmt, ...) \
	fprintf(stderr, prefix fmt "\n", ##__VA_ARGS__)

#define LOGI(fmt, ...) LOG("[INF] ", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG("[ERR] ", fmt, ##__VA_ARGS__)
