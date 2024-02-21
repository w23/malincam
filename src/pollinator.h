#pragma once

#include "array.h"

#include <stdint.h>

typedef enum {
	POLLINATOR_CONTINUE = 0,
	POLLINATOR_STOP,
} PollinatorAction;

#define POLLIN_FD_READ (1<<0)
#define POLLIN_FD_WRITE (1<<1)
#define POLLIN_FD_ERR (1<<2)

// No pollinator functions are safe to call from within the callback
typedef int (pollin_fd_f)(int fd, uint32_t flags, uintptr_t arg1, uintptr_t arg2);

typedef struct Pollinator {
	Array fds;
	Array fd_arg;
} Pollinator;

void pollinatorInit(Pollinator *p);
void pollinatorFinalize(Pollinator *p);

/*
typedef struct {
	int index_;
} PollinatorHandle;

#define POLLINATOR_IS_INVALID_HANDLE(handle) ((handle).index_ < 0)
*/

int pollinatorRegisterFd(Pollinator *p, int fd, pollin_fd_f *func, uintptr_t arg1, uintptr_t arg2);
// TODO pollinatorUnregister(Pollinator *p, PollinatorHandle handle);

int pollinatorPoll(Pollinator *p, int timeout_ms);
