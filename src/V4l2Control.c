#include "V4l2Control.h"

#include "v4l2-print.h"
#include "common.h"

#include <errno.h>
#include <string.h> // strerror
#include <sys/ioctl.h> // ioctl

V4l2Controls v4l2ControlsCreateFromV4l2Fd(int fd) {
	V4l2Controls ctrls = {.fd = fd};
	arrayInit(&ctrls.controls, V4l2Control);

	struct v4l2_query_ext_ctrl qctrl = {0};
	for (int i = 0;; i++) {
		qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL | V4L2_CTRL_FLAG_NEXT_COMPOUND;
		if (0 > ioctl(fd, VIDIOC_QUERY_EXT_CTRL, &qctrl)) {
			LOGI("Total ext controls: %d", i);
			break;
		}

		// TODO filter out unsupported controls

		struct v4l2_ext_control ctrl_val = {
			.id = qctrl.id,
			.size = 0,
		};

		// Save the default value in case it's not readable, or reading resulted in error
		int64_t value = qctrl.default_value;

		if (!(qctrl.flags & V4L2_CTRL_FLAG_WRITE_ONLY) && !(qctrl.flags & V4L2_CTRL_FLAG_HAS_PAYLOAD)) {
			struct v4l2_ext_controls ctrl_vals = {
				.which = V4L2_CTRL_WHICH_CUR_VAL,
				.count = 1,
				.controls = &ctrl_val,
			};

			if (0 > ioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrl_vals)) {
				LOGE("ioctl(VIDIOC_G_EXT_CTRLS[.id=%d]) failed: %s %d",
					qctrl.id, strerror(errno), errno);
			} else {
				if (qctrl.type == V4L2_CTRL_TYPE_INTEGER64) {
					value = ctrl_val.value64;
				} else {
					value = ctrl_val.value;
				}
			}
		}

		LOGI("[fd=%d] ctrl_ext[%d]: id=(%d)%s type=%s name='%s' range=[%lld.+%llu.%lld] def=%lld cur=%d flags=%08x",
			fd, i,
			qctrl.id, v4l2CtrlIdName(qctrl.id),
			v4l2CtrlTypeName(qctrl.type),
			qctrl.name,
			qctrl.minimum, qctrl.step, qctrl.maximum,
			qctrl.default_value,
			ctrl_val.value,
			qctrl.flags
		);

		v4l2PrintControlFlags(qctrl.flags);

		if (qctrl.type == V4L2_CTRL_TYPE_MENU || qctrl.type == V4L2_CTRL_TYPE_INTEGER_MENU) {
			for (int j = qctrl.minimum; j <= qctrl.maximum; j++) {
				struct v4l2_querymenu menu = {
					.id = qctrl.id,
					.index = j,
				};

				if (0 > ioctl(fd, VIDIOC_QUERYMENU, &menu)) {
					LOGE("ioctl(VIDIOC_QUERYMENU[id=%d index=%d]) failed: %s (%d)",
						menu.id, menu.index, strerror(errno), errno);
					break;
				}

				if (qctrl.type == V4L2_CTRL_TYPE_MENU) {
					LOGI("  menuitem[%d]: %sname=%s", menu.index,
						((int)menu.index == ctrl_val.value) ? "-> " : "", menu.name);
				} else {
					LOGI("  menuitem[%d]: %svalue=%lld", menu.index,
						((int)menu.index == ctrl_val.value) ? "-> " : "", menu.value);
				}
			}
		}

		const V4l2Control control = {
			.query = qctrl,
			.value = value,
		};

		arrayAppend(&ctrls.controls, &control);
	}

	return ctrls;
}

void v4l2ControlsDestroy(V4l2Controls *controls) {
	if (!controls)
		return;

	arrayDestroy(&controls->controls);
}

V4l2Control *v4l2ControlGet(V4l2Controls *controls, uint32_t ctrl) {
	for (int i = 0; i < controls->controls.size; ++i) {
		V4l2Control *const c = arrayAt(&controls->controls, V4l2Control, i);
		if (c->query.id == ctrl)
			return c;
	}

	return NULL;
}

int v4l2ControlSet(V4l2Controls *controls, V4l2Control *ctrl, int64_t value) {
	if (!ctrl)
		return -EINVAL;

	struct v4l2_ext_control val = {
		.id = ctrl->query.id,
	};

	switch (ctrl->query.type) {
		case V4L2_CTRL_TYPE_INTEGER64:
			val.value64 = value;
			break;

		case V4L2_CTRL_TYPE_INTEGER:
		case V4L2_CTRL_TYPE_INTEGER_MENU:
		case V4L2_CTRL_TYPE_BOOLEAN:
		case V4L2_CTRL_TYPE_MENU:
			val.value = value;
			break;

		default:
			return -EPERM;
	}

	struct v4l2_ext_controls ctrls = {
		.which = V4L2_CTRL_WHICH_CUR_VAL,
		.count = 1,
		.controls = &val,
	};

	if (0 > ioctl(controls->fd, VIDIOC_S_EXT_CTRLS, &ctrls)) {
		const int error = errno;
		LOGE("ioctl(VIDIOC_S_EXT_CTRLS[.id=%d]) failed: %s %d)", val.id, strerror(error), error);
		return -error;
	}

	ctrl->value = value;

	return 0;
}

int v4l2ControlSetById(V4l2Controls *controls, uint32_t ctrl_id, int64_t value) {
	V4l2Control *const ctrl = v4l2ControlGet(controls, ctrl_id);

	if (!ctrl)
		return -ENOENT;

	return v4l2ControlSet(controls, ctrl, value);
}

V4l2Controls v4l2ControlsCreate(void);
void v4l2ControlsAppend(V4l2Controls *ctrls, const V4l2Controls *appendage);
