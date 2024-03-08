#pragma once
#include <stdio.h>
#include <assert.h>

#define UNUSED(a) ((void)(a));

#define COUNTOF(a) (sizeof(a)/sizeof(a[0]))

#define ASSERT(...) assert(__VA_ARGS__)

#define LOG(prefix, fmt, ...) \
	fprintf(stderr, prefix fmt "\n", ##__VA_ARGS__)

#define LOGI(fmt, ...) LOG("[INF] ", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG("[ERR] ", fmt, ##__VA_ARGS__)
