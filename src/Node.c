#include "Node.h"

#include "device.h"
#include "common.h"

#include <string.h>

int nodeStart(Node *node) {
	LOGI("%s: node=%s", __func__, node->name);
	if (node->output) {
		const int status = deviceStreamStart(node->output);
		if (0 != status) {
			LOGE("Unable to start %s:output: %s(%d)", node->name, strerror(status), status);
			return status;
		}
	}

	if (node->input) {
		const int status = deviceStreamStart(node->input);
		if (0 != status) {
			LOGE("Unable to start %s:input: %s(%d)", node->name, strerror(status), status);
			if (node->output) {
				deviceStreamStop(node->output);
			}
			return status;
		}
	}

	return 0;
}

int nodeStop(Node *node) {
	LOGI("%s: node=%s", __func__, node->name);
	int status = 0;
	if (node->output) {
		status = deviceStreamStop(node->output);
		if (0 != status) {
			LOGE("Unable to stop %s:output: %s(%d)", node->name, strerror(status), status);
		}
	}

	const int output_status = status;
	if (node->input) {
		status = deviceStreamStop(node->input);
		if (0 != status) {
			LOGE("Unable to stop %s:input: %s(%d)", node->name, strerror(status), status);
		}
	}

	return output_status ? output_status : status;
}
