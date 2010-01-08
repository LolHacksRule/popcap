#include <singlebytecodec.h>

static const int cp866_decoding_table[256] = {
	0x0,
	0x1,
	0x2,
	0x3,
	0x4,
	0x5,
	0x6,
	0x7,
	0x8,
	0x9,
	0xa,
	0xb,
	0xc,
	0xd,
	0xe,
	0xf,
	0x10,
	0x11,
	0x12,
	0x13,
	0x14,
	0x15,
	0x16,
	0x17,
	0x18,
	0x19,
	0x1a,
	0x1b,
	0x1c,
	0x1d,
	0x1e,
	0x1f,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
	0x26,
	0x27,
	0x28,
	0x29,
	0x2a,
	0x2b,
	0x2c,
	0x2d,
	0x2e,
	0x2f,
	0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	0x38,
	0x39,
	0x3a,
	0x3b,
	0x3c,
	0x3d,
	0x3e,
	0x3f,
	0x40,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x46,
	0x47,
	0x48,
	0x49,
	0x4a,
	0x4b,
	0x4c,
	0x4d,
	0x4e,
	0x4f,
	0x50,
	0x51,
	0x52,
	0x53,
	0x54,
	0x55,
	0x56,
	0x57,
	0x58,
	0x59,
	0x5a,
	0x5b,
	0x5c,
	0x5d,
	0x5e,
	0x5f,
	0x60,
	0x61,
	0x62,
	0x63,
	0x64,
	0x65,
	0x66,
	0x67,
	0x68,
	0x69,
	0x6a,
	0x6b,
	0x6c,
	0x6d,
	0x6e,
	0x6f,
	0x70,
	0x71,
	0x72,
	0x73,
	0x74,
	0x75,
	0x76,
	0x77,
	0x78,
	0x79,
	0x7a,
	0x7b,
	0x7c,
	0x7d,
	0x7e,
	0x7f,
	0x410,
	0x411,
	0x412,
	0x413,
	0x414,
	0x415,
	0x416,
	0x417,
	0x418,
	0x419,
	0x41a,
	0x41b,
	0x41c,
	0x41d,
	0x41e,
	0x41f,
	0x420,
	0x421,
	0x422,
	0x423,
	0x424,
	0x425,
	0x426,
	0x427,
	0x428,
	0x429,
	0x42a,
	0x42b,
	0x42c,
	0x42d,
	0x42e,
	0x42f,
	0x430,
	0x431,
	0x432,
	0x433,
	0x434,
	0x435,
	0x436,
	0x437,
	0x438,
	0x439,
	0x43a,
	0x43b,
	0x43c,
	0x43d,
	0x43e,
	0x43f,
	0x2591,
	0x2592,
	0x2593,
	0x2502,
	0x2524,
	0x2561,
	0x2562,
	0x2556,
	0x2555,
	0x2563,
	0x2551,
	0x2557,
	0x255d,
	0x255c,
	0x255b,
	0x2510,
	0x2514,
	0x2534,
	0x252c,
	0x251c,
	0x2500,
	0x253c,
	0x255e,
	0x255f,
	0x255a,
	0x2554,
	0x2569,
	0x2566,
	0x2560,
	0x2550,
	0x256c,
	0x2567,
	0x2568,
	0x2564,
	0x2565,
	0x2559,
	0x2558,
	0x2552,
	0x2553,
	0x256b,
	0x256a,
	0x2518,
	0x250c,
	0x2588,
	0x2584,
	0x258c,
	0x2590,
	0x2580,
	0x440,
	0x441,
	0x442,
	0x443,
	0x444,
	0x445,
	0x446,
	0x447,
	0x448,
	0x449,
	0x44a,
	0x44b,
	0x44c,
	0x44d,
	0x44e,
	0x44f,
	0x401,
	0x451,
	0x404,
	0x454,
	0x407,
	0x457,
	0x40e,
	0x45e,
	0xb0,
	0x2219,
	0xb7,
	0x221a,
	0x2116,
	0xa4,
	0x25a0,
	0xa0,
};

