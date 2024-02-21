#define ARRAY_H_IMPLEMENT
#include "array.h"

#include "common.h"
#include "Node.h"
#include "Pilatform.h"
#include "pollinator.h"
#include "pump.h"

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

	//v4l2PrintBuffer(&buf->buffer);

	const uint32_t offset = 0; // TODO ... IS_STREAM_MPLANE(st) ? buf->buffer.m.planes[0].m.data_offset : 0;
	const uint32_t length = IS_STREAM_MPLANE(st) ? buf->buffer.m.planes[0].bytesused : buf->buffer.bytesused;

	const uint64_t now_us = nowUs();
	const uint64_t dt_us = now_us - prev_frame_us;
	LOGI("Frame %d (map=%p off=%d size=%d), time=%.3fms, dt=%fms, fps=%fms",
		frame_count,
		buf->mapped[0],
		offset, length,
		now_us / 1000.f,
		dt_us / 1000.f,
		1000000.f / dt_us);
	prev_frame_us = now_us;

	if (length != fwrite(buf->mapped[0], 1, length, fout)) {
		LOGE("Failed to write %d bytes: %s (%d)", length, strerror(errno), errno);
		return -2;
	}

	frame_count++;

	if (0 != deviceStreamPushBuffer(st, buf))
		return -2;

	return 0;
}

struct Chain {
	Array devices;
};

static struct {
	Pump *cam2debay;
	Pump *debay2enc;

	int frame;
	FILE *fout;
} g;

static int bitSetFunc(int fd, uint32_t flags, uintptr_t arg1, uintptr_t arg2) {
	if (flags&POLLIN_FD_ERR) {
		LOGE("Error registered on fd=%d", fd);
		exit(1);
	}

	uint32_t *const bits = (uint32_t*)arg1;
	*bits |= arg2;

	return POLLINATOR_CONTINUE;
}

int main(int argc, const char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);
	g_begin_us = nowUs();
	if (argc != 3) {
		fprintf(stderr, "Usage: %s filename.{mjpeg|h264} frame_count\n", argv[0]);
		return 1;
	}

	const char *out_filename = argv[1];

	Node *const cam = piOpenCamera();
	if (!cam) {
		LOGE("Unable to open Rpi camera");
		return 1;
	}

	Node *const isp = piOpenISP();
	if (!isp) {
		LOGE("Unable to open Rpi ISP");
		return 1;
	}

	Node *const enc = piOpenEncoder(PiEncoderMJPEG);
	if (!enc) {
		LOGE("Unable to open Rpi encoder");
		return 1;
	}

	g.fout = fopen(out_filename, "wb");
	if (!g.fout) {
		LOGE("fopen(\"%s\") => %s (%d)", out_filename, strerror(errno), errno);
		return 1;
	}

	// N. Start streaming
	if (0 != nodeStart(enc)) {
		LOGE("Unable to start encoder");
		return 1;
	}

	if (0 != nodeStart(isp)) {
		LOGE("Unable to start ISP");
		return 1;
	}

	if (0 != nodeStart(cam)) {
		LOGE("Unable to start camera");
		return 1;
	}

	int max_frames = argc > 2 ? atoi(argv[2]) : 60;
	max_frames = max_frames >= 0 ? max_frames : 60;

	Pollinator pol;
	pollinatorInit(&pol);

	Pump *const cam_to_isp = pumpCreate(cam->output, isp->input);
	Pump *const isp_to_enc = pumpCreate(isp->output, enc->input);

#define CAM_TO_ISP_BIT (1<<0)
#define ISP_TO_ENC_BIT (1<<1)
#define FRAME_READY_BIT (1<<2)
	uint32_t bits = 0;

	pollinatorRegisterFd(&pol, cam->output->dev_fd, bitSetFunc, (uintptr_t)&bits, CAM_TO_ISP_BIT);
	pollinatorRegisterFd(&pol, isp->input->dev_fd, bitSetFunc, (uintptr_t)&bits, CAM_TO_ISP_BIT);

	pollinatorRegisterFd(&pol, isp->output->dev_fd, bitSetFunc, (uintptr_t)&bits, ISP_TO_ENC_BIT);
	pollinatorRegisterFd(&pol, enc->input->dev_fd, bitSetFunc, (uintptr_t)&bits, ISP_TO_ENC_BIT | FRAME_READY_BIT);

	prev_frame_us = nowUs();
	g.frame = 0;
	//while (frames > 0) { --frames;
	while (frame_count < max_frames) {
		bits = 0;

		//const uint64_t poll_pre = nowUs();
		const int result = pollinatorPoll(&pol, 5000);
		//const uint64_t poll_after = nowUs();
		//LOGI("Slept for %.3fms", (poll_after - poll_pre) / 1000.);

		if (result < 0) {
			LOGE("Pollinator returned %d", result);
			exit(1);
		}

		if (bits & CAM_TO_ISP_BIT) 
			pumpPump(cam_to_isp);

		if (bits & ISP_TO_ENC_BIT)
			pumpPump(isp_to_enc);

		if (bits & FRAME_READY_BIT)
			readFrame(enc->output, g.fout);
	}

	pumpDestroy(isp_to_enc);
	pumpDestroy(cam_to_isp);

	pollinatorFinalize(&pol);

	fclose(g.fout);

	nodeStop(cam);
	nodeStop(isp);
	nodeStop(enc);

	nodeDestroy(enc);
	nodeDestroy(isp);
	nodeDestroy(cam);

	return 0;
}
