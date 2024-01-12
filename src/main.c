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

	devV4L2Close(dev);
	return 0;
}
