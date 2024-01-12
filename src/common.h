#pragma once
#include <stdio.h>

#define LOG(prefix, fmt, ...) \
	fprintf(stderr, prefix fmt "\n", ##__VA_ARGS__)

#define LOGI(fmt, ...) LOG("[INF] ", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG("[ERR] ", fmt, ##__VA_ARGS__)
