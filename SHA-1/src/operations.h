#pragma once

#define ArrayCount(buf) (sizeof(buf) / sizeof((buf)[0]))
#define BitsToBytes(bits) ((bits) / 8)


struct Message {
	uint8_t data[64];
	uint64_t size;
	int currentSize;
};
