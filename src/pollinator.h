#pragma once

#include "array.h"

#include <stdint.h>

typedef enum {
	POLLINATOR_CONTINUE = 0,
	POLLINATOR_STOP,
} PollinatorAction;

#define POLLIN_FD_READ (1<<0)
#define POLLIN_FD_WRITE (1<<1)
#define POLLIN_FD_EXCEPT (1<<2)
#define POLLIN_FD_ERR (1<<3)

// No pollinator functions are safe to call from within the callback
typedef int (pollin_fd_f)(int fd, uint32_t flags, uintptr_t arg1, uintptr_t arg2);

struct Pollinator;

struct Pollinator *pollinatorCreate(void);
void pollinatorDestroy(struct Pollinator *p);

typedef struct {
	int fd;
	uint32_t event_bits; // POLLIN_FD_*, zero means unregister
	pollin_fd_f *func;
	uintptr_t arg1, arg2;
} PollinatorMonitorFd;

// Returns < 0 on failure
int pollinatorMonitorFd(struct Pollinator *p, const PollinatorMonitorFd *reg);

int pollinatorPoll(struct Pollinator *p, int timeout_ms);
