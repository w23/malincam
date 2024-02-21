#include "pollinator.h"
#include "common.h"

#include <poll.h>
#include <errno.h>
#include <string.h> // strerror

typedef struct {
	pollin_fd_f *func;
	uintptr_t arg1, arg2;
} PollinatorFd;

void pollinatorInit(Pollinator *p) {
	arrayInit(&p->fds, PollinatorFd);
	arrayInit(&p->fd_arg, struct pollfd);
}

void pollinatorFinalize(Pollinator *p) {
	arrayDestroy(&p->fd_arg);
	arrayDestroy(&p->fds);
}

//int pollinatorRegisterFd(Pollinator *p, int fd, void *userptr, pollin_fd_f *func) {
int pollinatorRegisterFd(Pollinator *p, int fd, pollin_fd_f *func, uintptr_t arg1, uintptr_t arg2) {
	PollinatorFd new_fd = {
		.func = func,
		.arg1 = arg1,
		.arg2 = arg2,
	};
	arrayAppend(&p->fds, &new_fd);

	struct pollfd arg = {
		.fd = fd,
		.events = POLLIN | POLLOUT | POLLERR | POLLHUP,
		.revents = 0,
	};
	arrayAppend(&p->fd_arg, &arg);

	return 0;
}

int pollinatorPoll(Pollinator *p, int timeout_ms) {
	const int count = poll((struct pollfd*)p->fd_arg.data, p->fd_arg.size, timeout_ms);
	if (count < 0) {
		LOGE("poll(num=%d) returned %d: %s", p->fd_arg.size, errno, strerror(errno));
		return count;
	}

	int result = POLLINATOR_CONTINUE;
	for (int i = 0; i < p->fd_arg.size; ++i) {
		const struct pollfd *const pfd = arrayAtConst(&p->fd_arg, struct pollfd, i);
		const PollinatorFd *const fd = arrayAtConst(&p->fds, PollinatorFd, i);
		
		const uint32_t flags = 0
			| ((pfd->revents & POLLIN) ? POLLIN_FD_READ : 0)
			| ((pfd->revents & POLLOUT) ? POLLIN_FD_WRITE : 0)
			| ((pfd->revents & (POLLERR | POLLHUP)) ? POLLIN_FD_ERR : 0);

		if (0 == flags)
			continue;

		const int func_result = fd->func(pfd->fd, flags, fd->arg1, fd->arg2);
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
