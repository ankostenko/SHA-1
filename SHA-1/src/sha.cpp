#define DEBUG_BUILD 0

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <iostream>
#include <memory.h>
#include <stdint.h>

#include <chrono>

#define S(n, x) (((x) << n) | ((x) >> (32 - n)))

struct Message {
	uint8_t data[64];
	uint64_t size;
	int currentSize;
};

struct Wt {
	union {
		struct {
			uint8_t b1;
			uint8_t b2;
			uint8_t b3;
			uint8_t b4;
		};
		uint32_t wNumber;
	};
};

uint32_t H0 = 0x67452301;
uint32_t H1 = 0xEFCDAB89;
uint32_t H2 = 0x98BADCFE;
uint32_t H3 = 0x10325476;
uint32_t H4 = 0xC3D2E1F0;


Wt W[80];

Message msg;

bool ReadFile(FILE *fileHandle) {
	// Reset bufferWrite to read new message
	uint8_t *bufferWrite = msg.data;
	msg.currentSize = 0;

	while (1) {
		int dataByte = fgetc(fileHandle);
	
		if (dataByte == EOF) {
			return true;
		}
	
		*bufferWrite = dataByte;
		bufferWrite += 1;
		msg.size += 1;
		msg.currentSize += 1;
		
		// [message] [1000 0000] [length of the message]
		// (n bytes)    (8bits)          (64bits)
		if (msg.currentSize == 64) {
			return false;
		}
	}

	return true;
}

void AppendLengthToMessage() {
	// Maximum message size is 2^64
	// Is it correct?
	uint64_t messageLength = (msg.size * 8);
	for (int i = 0; i < 8; i++) {
		msg.data[63 - i] = (uint8_t)(messageLength >> (8 * i));
	}
}

void AddZeroPadding(uint8_t *lastWrite) {
	// Evaluate amount of bytes we need to zero
	uint64_t bytesToZero = 64 - msg.currentSize;
	for (int i = 0; i < bytesToZero; i++) {
		*lastWrite = 0x00;
		lastWrite++;
	}
}

void AddOnePadding(uint8_t *lastWrite) {
	// Add 1 as a byte
	*(msg.data + msg.currentSize) = 0x80; // 0b1000_0000
	lastWrite++;
}

int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Input path to a file\n");
#if DEBUG_BUILD
		std::cin.get();
#endif
		return 0;
	}

	FILE *fileHandle = fopen(argv[1], "rb");
	if (!fileHandle) {
		printf("Cannot open file!\n");
	
#if DEBUG_BUILD
		std::cin.get();
#endif
		return 0;
	}

	bool isFullyRead = false;
	uint32_t hash[5];
	while (1) {
		memset(msg.data, 0, 64);
		/*
			1. Read up to 64 bytes from the file and write data to the buffer
			2. Return from the function and process read data
			3. Repeat until all data is read
			4. Process the last chunk of data
			5. Call the function again and return 0 signaling that nothing left to read
		*/
		isFullyRead = ReadFile(fileHandle);

		// We can put 1 and a length of the message
		if ((msg.currentSize < (64 - 9)) && isFullyRead) {
			AddOnePadding(msg.data + msg.currentSize);
			AppendLengthToMessage();
		}

		// We can put only 1 
		if ((msg.currentSize != 64 && msg.currentSize >= (64 - 8)) && isFullyRead) {
			AddOnePadding(msg.data + msg.currentSize);
		}
process:

		// Process chunk of data
		for (int i = 0; i < 16; i++) {
			W[i] = { msg.data[i * 4 + 3], msg.data[i * 4 + 2], msg.data[i * 4 + 1], msg.data[i * 4] };
		}

		for (int i = 16; i < 80; i++) {
			W[i].wNumber = (uint32_t)S(1, (W[i - 3].wNumber ^ W[i - 8].wNumber ^ W[i - 14].wNumber ^ W[i - 16].wNumber));
		}

		uint32_t a = H0;
		uint32_t b = H1;
		uint32_t c = H2;
		uint32_t d = H3;
		uint32_t e = H4;

		// Main loop
		uint32_t f, k;

		for (int i = 0; i < 80; i++) {
			if (i >= 0 && i < 20) {
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			} else if (i >= 20 && i < 40) {
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			} else if (i >= 40 && i < 60) {
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			} else if (i >= 60 && i < 80) {
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}

			uint32_t temp = S(5, a) + f + e + k + W[i].wNumber;
			e = d;
			d = c;
			c = S(30, b);
			b = a;
			a = temp;
		}

		H0 = H0 + a;
		H1 = H1 + b;
		H2 = H2 + c;
		H3 = H3 + d;
		H4 = H4 + e;

		hash[0] = H0;
		hash[1] = H1;
		hash[2] = H2;
		hash[3] = H3;
		hash[4] = H4;

		if (isFullyRead) {
			// We put only 1 and the size goes to the next chunk
			if ((msg.currentSize != 64 && msg.currentSize >= (64 - 8)) && isFullyRead) {
				msg.currentSize = 0;
				AddZeroPadding(msg.data + msg.currentSize);
				AppendLengthToMessage();
				goto process;
			}
			break;
		}
	}

	printf("%x %x %x %x %x\n", hash[0], hash[1], hash[2], hash[3], hash[4]);
	
#if DEBUG_BUILD
	std::cin.get();
#endif
	
	return 0;
}