#pragma once

#define ArrayCount(buf) (sizeof(buf) / sizeof((buf)[0]))
#define BitsToBytes(bits) ((bits) / 8)
