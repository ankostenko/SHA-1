#pragma once

#define ArrayCount(buf) (sizeof(buf) / sizeof((buf)[0]))
#define BitsToBytes(bits) ((bits) / 8)

// This is really bad abstraction
inline void WriteBytesToBuffer(char *dst, const wchar_t *src, int numberOfBytes) {
	memcpy(dst, src, numberOfBytes);
	dst += numberOfBytes;
}

inline void WriteBytesToBuffer(char *dst, char byte) {
	*dst |= byte;
	dst += 1;
}