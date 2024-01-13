#include "common.h"
#include "v4l2.h"

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
		.width = 0,
		.height = 0,
		.userptr = NULL,
		.buffer_func = NULL,
	};

	devV4L2Prepare(dev, &opts);

	devV4L2Close(dev);
	return 0;
}
