#define ARRAY_H_IMPLEMENT
#include "array.h"

#include "common.h"
#include "Node.h"
#include "Pilatform.h"
#include "pollinator.h"
#include "pump.h"
#include "UVC.h"

#include <errno.h>
#include <string.h> // strerror

//#define TEST_UVC_ONLY
#ifdef TEST_UVC_ONLY
#include <unistd.h> // sleep
#include <fcntl.h> // open
#endif

#include <time.h> // clock_gettime

uint64_t g_begin_us = 0;

uint64_t nowUs(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (ts.tv_sec * 1000000ul + ts.tv_nsec / 1000ul) - g_begin_us;
}

#ifndef TEST_UVC_ONLY
static uint64_t prev_frame_us = 0;
#endif

#if 0
static int frame_count = 0;

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
#endif

static int bitSetFunc(int fd, uint32_t flags, uintptr_t arg1, uintptr_t arg2) {
	if (flags&POLLIN_FD_ERR) {
		LOGE("Error registered on fd=%d, flags=%08x", fd, flags);
		exit(1);
	}

	uint32_t *const bits = (uint32_t*)arg1;
	*bits |= arg2;

	return POLLINATOR_CONTINUE;
}

#ifdef TEST_UVC_ONLY
static int testUvc(void) {
	Node *const uvc = uvcOpen("/dev/video2");
	if (!uvc) {
		LOGE("Unable to open uvc-gadget device");
		return 1;
	}

	//int fd2 = open("/dev/video2", O_RDWR | O_NONBLOCK);

	struct Pollinator *const pol = pollinatorCreate();

	uint32_t bits = 0;
	//pollinatorRegisterFd(&pol, uvc->input->dev_fd, bitSetFunc, (uintptr_t)&bits, 1);
	pollinatorRegisterFd(pol, &(PollinatorRegisterFd){
		//.fd = fd2,
		.fd = uvc->input->dev_fd,
		.event_bits = POLLIN_FD_EXCEPT,
		.func =	bitSetFunc,
		.arg1 = (uintptr_t)&bits,
		.arg2 = 1,
	});

	for (;;) {
		bits = 0;

		const uint64_t poll_pre = nowUs();
		const int result = pollinatorPoll(pol, 5000);
		const uint64_t poll_after = nowUs();
		LOGI("Slept for %.3fms", (poll_after - poll_pre) / 1000.);

		if (result < 0) {
			LOGE("Pollinator returned %d", result);
			exit(1);
		}

		if (bits) {
			//sleep(1);
			//if (0 == uvcProcessEvents(uvc))
				//sleep(1);
			uvcProcessEvents(uvc);
		}
	}

	return 0;
}
#endif // ifdef TEST_UVC_ONLY

int main(int argc, const char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);
	g_begin_us = nowUs();

#ifdef TEST_UVC_ONLY
	return testUvc();
#else
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

	Node *const enc = piOpenEncoder(PiEncoderJPEG);
	if (!enc) {
		LOGE("Unable to open Rpi encoder");
		return 1;
	}

	Node *const uvc = uvcOpen("/dev/video2");
	if (!uvc) {
		LOGE("Unable to open uvc-gadget device");
		return 1;
	}

	// FIXME only start streaming when uvc-gadget says so

	// Start streaming
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

	struct Pollinator *const pol = pollinatorCreate();

	Pump *const cam_to_isp = pumpCreate(cam->output, isp->input);
	Pump *const isp_to_enc = pumpCreate(isp->output, enc->input);
	//Pump *const enc_to_uvc = pumpCreate(enc->output, uvc->input);

#define CAM_TO_ISP_BIT (1<<0)
#define ISP_TO_ENC_BIT (1<<1)
#define ENC_TO_UVC_BIT (1<<2)
#define UVC_EVENTS_BIT (1<<3)
	uint32_t bits = 0;

	pollinatorRegisterFd(pol, &(PollinatorRegisterFd){
		.fd = cam->output->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&bits,
		.arg2 = CAM_TO_ISP_BIT});
	pollinatorRegisterFd(pol, &(PollinatorRegisterFd){
		.fd = isp->input->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&bits,
		.arg2 = CAM_TO_ISP_BIT});

	// FIXME if using single-device isp /dev/video12, then this fd will be the same as input
	pollinatorRegisterFd(pol, &(PollinatorRegisterFd){
		.fd = isp->output->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&bits,
		.arg2 = ISP_TO_ENC_BIT});

	pollinatorRegisterFd(pol, &(PollinatorRegisterFd){
		.fd = enc->input->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&bits,
		.arg2 = ISP_TO_ENC_BIT | ENC_TO_UVC_BIT});
	pollinatorRegisterFd(pol, &(PollinatorRegisterFd){
		.fd = uvc->input->dev_fd,
		.event_bits = POLLIN_FD_EXCEPT | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&bits,
		.arg2 = ENC_TO_UVC_BIT | UVC_EVENTS_BIT});

	prev_frame_us = nowUs();
	for (;;) {
		bits = 0;

		//const uint64_t poll_pre = nowUs();
		const int result = pollinatorPoll(pol, 5000);
		//const uint64_t poll_after = nowUs();
		//LOGI("Slept for %.3fms", (poll_after - poll_pre) / 1000.);

		if (result < 0) {
			LOGE("Pollinator returned %d", result);
			exit(1);
		}

		if (bits & UVC_EVENTS_BIT) {
			uvcProcessEvents(uvc);
		}

		if (bits & CAM_TO_ISP_BIT) {
			const int result = pumpPump(cam_to_isp);
			if (0 != result) {
				LOGE("cam-to-isp pump error: %d", result);
				return 1;
			}
		}

		if (bits & ISP_TO_ENC_BIT) {
			const int result = pumpPump(isp_to_enc);
			if (0 != result) {
				LOGE("isp-to-enc pump error: %d", result);
				return 1;
			}
		}

		/*
		if (bits & ENC_TO_UVC_BIT) {
			const int result = pumpPump(enc_to_uvc);
			if (0 != result) {
				LOGE("enc-to-uvc pump error: %d", result);
				return 1;
			}
		}*/
	}

	pumpDestroy(isp_to_enc);
	pumpDestroy(cam_to_isp);

	pollinatorDestroy(pol);

	nodeStop(cam);
	nodeStop(isp);
	nodeStop(enc);

	nodeDestroy(enc);
	nodeDestroy(isp);
	nodeDestroy(cam);

	return 0;
#endif // else ifdef TEST_UVC_ONLY
}
