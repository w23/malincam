#include <linux/videodev2.h>

#include <stdio.h>
#include <errno.h>
#include <string.h> // strerror

// open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h> // close
#include <sys/ioctl.h> // ioctl
#include <stdint.h> // uint32_t et al.

#define LOG(prefix, fmt, ...) \
	fprintf(stderr, prefix fmt "\n", ##__VA_ARGS__)

#define LOGI(fmt, ...) LOG("[INF] ", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG("[ERR] ", fmt, ##__VA_ARGS__)

#define MCAM_V4L2_CAPS(X) \
	X(V4L2_CAP_VIDEO_CAPTURE) \
	X(V4L2_CAP_VIDEO_OUTPUT) \
	X(V4L2_CAP_VIDEO_OVERLAY) \
	X(V4L2_CAP_VBI_CAPTURE) \
	X(V4L2_CAP_VBI_OUTPUT) \
	X(V4L2_CAP_SLICED_VBI_CAPTURE) \
	X(V4L2_CAP_SLICED_VBI_OUTPUT) \
	X(V4L2_CAP_RDS_CAPTURE) \
	X(V4L2_CAP_VIDEO_OUTPUT_OVERLAY) \
	X(V4L2_CAP_HW_FREQ_SEEK) \
	X(V4L2_CAP_RDS_OUTPUT) \
	X(V4L2_CAP_VIDEO_CAPTURE_MPLANE) \
	X(V4L2_CAP_VIDEO_OUTPUT_MPLANE) \
	X(V4L2_CAP_VIDEO_M2M_MPLANE) \
	X(V4L2_CAP_VIDEO_M2M) \
	X(V4L2_CAP_TUNER) \
	X(V4L2_CAP_AUDIO) \
	X(V4L2_CAP_RADIO) \
	X(V4L2_CAP_MODULATOR) \
	X(V4L2_CAP_SDR_CAPTURE) \
	X(V4L2_CAP_EXT_PIX_FORMAT) \
	X(V4L2_CAP_SDR_OUTPUT) \
	X(V4L2_CAP_META_CAPTURE) \
	X(V4L2_CAP_READWRITE) \
	X(V4L2_CAP_STREAMING) \
	X(V4L2_CAP_META_OUTPUT) \
	X(V4L2_CAP_TOUCH) \
	X(V4L2_CAP_IO_MC) \
	X(V4L2_CAP_DEVICE_CAPS) \

static void v4l2_print_caps(uint32_t caps) {
#define X(cap) if (caps & cap) LOGI("  %s", #cap);
	MCAM_V4L2_CAPS(X)
#undef X
}

static int v4l2_query_caps(int fd) {
	struct v4l2_capability caps;
	if (0 != ioctl(fd, VIDIOC_QUERYCAP, &caps)) {
		LOGE("Failed to ioctl(%d, VIDIOC_QUERYCAP): %d, %s", fd, errno, strerror(errno));
		return 1;
	}

	LOGI("caps.driver = %s", caps.driver);
	LOGI("caps.card = %s", caps.card);
	LOGI("caps.bus_info = %s", caps.bus_info);
#define FROM_KERNEL_VERSION(v) (((v)>>16)&0xff), (((v)>>8)&0xff), ((v)&0xff)
	LOGI("caps.version = %d.%d.%d", FROM_KERNEL_VERSION(caps.version));
	LOGI("caps.capabilities = %08x:", caps.capabilities);
	v4l2_print_caps(caps.capabilities);
	
	if (caps.capabilities & V4L2_CAP_DEVICE_CAPS) {
		LOGI("caps.device_caps = %08x:", caps.device_caps);
		v4l2_print_caps(caps.device_caps);
	}

	return 0;
}

static const char *v4l2_input_type_name(uint32_t type) {
	switch (type) {
		case V4L2_INPUT_TYPE_TUNER: return "V4L2_INPUT_TYPE_TUNER";
		case V4L2_INPUT_TYPE_CAMERA: return "V4L2_INPUT_TYPE_CAMERA";
		case V4L2_INPUT_TYPE_TOUCH: return "V4L2_INPUT_TYPE_TOUCH";
		default: return "UNKNOWN";
	}

}

static int v4l2_enum_input(int fd) {
	for (int i = 0;; ++i) {
		struct v4l2_input input;
		input.index = i;
		if (0 != ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
			if (errno == EINVAL) {
				LOGI("Device has %d inputs", i);
				return 0;
			}

			if (errno = ENOTTY) {
				LOGI("Device has no inputs");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_ENUMINPUT): %d, %s", fd, errno, strerror(errno));
			return 1;
		}

		LOGI("input[%d].name = %s", i, input.name);
		LOGI("input[%d].type = %s (%d)", i, v4l2_input_type_name(input.type), input.type);
		LOGI("input[%d].audioset = %08x", i, input.audioset);
		LOGI("input[%d].tuner = %x", i, input.tuner);
		LOGI("input[%d].std = %llx", i, (long long)input.std); // TODO
		LOGI("input[%d].status = %d", i, input.status); // TODO
		LOGI("input[%d].capabilities = %08x", i, input.capabilities); // TODO
	}
}

static int v4l2_enum_controls_ext(int fd) {
	struct v4l2_queryctrl qctrl;
	qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	while (0 == ioctl(fd, VIDIOC_QUERYCTRL, &qctrl)) {
		LOGI("qctrl.name = %s", qctrl.name);
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}

	return 0;
}

static int v4l2_enum_formats(int fd) {
	for (int i = 0;; ++i) {
		struct v4l2_fmtdesc fmt;
		fmt.index = i;
		fmt.mbus_code = 0;
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // TODO
		if (0 != ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {
			if (errno == EINVAL) {
				LOGI("Device has %d formats", i);
				return 0;
			}

			if (errno = ENOTTY) {
				LOGI("Device has no formats");
				return 0;
			}

			LOGE("Failed to ioctl(%d, VIDIOC_ENUM_FMT): %d, %s", fd, errno, strerror(errno));
			return 1;
		}

		LOGI("fmt[%d].flags = %08x", i, fmt.flags);
		LOGI("fmt[%d].description = %s", i, fmt.description);
		LOGI("fmt[%d].pixelformat = %08x", i, fmt.pixelformat);
	}
}

static int v4l2_open(const char *devname) {
	int status = 0;
	LOGI("Opening %s...", devname);
	const int fd = open(devname, O_RDWR);
	if (fd < 0) {
		LOGE("Failed to open \"%s\": %d, %s", devname, errno, strerror(errno));
		return 1;
	}

	LOGI("Opened \"%s\" = %d", devname, fd);

	if (0 != v4l2_query_caps(fd)) {
		status = 1;
		goto cleanup;
	}

	v4l2_enum_input(fd);
	v4l2_enum_controls_ext(fd);
	v4l2_enum_formats(fd);

cleanup:
	close(fd);
	return status;
}

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /dev/video#\n", argv[0]);
		return 1;
	}

	return v4l2_open(argv[1]);
}
