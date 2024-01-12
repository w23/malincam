#pragma once

struct DeviceV4L2;

struct DeviceV4L2* devV4L2Open(const char *devname);
void devV4L2Close(struct DeviceV4L2* dev);
