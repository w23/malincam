#define ARRAY_H_IMPLEMENT
#include "array.h"

#include "common.h"
#include "device.h"
#include "subdev.h"

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

static int readFrame(DeviceStream *st, FILE *fout) {
	const Buffer *const buf = deviceStreamPullBuffer(st);
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

	if (frame_count == 0 && buf->buffer.bytesused != fwrite(buf->mapped[0], 1, buf->buffer.bytesused, fout)) {
		LOGE("Failed to write %d bytes: %s (%d)", buf->buffer.bytesused, strerror(errno), errno);
		return -2;
	}

	frame_count++;

	if (0 != deviceStreamPushBuffer(st, buf))
		return -2;

	return 0;
}

static void pullFrames(Device *dev, int frames) {
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
					const int res = readFrame(&dev->capture, fout);
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

#define SENSOR_SUBDEV "/dev/v4l-subdev0"
#define CAMERA_DEV "/dev/video0"
#define DEBAYER_ISP_DEV "/dev/video12"
#define ENCODER_DEV "/dev/video11"

int main(int argc, const char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);
	g_begin_us = nowUs();
	/* if (argc != 3) { */
	/* 	fprintf(stderr, "Usage: %s /dev/v4l-subdev# /dev/video#\n", argv[0]); */
	/* 	return 1; */
	/* } */

	// 1. Set pixel format and resolution on the sensor subdevice
	Subdev *const sensor = subdevOpen(SENSOR_SUBDEV, 2);
	if (!sensor) {
		LOGE("Failed to open sensor subdev");
		return 1;
	}

	SubdevSet ss = {
		.pad = 0,
		.mbus_code = MEDIA_BUS_FMT_SRGGB10_1X10,

		// TODO where to crop?
		.width = 1332,
		.height = 990,
		//.mbus_code = MEDIA_BUS_FMT_SRGGB12_1X12,
		//.width = 2664,
		//.height = 1980,
	};
	if (0 != subdevSet(sensor, &ss)) {
		LOGE("Failed to set up subdev");
		return 1;
	}

	// 2. Open camera device
	Device *const camera = deviceOpen(CAMERA_DEV);
	if (!camera) {
		LOGE("Failed to open camera device");
		return 1;
	}

	if (0 != deviceStreamQueryFormats(&camera->capture, ss.mbus_code)) {
		LOGE("Failed to query camera:capture stream formats");
		return 1;
	}

	const DeviceStreamPrepareOpts camera_capture_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_EXPORT,
		//.pixelformat = V4L2_PIX_FMT_YUYV,
		//.pixelformat = V4L2_PIX_FMT_SRGGB10,
		.pixelformat = V4L2_PIX_FMT_SBGGR10,
		.width = ss.width,
		.height = ss.height,
		//.userptr = NULL,
		//.buffer_func = NULL,
	};

	if (0 != deviceStreamPrepare(&camera->capture, &camera_capture_opts)) {
		LOGE("Unable to prepare camera:capture stream");
		return 1;
	}

	// 3. Open Bayer to YUV encoder
	// /dev/video12 ?
	Device *const debayer_isp = deviceOpen(DEBAYER_ISP_DEV);
	if (!debayer_isp) {
		LOGE("Failed to open debayer isp device");
		return 1;
	}

	if (0 != deviceStreamQueryFormats(&debayer_isp->output, 0)) {
		LOGE("Failed to query debayer:output stream formats");
		return 1;
	}

	DeviceStreamPrepareOpts debayer_output_opts = camera_capture_opts;
	debayer_output_opts.buffer_memory = BUFFER_MEMORY_DMABUF_IMPORT;
	if (0 != deviceStreamPrepare(&debayer_isp->output, &debayer_output_opts)) {
		LOGE("Unable to prepare debayer:output stream");
		return 1;
	}

	if (0 != deviceStreamQueryFormats(&debayer_isp->capture, 0)) {
		LOGE("Failed to query debayer:capture stream formats");
		return 1;
	}

	const DeviceStreamPrepareOpts debayer_capture_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_DMABUF_EXPORT,
		.pixelformat = V4L2_PIX_FMT_YUYV,
		.width = ss.width,
		.height = ss.height,
		//.userptr = NULL,
		//.buffer_func = NULL,
	};

	if (0 != deviceStreamPrepare(&debayer_isp->capture, &debayer_capture_opts)) {
		LOGE("Unable to prepare debayer capture stream");
		return 1;
	}

	// See https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dev-encoder.html
	// 3.1 Enum CAPTURE (i.e. "read") formats
	// 3.2 Set CAPTURE format
	//     - Will return sizeimage, width, height set for currently set resolution
	// 3.3 Enum OUTPUT (i.e. "write") formats (Depends on CAPTURE format)
	// 3.4 Enum framesizes
	// 3.5 Enum frameintervals
	// 3.6 Query controls (codec opts, etc)
	// 3.7 Set OUTPUT format (including width x height)
	// 3.8 Set frame interval
	// 3.9 Set selection/crop
	// 3.10 Alloc and prepare buffers
	//      - See https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dmabuf.html#dmabuf
	//      - VIDIOC_REQBUFS to allocated them internally
	//        https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/vidioc-reqbufs.html#vidioc-reqbufs
	//      - VIDIOC_EXPBUF to get DMABUF fds
	//        https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/vidioc-expbuf.html#vidioc-expbuf
	// 3.11 Start streaming on both in/out
	//      Set timestamp field on "write" buffers to match them to "read" ones

	// 4. Open YUV to MJPEG encoder
	// /dev/video11
	Device *const encoder = deviceOpen(ENCODER_DEV);
	if (!encoder) {
		LOGE("Failed to open encoder device");
		return 1;
	}

	if (0 != deviceStreamQueryFormats(&encoder->output, 0)) {
		LOGE("Failed to query encoder:output stream formats");
		return 1;
	}

	DeviceStreamPrepareOpts encoder_output_opts = debayer_capture_opts;
	encoder_output_opts.buffer_memory = BUFFER_MEMORY_DMABUF_IMPORT;
	if (0 != deviceStreamPrepare(&encoder->output, &encoder_output_opts)) {
		LOGE("Unable to prepare encoder output stream");
		return 1;
	}

	if (0 != deviceStreamQueryFormats(&encoder->capture, 0)) {
		LOGE("Failed to query encoder:capture stream formats");
		return 1;
	}

	const DeviceStreamPrepareOpts encoder_capture_opts = {
		.buffers_count = 3,
		.buffer_memory = BUFFER_MEMORY_MMAP,
		.pixelformat = V4L2_PIX_FMT_MJPEG,
		.width = ss.width,
		.height = ss.height,
		//.userptr = NULL,
		//.buffer_func = NULL,
	};

	if (0 != deviceStreamPrepare(&encoder->capture, &encoder_capture_opts)) {
		LOGE("Unable to prepare encoder capture stream");
		return 1;
	}

	// N. Start streaming

	if (0 != deviceStreamStart(&encoder->capture)) {
		LOGE("Unable to start encoder:capture");
		return 1;
	}
	if (0 != deviceStreamStart(&encoder->output)) {
		LOGE("Unable to start encoder:output");
		return 1;
	}
	if (0 != deviceStreamStart(&debayer_isp->capture)) {
		LOGE("Unable to start debayer:capture");
		return 1;
	}
	if (0 != deviceStreamStart(&debayer_isp->output)) {
		LOGE("Unable to start debayer:output");
		return 1;
	}
	if (0 != deviceStreamStart(&camera->capture)) {
		LOGE("Unable to start camera:capture");
		return 1;
	}

	pullFrames(camera, 60);

	deviceStreamStop(&camera->capture);
	deviceStreamStop(&debayer_isp->output);
	deviceStreamStop(&debayer_isp->capture);
	deviceStreamStop(&encoder->output);
	deviceStreamStop(&encoder->capture);

	deviceClose(encoder);
	deviceClose(debayer_isp);
	deviceClose(camera);
	subdevClose(sensor);
	return 0;
}
