#pragma once

struct Node;

struct Node *piOpenCamera(void);

struct Node *piOpenISP(void);

enum PiEncoderType {
	PiEncoderMJPEG,
	PiEncoderH264,
	PiEncoderJPEG,
};

struct Node *piOpenEncoder(enum PiEncoderType type);
