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
typedef int (pollin_fd_f)(void *userptr, int fd, uint32_t flags);

typedef struct Pollinator {
	Array fds;
	Array fd_arg;
} Pollinator;

void pollinatorInit(Pollinator *p);
void pollinatorFinalize(Pollinator *p);

int pollinatorRegisterFd(Pollinator *p, int fd, void *userptr, pollin_fd_f *func);
int pollinatorPoll(Pollinator *p, int timeout_ms);
