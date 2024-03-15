#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>

#include <stdint.h>

void v4l2PrintCapabilityBits(uint32_t caps);
void v4l2PrintBufferCapabilityBits(uint32_t caps);
void v4l2PrintCapability(const struct v4l2_capability* caps);
const char *v4l2InputTypeName(uint32_t type);
void v4l2PrintInput(const struct v4l2_input* input);
const char *v4l2BufTypeName(uint32_t type);
void v4l2PrintFormatDesc(const struct v4l2_fmtdesc* fmt);
void v4l2PrintFormat(const struct v4l2_format* fmt);
void v4l2PrintRequestBuffers(const struct v4l2_requestbuffers* req);
const char *v4l2MemoryTypeName(enum v4l2_memory type);
void v4l2PrintBuffer(const struct v4l2_buffer *buf);
void v4l2PrintFormatFlags(uint32_t flags);
const char *v4l2PixFmtName(uint32_t fmt);
void v4l2PrintFrmSizeEnum(const struct v4l2_frmsizeenum *fse);
const char *v4l2MbusFmtName(uint32_t format);
void v4l2PrintSelection(const struct v4l2_selection* sel);
const char* v4l2SelTgtName(uint32_t target);
const char* v4l2CtrlIdName(uint32_t ctrl_id);
const char* v4l2CtrlTypeName(uint32_t ctrl_type);
void v4l2PrintControlFlags(uint32_t flags);

void v4l2PrintSubdevCapability(const struct v4l2_subdev_capability *cap);
void v4l2PrintSubdevFormat(const struct v4l2_subdev_format *format);
void v4l2PrintSubdevSelection(const struct v4l2_subdev_selection *sel);
void v4l2PrintFrameInterval(const struct v4l2_subdev_frame_interval *fi);
void v4l2PrintSubdevMbusCode(const struct v4l2_subdev_mbus_code_enum *mbc);
const char *v4l2MbusFmtName(uint32_t format);
void v4l2PrintSubdevFrameSize(const struct v4l2_subdev_frame_size_enum *fsz);
void v4l2PrintSubdevFrameInterval(const struct v4l2_subdev_frame_interval_enum *fiv);
const char* v4l2SelTgtName(uint32_t target);
void v4l2PrintSelFlags(uint32_t bits);