static const struct encoding_map cp866_encoding_map[] = {
	{ 0x0, 0x0 },
	{ 0x1, 0x1 },
	{ 0x2, 0x2 },
	{ 0x3, 0x3 },
	{ 0x4, 0x4 },
	{ 0x5, 0x5 },
	{ 0x6, 0x6 },
	{ 0x7, 0x7 },
	{ 0x8, 0x8 },
	{ 0x9, 0x9 },
	{ 0xa, 0xa },
	{ 0xb, 0xb },
	{ 0xc, 0xc },
	{ 0xd, 0xd },
	{ 0xe, 0xe },
	{ 0xf, 0xf },
	{ 0x10, 0x10 },
	{ 0x11, 0x11 },
	{ 0x12, 0x12 },
	{ 0x13, 0x13 },
	{ 0x14, 0x14 },
	{ 0x15, 0x15 },
	{ 0x16, 0x16 },
	{ 0x17, 0x17 },
	{ 0x18, 0x18 },
	{ 0x19, 0x19 },
	{ 0x1a, 0x1a },
	{ 0x1b, 0x1b },
	{ 0x1c, 0x1c },
	{ 0x1d, 0x1d },
	{ 0x1e, 0x1e },
	{ 0x1f, 0x1f },
	{ 0x20, 0x20 },
	{ 0x21, 0x21 },
	{ 0x22, 0x22 },
	{ 0x23, 0x23 },
	{ 0x24, 0x24 },
	{ 0x25, 0x25 },
	{ 0x26, 0x26 },
	{ 0x27, 0x27 },
	{ 0x28, 0x28 },
	{ 0x29, 0x29 },
	{ 0x2a, 0x2a },
	{ 0x2b, 0x2b },
	{ 0x2c, 0x2c },
	{ 0x2d, 0x2d },
	{ 0x2e, 0x2e },
	{ 0x2f, 0x2f },
	{ 0x30, 0x30 },
	{ 0x31, 0x31 },
	{ 0x32, 0x32 },
	{ 0x33, 0x33 },
	{ 0x34, 0x34 },
	{ 0x35, 0x35 },
	{ 0x36, 0x36 },
	{ 0x37, 0x37 },
	{ 0x38, 0x38 },
	{ 0x39, 0x39 },
	{ 0x3a, 0x3a },
	{ 0x3b, 0x3b },
	{ 0x3c, 0x3c },
	{ 0x3d, 0x3d },
	{ 0x3e, 0x3e },
	{ 0x3f, 0x3f },
	{ 0x40, 0x40 },
	{ 0x41, 0x41 },
	{ 0x42, 0x42 },
	{ 0x43, 0x43 },
	{ 0x44, 0x44 },
	{ 0x45, 0x45 },
	{ 0x46, 0x46 },
	{ 0x47, 0x47 },
	{ 0x48, 0x48 },
	{ 0x49, 0x49 },
	{ 0x4a, 0x4a },
	{ 0x4b, 0x4b },
	{ 0x4c, 0x4c },
	{ 0x4d, 0x4d },
	{ 0x4e, 0x4e },
	{ 0x4f, 0x4f },
	{ 0x50, 0x50 },
	{ 0x51, 0x51 },
	{ 0x52, 0x52 },
	{ 0x53, 0x53 },
	{ 0x54, 0x54 },
	{ 0x55, 0x55 },
	{ 0x56, 0x56 },
	{ 0x57, 0x57 },
	{ 0x58, 0x58 },
	{ 0x59, 0x59 },
	{ 0x5a, 0x5a },
	{ 0x5b, 0x5b },
	{ 0x5c, 0x5c },
	{ 0x5d, 0x5d },
	{ 0x5e, 0x5e },
	{ 0x5f, 0x5f },
	{ 0x60, 0x60 },
	{ 0x61, 0x61 },
	{ 0x62, 0x62 },
	{ 0x63, 0x63 },
	{ 0x64, 0x64 },
	{ 0x65, 0x65 },
	{ 0x66, 0x66 },
	{ 0x67, 0x67 },
	{ 0x68, 0x68 },
	{ 0x69, 0x69 },
	{ 0x6a, 0x6a },
	{ 0x6b, 0x6b },
	{ 0x6c, 0x6c },
	{ 0x6d, 0x6d },
	{ 0x6e, 0x6e },
	{ 0x6f, 0x6f },
	{ 0x70, 0x70 },
	{ 0x71, 0x71 },
	{ 0x72, 0x72 },
	{ 0x73, 0x73 },
	{ 0x74, 0x74 },
	{ 0x75, 0x75 },
	{ 0x76, 0x76 },
	{ 0x77, 0x77 },
	{ 0x78, 0x78 },
	{ 0x79, 0x79 },
	{ 0x7a, 0x7a },
	{ 0x7b, 0x7b },
	{ 0x7c, 0x7c },
	{ 0x7d, 0x7d },
	{ 0x7e, 0x7e },
	{ 0x7f, 0x7f },
	{ 0xa0, 0xff },
	{ 0xa4, 0xfd },
	{ 0xb0, 0xf8 },
	{ 0xb7, 0xfa },
	{ 0x401, 0xf0 },
	{ 0x404, 0xf2 },
	{ 0x407, 0xf4 },
	{ 0x40e, 0xf6 },
	{ 0x410, 0x80 },
	{ 0x411, 0x81 },
	{ 0x412, 0x82 },
	{ 0x413, 0x83 },
	{ 0x414, 0x84 },
	{ 0x415, 0x85 },
	{ 0x416, 0x86 },
	{ 0x417, 0x87 },
	{ 0x418, 0x88 },
	{ 0x419, 0x89 },
	{ 0x41a, 0x8a },
	{ 0x41b, 0x8b },
	{ 0x41c, 0x8c },
	{ 0x41d, 0x8d },
	{ 0x41e, 0x8e },
	{ 0x41f, 0x8f },
	{ 0x420, 0x90 },
	{ 0x421, 0x91 },
	{ 0x422, 0x92 },
	{ 0x423, 0x93 },
	{ 0x424, 0x94 },
	{ 0x425, 0x95 },
	{ 0x426, 0x96 },
	{ 0x427, 0x97 },
	{ 0x428, 0x98 },
	{ 0x429, 0x99 },
	{ 0x42a, 0x9a },
	{ 0x42b, 0x9b },
	{ 0x42c, 0x9c },
	{ 0x42d, 0x9d },
	{ 0x42e, 0x9e },
	{ 0x42f, 0x9f },
	{ 0x430, 0xa0 },
	{ 0x431, 0xa1 },
	{ 0x432, 0xa2 },
	{ 0x433, 0xa3 },
	{ 0x434, 0xa4 },
	{ 0x435, 0xa5 },
	{ 0x436, 0xa6 },
	{ 0x437, 0xa7 },
	{ 0x438, 0xa8 },
	{ 0x439, 0xa9 },
	{ 0x43a, 0xaa },
	{ 0x43b, 0xab },
	{ 0x43c, 0xac },
	{ 0x43d, 0xad },
	{ 0x43e, 0xae },
	{ 0x43f, 0xaf },
	{ 0x440, 0xe0 },
	{ 0x441, 0xe1 },
	{ 0x442, 0xe2 },
	{ 0x443, 0xe3 },
	{ 0x444, 0xe4 },
	{ 0x445, 0xe5 },
	{ 0x446, 0xe6 },
	{ 0x447, 0xe7 },
	{ 0x448, 0xe8 },
	{ 0x449, 0xe9 },
	{ 0x44a, 0xea },
	{ 0x44b, 0xeb },
	{ 0x44c, 0xec },
	{ 0x44d, 0xed },
	{ 0x44e, 0xee },
	{ 0x44f, 0xef },
	{ 0x451, 0xf1 },
	{ 0x454, 0xf3 },
	{ 0x457, 0xf5 },
	{ 0x45e, 0xf7 },
	{ 0x2116, 0xfc },
	{ 0x2219, 0xf9 },
	{ 0x221a, 0xfb },
	{ 0x2500, 0xc4 },
	{ 0x2502, 0xb3 },
	{ 0x250c, 0xda },
	{ 0x2510, 0xbf },
	{ 0x2514, 0xc0 },
	{ 0x2518, 0xd9 },
	{ 0x251c, 0xc3 },
	{ 0x2524, 0xb4 },
	{ 0x252c, 0xc2 },
	{ 0x2534, 0xc1 },
	{ 0x253c, 0xc5 },
	{ 0x2550, 0xcd },
	{ 0x2551, 0xba },
	{ 0x2552, 0xd5 },
	{ 0x2553, 0xd6 },
	{ 0x2554, 0xc9 },
	{ 0x2555, 0xb8 },
	{ 0x2556, 0xb7 },
	{ 0x2557, 0xbb },
	{ 0x2558, 0xd4 },
	{ 0x2559, 0xd3 },
	{ 0x255a, 0xc8 },
	{ 0x255b, 0xbe },
	{ 0x255c, 0xbd },
	{ 0x255d, 0xbc },
	{ 0x255e, 0xc6 },
	{ 0x255f, 0xc7 },
	{ 0x2560, 0xcc },
	{ 0x2561, 0xb5 },
	{ 0x2562, 0xb6 },
	{ 0x2563, 0xb9 },
	{ 0x2564, 0xd1 },
	{ 0x2565, 0xd2 },
	{ 0x2566, 0xcb },
	{ 0x2567, 0xcf },
	{ 0x2568, 0xd0 },
	{ 0x2569, 0xca },
	{ 0x256a, 0xd8 },
	{ 0x256b, 0xd7 },
	{ 0x256c, 0xce },
	{ 0x2580, 0xdf },
	{ 0x2584, 0xdc },
	{ 0x2588, 0xdb },
	{ 0x258c, 0xdd },
	{ 0x2590, 0xde },
	{ 0x2591, 0xb0 },
	{ 0x2592, 0xb1 },
	{ 0x2593, 0xb2 },
	{ 0x25a0, 0xfe },
};

SingleByteCodecState __uniconv_cp866_state = {
	"cp866", cp866_decoding_table, 256, cp866_encoding_map, 256
};
