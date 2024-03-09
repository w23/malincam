#include "pollinator.h"
#include "common.h"

#include <sys/epoll.h>
#include <errno.h>
#include <string.h> // strerror
#include <unistd.h> // close
#include <stdlib.h> // calloc

typedef struct Pollinator {
	Array fds;
	int epoll_fd;
} Pollinator;

typedef struct {
	int fd;
	pollin_fd_f *func;
	uintptr_t arg1, arg2;
} PollinatorFd;

struct Pollinator *pollinatorCreate(void) {
	Pollinator *p = calloc(sizeof(Pollinator), 1);
	arrayInit(&p->fds, PollinatorFd);

	p->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
	ASSERT(p->epoll_fd > 0);
	return p;
}

void pollinatorFinalize(Pollinator *p) {
	if (!p)
		return;

	arrayDestroy(&p->fds);
	close(p->epoll_fd);
	free(p);
}

//int pollinatorRegisterFd(Pollinator *p, int fd, void *userptr, pollin_fd_f *func) {
int pollinatorRegisterFd(Pollinator *p, const PollinatorRegisterFd *reg) {
	const int index = arraySize(&p->fds);
	PollinatorFd new_pfd = {
		.fd = reg->fd,
		.func = reg->func,
		.arg1 = reg->arg1,
		.arg2 = reg->arg2,
	};
	arrayAppend(&p->fds, &new_pfd);

	struct epoll_event event = {
		.data.u32 = index,
		.events = EPOLLET
			| ((reg->event_bits & POLLIN_FD_READ) ? EPOLLIN : 0)
			| ((reg->event_bits & POLLIN_FD_WRITE) ? EPOLLOUT : 0)
			| ((reg->event_bits & POLLIN_FD_EXCEPT) ? EPOLLPRI : 0)
			| ((reg->event_bits & POLLIN_FD_ERR) ? EPOLLERR : 0),
	};
	const int result = epoll_ctl(p->epoll_fd, EPOLL_CTL_ADD, reg->fd, &event);
	ASSERT(result == 0);
	return 0;
}

int pollinatorPoll(Pollinator *p, int timeout_ms) {
#define MAX_EVENTS 16
	struct epoll_event events[MAX_EVENTS];
	const int count = epoll_wait(p->epoll_fd, events, MAX_EVENTS, timeout_ms);
	if (count < 0) {
		LOGE("epoll_wait returned %d: %s", errno, strerror(errno));
		return count;
	}

	int result = POLLINATOR_CONTINUE;
	for (int i = 0; i < count; ++i) {
		const struct epoll_event *const e = events + i;
		const int index = e->data.u32;
		const PollinatorFd *const fd = arrayAtConst(&p->fds, PollinatorFd, index);

		/*
		LOGI("[e=%d/%d] fd=%d revents=%s%s%s%s", i, count, fd->fd,
			(e->events & EPOLLIN) ? "EPOLLIN " : "",
			(e->events & EPOLLOUT) ? "EPOLLOUT " : "",
			(e->events & EPOLLPRI) ? "EPOLLPRI " : "",
			(e->events & EPOLLERR) ? "EPOLLERR " : ""
		);
		*/

		const uint32_t flags = 0
			| ((e->events & EPOLLIN) ? POLLIN_FD_READ : 0)
			| ((e->events & EPOLLOUT) ? POLLIN_FD_WRITE : 0)
			| ((e->events & EPOLLPRI) ? POLLIN_FD_EXCEPT : 0)
			| ((e->events & (EPOLLERR | EPOLLHUP)) ? POLLIN_FD_ERR : 0);

		if (0 == flags)
			continue;

		const int func_result = fd->func(fd->fd, flags, fd->arg1, fd->arg2);
		switch (func_result) {
			case POLLINATOR_CONTINUE:
				break;
			case POLLINATOR_STOP:
				LOGE("POLLINATOR_STOP is not implemented");
				break;
			default:
				result = func_result;
		}
	}

	return result;
}
