#define ARRAY_H_IMPLEMENT
#include "array.h"

#include "common.h"
#include "v4l2.h"
#include "v4l2-subdev.h"

#include <errno.h>
#include <string.h> // strerror

#include <time.h> // clock_gettime

uint64_t g_begin_us = 0;

uint64_t nowUs(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (ts.tv_sec * 1000000ul + ts.tv_nsec / 1000ul) - g_begin_us;
}

static int frame_count = 0;
static uint64_t prev_frame_us = 0;

static int readFrame(DeviceV4L2 *dev, FILE *fout) {
	const Buffer *const buf = devV4L2PullBuffer(dev);
	// TODO differentiate between fatal and EAGAIN | EIO
	if (!buf)
		return 1;

	const uint64_t now_us = nowUs();
	const uint64_t dt_us = now_us - prev_frame_us;
	LOGI("Frame %d, time=%.3fms, dt=%fms, fps=%fms",
		frame_count,
		now_us / 1000.f,
		dt_us / 1000.f,
		1000000.f / dt_us);
	prev_frame_us = now_us;

	if (frame_count == 0 && buf->buffer.length != fwrite(buf->v.mmap.ptr, 1, buf->buffer.length, fout)) {
		LOGE("Failed to write %d bytes: %s (%d)", buf->buffer.length, strerror(errno), errno);
		return -2;
	}

	frame_count++;

	if (0 != devV4L2PushBuffer(dev, buf))
		return -2;

	return 0;
}

static void pullFrames(DeviceV4L2 *dev, int frames) {
	const char *out_filename = "frames.mjpeg";
	FILE *fout = fopen(out_filename, "wb");
	if (!fout) {
		LOGE("fopen(\"%s\") => %s (%d)", out_filename, strerror(errno), errno);
		return;
	}

	prev_frame_us = nowUs();
	while (frames-- > 0) {
		for (;;) {
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(dev->fd, &fds);

			struct timeval tv = {
				.tv_sec = 1,
				.tv_usec = 0,
			};

			const int result = select(dev->fd + 1, &fds, NULL, NULL, &tv);
			switch (result) {
				case -1:
					if (EINTR == errno)
						continue;
					LOGE("select() = %s (%d)", strerror(errno), errno);
					goto exit;
				case 0:
					LOGI("select() timeout");
					continue;
				case 1: {
					const int res = readFrame(dev, fout);
					if (res < 0) {
						LOGE("readFrame failed, exiting");
						goto exit;
					}

					// Retry
					if (res > 0)
						continue;

					break;
					}
			} // switch select result

			break;
		} // select loop
	} // frame count loop

exit:
	fclose(fout);
}

int main(int argc, const char *argv[]) {
	g_begin_us = nowUs();
	if (argc != 3) {
		fprintf(stderr, "Usage: %s /dev/v4l-subdev# /dev/video#\n", argv[0]);
		return 1;
	}

	Subdev *sd = subdevOpen(argv[1], 2);
	if (!sd) exit(1);

	SubdevSet ss = {
		.pad = 0,
		.mbus_code = MEDIA_BUS_FMT_SRGGB10_1X10,
		.width = 1332,
		.height = 990,
		//.mbus_code = MEDIA_BUS_FMT_SRGGB12_1X12,
		//.width = 2664,
		//.height = 1980,
	};
	if (0 != subdevSet(sd, &ss)) {
		LOGE("Failed to set up subdev");
		return 1;
	}

	struct DeviceV4L2 *dev = devV4L2Open(argv[2]);
	if (!dev) {
		LOGE("Failed to open device \"%s\"", argv[1]);
		return 1;
	}

	int status = 0;
	V4L2PrepareOpts opts = {
		.buffers_count = 3,
		.memory_type = V4L2_MEMORY_MMAP,
		//.pixelformat = V4L2_PIX_FMT_YUYV,
		//.pixelformat = V4L2_PIX_FMT_SRGGB10,
		.pixelformat = V4L2_PIX_FMT_SBGGR10,
		.width = ss.width,
		.height = ss.height,
		//.userptr = NULL,
		//.buffer_func = NULL,
	};

	if (0 != devV4L2EndpointStart(dev, 0, &opts)) {
		LOGE("Unable to start");
		status = 1;
		goto exit;
	}

	pullFrames(dev, 60);

	devV4L2EndpointStop(dev, 0);

exit:
	devV4L2Close(dev);
	return status;
}
