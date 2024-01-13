#include "common.h"
#include "v4l2.h"

#include <errno.h>
#include <string.h> // strerror

static int readFrame(DeviceV4L2 *dev, FILE *fout) {
	const Buffer *const buf = devV4L2PullBuffer(dev);
	// TODO differentiate between fatal and EAGAIN | EIO
	if (!buf)
		return 1;

	if (buf->buffer.length != fwrite(buf->v.mmap.ptr, 1, buf->buffer.length, fout)) {
		LOGE("Failed to write %d bytes: %s (%d)", buf->buffer.length, strerror(errno), errno);
		return -2;
	}

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
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /dev/video#\n", argv[0]);
		return 1;
	}

	struct DeviceV4L2 *dev = devV4L2Open(argv[1]);
	if (!dev) {
		LOGE("Failed to open device \"%s\"", argv[1]);
		return 1;
	}

	V4L2PrepareOpts opts = {
		.buffers_count = 3,
		.memory_type = V4L2_MEMORY_MMAP,
		.buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.width = 0, // Preserve
		.height = 0,
		//.userptr = NULL,
		//.buffer_func = NULL,
	};

	if (0 != devV4L2Start(dev, &opts)) {
		LOGE("Unable to start");
		goto fail;
	}

	pullFrames(dev, 60);

	//devV4L2Stop(dev);
	devV4L2Close(dev);
	return 0;

fail:
	devV4L2Close(dev);
	return 1;
}
