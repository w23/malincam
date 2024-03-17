#include "Led.h"

#include <fcntl.h> // open
#include <errno.h> // errno
#include <string.h> // strerror
#include <unistd.h> // close, write

#define LED_SYS_BRIGHTNESS "/sys/class/leds/ACT/brightness"
#define LED_BLINK_PERIOD_MS 1000

static const char g_led_on[] = "255";
static const char g_led_off[] = "0";

static struct {
	int enabled;
	int on;
} g = {
	.on = 1
};

static void ledUpdate(int on) {
	if (on == g.on)
		return;

	g.on = on;

	const int fd = open(LED_SYS_BRIGHTNESS, O_WRONLY);
	if (!fd) {
		LOGE("Unable to open LED brightness node %s: %s (%d)", LED_SYS_BRIGHTNESS, strerror(errno), errno);
		return;
	}

	const char *const val = on ? g_led_on : g_led_off;
	int len = write(fd, val, strlen(val));
	if (len <= 0) {
		LOGE("Error writing to %s: %s (%d)", LED_SYS_BRIGHTNESS, strerror(errno), errno);
	}

	close(fd);
}

void ledBlinkEnable(int enabled) {
	ledUpdate(!enabled);
	g.enabled = enabled;
}

void ledBlinkUpdate(u32 now_ms) {
	if (!g.enabled)
		return;

	const int state = (now_ms / LED_BLINK_PERIOD_MS) & 1;
	ledUpdate(state);
}
