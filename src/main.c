#define ARRAY_H_IMPLEMENT
#include "array.h"

#include "common.h"
#include "Led.h"
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
	UNUSED(fd);
	if (flags&POLLIN_FD_ERR) {
		//LOGE("Error registered on fd=%d, flags=%08x", fd, flags);
		//exit(1);
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
	//pollinatorMonitorFd(&pol, uvc->input->dev_fd, bitSetFunc, (uintptr_t)&bits, 1);
	pollinatorMonitorFd(pol, &(PollinatorMonitorFd){
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

#define CAM_TO_ISP_BIT (1<<0)
#define ISP_TO_ENC_BIT (1<<1)
#define ENC_TO_UVC_BIT (1<<2)
#define UVC_EVENTS_BIT (1<<3)

typedef struct {
	Node *cam;
	Node *isp;
	Node *enc;
	Node *uvc;

	struct Pollinator *pol;

	Pump *cam_to_isp;
	Pump *isp_to_enc;
	Pump *enc_to_uvc;

	uint32_t fd_bits;
} Pipeline;

static Pipeline g_pipeline = {0};

static int uvcEventStreamon(int streamon);

static int pipelineCreate(void) {
	Pipeline *const p = &g_pipeline;

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

	Node *const uvc = uvcOpen((UvcOpenArgs){
		.dev_name = "/dev/video2",
		.event_streamon = uvcEventStreamon,
		// TODO .controls.brightness = 
	});
	if (!uvc) {
		LOGE("Unable to open uvc-gadget device");
		return 1;
	}

	p->cam = cam;
	p->isp = isp;
	p->enc = enc;
	p->uvc = uvc;

	p->pol = pollinatorCreate();

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = uvc->input->dev_fd,
		.event_bits = POLLIN_FD_EXCEPT,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = UVC_EVENTS_BIT});

	return 0;
}

static void pipelineDestroy(void) {
	Pipeline *const p = &g_pipeline;

	pumpDestroy(p->isp_to_enc);
	pumpDestroy(p->cam_to_isp);

	nodeDestroy(p->enc);
	nodeDestroy(p->isp);
	nodeDestroy(p->cam);

	pollinatorDestroy(p->pol);
}

int pipelineStart(void) {
	Pipeline *const p = &g_pipeline;

	if (0 != nodeStart(p->uvc)) {
		LOGE("Unable to start uvc-gadget");
		return 1;
	}

	if (0 != nodeStart(p->enc)) {
		LOGE("Unable to start encoder");
		return 1;
	}

	if (0 != nodeStart(p->isp)) {
		LOGE("Unable to start ISP");
		return 1;
	}

	if (0 != nodeStart(p->cam)) {
		LOGE("Unable to start camera");
		return 1;
	}

	p->cam_to_isp = pumpCreate(p->cam->output, p->isp->input);
	p->isp_to_enc = pumpCreate(p->isp->output, p->enc->input);
	p->enc_to_uvc = pumpCreate(p->enc->output, p->uvc->input);

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->cam->output->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = CAM_TO_ISP_BIT});

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->isp->input->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = CAM_TO_ISP_BIT});

	// FIXME if using single-device isp /dev/video12, then this fd will be the same as input
	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->isp->output->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = ISP_TO_ENC_BIT});

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->enc->input->dev_fd,
		.event_bits = POLLIN_FD_READ | POLLIN_FD_WRITE,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = ISP_TO_ENC_BIT | ENC_TO_UVC_BIT});

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->uvc->input->dev_fd,
		.event_bits = POLLIN_FD_EXCEPT | POLLIN_FD_WRITE | POLLIN_FD_READ,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = ENC_TO_UVC_BIT | UVC_EVENTS_BIT});

	ledBlinkEnable(1);
	return 0;
}

int pipelineStop(void) {
	Pipeline *const p = &g_pipeline;

	ledBlinkEnable(0);

	nodeStop(g_pipeline.uvc);
	nodeStop(g_pipeline.enc);
	nodeStop(g_pipeline.isp);
	nodeStop(g_pipeline.cam);

	pumpDestroy(p->enc_to_uvc); p->enc_to_uvc = NULL;
	pumpDestroy(p->isp_to_enc); p->isp_to_enc = NULL;
	pumpDestroy(p->cam_to_isp); p->cam_to_isp = NULL;

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->cam->output->dev_fd,
		.event_bits = 0,
	});

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->isp->input->dev_fd,
		.event_bits = 0});

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->isp->output->dev_fd,
		.event_bits = 0});

	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->enc->input->dev_fd,
		.event_bits = 0});

	// Continue monitoring uvc events
	pollinatorMonitorFd(p->pol, &(PollinatorMonitorFd){
		.fd = p->uvc->input->dev_fd,
		.event_bits = POLLIN_FD_EXCEPT,
		.func = bitSetFunc,
		.arg1 = (uintptr_t)&p->fd_bits,
		.arg2 = UVC_EVENTS_BIT});

	return 0;
}

static int pipelineProcess(void) {
	Pipeline *const p = &g_pipeline;

	g_pipeline.fd_bits = 0;

	const uint64_t poll_pre = nowUs();
	const int result = pollinatorPoll(g_pipeline.pol, 5000);
	const uint64_t now_us = nowUs();
	if (!g_pipeline.fd_bits) {
		LOGI("Slept for %.3fms", (now_us - poll_pre) / 1000.);
		//g_pipeline.fd_bits = 0xff;
	}

	ledBlinkUpdate(now_us / 1000);

	if (result < 0) {
		LOGE("Pollinator returned %d", result);
		exit(1);
	}

	if (g_pipeline.fd_bits & UVC_EVENTS_BIT) {
		uvcProcessEvents(g_pipeline.uvc);
	}

	// After this point stream might have stopped already, but we'd still have lingering bits singaling transfer...

	if (g_pipeline.cam_to_isp && g_pipeline.fd_bits & CAM_TO_ISP_BIT) {
		const int result = pumpPump(g_pipeline.cam_to_isp);
		if (0 != result) {
			LOGE("cam-to-isp pump error: %d", result);
			//return 1;
		}
	}

	if (g_pipeline.isp_to_enc && g_pipeline.fd_bits & ISP_TO_ENC_BIT) {
		const int result = pumpPump(g_pipeline.isp_to_enc);
		if (0 != result) {
			LOGE("isp-to-enc pump error: %d", result);
			//return 1;
		}
	}

	if (g_pipeline.enc_to_uvc && g_pipeline.fd_bits & ENC_TO_UVC_BIT) {
		const int result = pumpPump(p->enc_to_uvc);
		if (0 != result) {
			LOGE("enc-to-uvc pump error: %d", result);
			//return 1;
		}
	}

	return 0;
}

static int uvcEventStreamon(int streamon) {
	switch (streamon) {
		case 1: return pipelineStart();
		case 0: return pipelineStop();
	}
	return -EINVAL;
}

int main(int argc, const char *argv[]) {
	UNUSED(argc);
	UNUSED(argv);
	g_begin_us = nowUs();

#ifdef TEST_UVC_ONLY
	return testUvc();
#else

	prev_frame_us = nowUs();

	if (pipelineCreate() != 0) {
		LOGE("Failed to create pipeline");
		return 1;
	}

	while (pipelineProcess() == 0);

	pipelineDestroy();

	return 0;
#endif // else ifdef TEST_UVC_ONLY
}
