#include <stdio.h>
#include <iostream>
#include <memory.h>
#include <stdint.h>

#include "operations.h"

// 512-bit chunk
uint8_t chunk[64];
uint8_t *lastWrite = chunk;

// To byte characters is used
const char message[] = "Hash function is working!";

uint32_t H0 = 0x67452301;
uint32_t H1 = 0xEFCDAB89;
uint32_t H2 = 0x98BADCFE;
uint32_t H3 = 0x10325476;
uint32_t H4 = 0xC3D2E1F0;

// 80 32-bit numbers
uint8_t W[80][4];

struct Wt{
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

Wt W2[80];

void AddPaddingToMessage() {
	// DEBUG
	memset(chunk, 0xFFFF, ArrayCount(chunk));

	// TODO: DEBUG
	memcpy(lastWrite, message, ArrayCount(message) - 1);
	lastWrite += ArrayCount(message) - 1;
	
	// Appending with 1
	*lastWrite = (char)0b10000000;
	lastWrite += 1;

	// Zero message until 448 bit = 56 bytes
	int toWriteByte = (chunk + 56) - lastWrite;
	for (int i = 0; i < toWriteByte; i++) {
		*lastWrite = 0x00;
		lastWrite += 1;
	}

	// Append a length of the message
	// TODO: check size of the message
	uint64_t messageSize = (ArrayCount(message) - 1) * 8;
	// I'm gonna take each byte of the size and OR them with each byte of the chunk from right to left
	for (int i = 0; i < 8; i++) {
		chunk[63 - i] = (uint8_t)(messageSize >> 8 * i);
	}
}

#define S(n, x) (((x) << n) | ((x) >> (32 - n)))
int main() {
	AddPaddingToMessage();

	// For each chunk
	// ===================================
	


	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			W[i][j] = chunk[i * 4 + j];
		}
	}

	for (int i = 0; i < 16; i++) {
		W2[i] = { chunk[i * 4 + 3], chunk[i * 4 + 2], chunk[i * 4 + 1], chunk[i * 4] };
	}

	for (int i = 16; i < 80; i++) {
		W2[i].wNumber = (uint32_t)S(1, (W2[i - 3].wNumber ^ W2[i - 8].wNumber ^ W2[i - 14].wNumber ^ W2[i - 16].wNumber));
	}

	for (int i = 16; i < 80; i++) {
		for (int j = 0; j < 4; j++) {
			W[i][j] = ((uint32_t)W[i - 3][j] ^ (uint32_t)W[i - 8][j] ^ (uint32_t)W[i - 14][j] ^ (uint32_t)W[i - 16][j]);
		}
		*W[i] = (uint32_t)S(1, (uint32_t)*W[i]);
	}

	uint32_t a = H0;
	uint32_t b = H1;
	uint32_t c = H2;
	uint32_t d = H3;
	uint32_t e = H4;

	uint32_t hash[5];

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

		
		auto res = (W[i][0] << 24) | (W[i][1] << 16) | (W[i][2] << 8) | (W[i][3]);
		auto res2 = (W2[i].wNumber);
		uint32_t temp = S(5, a) + f + e + k + (uint32_t)res;
		uint32_t temp2 = S(5, a) + f + e + k + (uint32_t)res2;
		e = d;
		d = c;
		c = S(30, b);
		b = a;
		a = temp2;
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
	// ===================================

	printf("%x %x %x %x %x", hash[0], hash[1], hash[2], hash[3], hash[4]);
	std::cin.get();
	return 0;
}