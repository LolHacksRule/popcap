#include <singlebytecodec.h>

static const int iso8859_4_decoding_table[256] = {
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
	0x80,
	0x81,
	0x82,
	0x83,
	0x84,
	0x85,
	0x86,
	0x87,
	0x88,
	0x89,
	0x8a,
	0x8b,
	0x8c,
	0x8d,
	0x8e,
	0x8f,
	0x90,
	0x91,
	0x92,
	0x93,
	0x94,
	0x95,
	0x96,
	0x97,
	0x98,
	0x99,
	0x9a,
	0x9b,
	0x9c,
	0x9d,
	0x9e,
	0x9f,
	0xa0,
	0x104,
	0x138,
	0x156,
	0xa4,
	0x128,
	0x13b,
	0xa7,
	0xa8,
	0x160,
	0x112,
	0x122,
	0x166,
	0xad,
	0x17d,
	0xaf,
	0xb0,
	0x105,
	0x2db,
	0x157,
	0xb4,
	0x129,
	0x13c,
	0x2c7,
	0xb8,
	0x161,
	0x113,
	0x123,
	0x167,
	0x14a,
	0x17e,
	0x14b,
	0x100,
	0xc1,
	0xc2,
	0xc3,
	0xc4,
	0xc5,
	0xc6,
	0x12e,
	0x10c,
	0xc9,
	0x118,
	0xcb,
	0x116,
	0xcd,
	0xce,
	0x12a,
	0x110,
	0x145,
	0x14c,
	0x136,
	0xd4,
	0xd5,
	0xd6,
	0xd7,
	0xd8,
	0x172,
	0xda,
	0xdb,
	0xdc,
	0x168,
	0x16a,
	0xdf,
	0x101,
	0xe1,
	0xe2,
	0xe3,
	0xe4,
	0xe5,
	0xe6,
	0x12f,
	0x10d,
	0xe9,
	0x119,
	0xeb,
	0x117,
	0xed,
	0xee,
	0x12b,
	0x111,
	0x146,
	0x14d,
	0x137,
	0xf4,
	0xf5,
	0xf6,
	0xf7,
	0xf8,
	0x173,
	0xfa,
	0xfb,
	0xfc,
	0x169,
	0x16b,
	0x2d9,
};

static const struct encoding_map iso8859_4_encoding_map[] = {
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
	{ 0x80, 0x80 },
	{ 0x81, 0x81 },
	{ 0x82, 0x82 },
	{ 0x83, 0x83 },
	{ 0x84, 0x84 },
	{ 0x85, 0x85 },
	{ 0x86, 0x86 },
	{ 0x87, 0x87 },
	{ 0x88, 0x88 },
	{ 0x89, 0x89 },
	{ 0x8a, 0x8a },
	{ 0x8b, 0x8b },
	{ 0x8c, 0x8c },
	{ 0x8d, 0x8d },
	{ 0x8e, 0x8e },
	{ 0x8f, 0x8f },
	{ 0x90, 0x90 },
	{ 0x91, 0x91 },
	{ 0x92, 0x92 },
	{ 0x93, 0x93 },
	{ 0x94, 0x94 },
	{ 0x95, 0x95 },
	{ 0x96, 0x96 },
	{ 0x97, 0x97 },
	{ 0x98, 0x98 },
	{ 0x99, 0x99 },
	{ 0x9a, 0x9a },
	{ 0x9b, 0x9b },
	{ 0x9c, 0x9c },
	{ 0x9d, 0x9d },
	{ 0x9e, 0x9e },
	{ 0x9f, 0x9f },
	{ 0xa0, 0xa0 },
	{ 0xa4, 0xa4 },
	{ 0xa7, 0xa7 },
	{ 0xa8, 0xa8 },
	{ 0xad, 0xad },
	{ 0xaf, 0xaf },
	{ 0xb0, 0xb0 },
	{ 0xb4, 0xb4 },
	{ 0xb8, 0xb8 },
	{ 0xc1, 0xc1 },
	{ 0xc2, 0xc2 },
	{ 0xc3, 0xc3 },
	{ 0xc4, 0xc4 },
	{ 0xc5, 0xc5 },
	{ 0xc6, 0xc6 },
	{ 0xc9, 0xc9 },
	{ 0xcb, 0xcb },
	{ 0xcd, 0xcd },
	{ 0xce, 0xce },
	{ 0xd4, 0xd4 },
	{ 0xd5, 0xd5 },
	{ 0xd6, 0xd6 },
	{ 0xd7, 0xd7 },
	{ 0xd8, 0xd8 },
	{ 0xda, 0xda },
	{ 0xdb, 0xdb },
	{ 0xdc, 0xdc },
	{ 0xdf, 0xdf },
	{ 0xe1, 0xe1 },
	{ 0xe2, 0xe2 },
	{ 0xe3, 0xe3 },
	{ 0xe4, 0xe4 },
	{ 0xe5, 0xe5 },
	{ 0xe6, 0xe6 },
	{ 0xe9, 0xe9 },
	{ 0xeb, 0xeb },
	{ 0xed, 0xed },
	{ 0xee, 0xee },
	{ 0xf4, 0xf4 },
	{ 0xf5, 0xf5 },
	{ 0xf6, 0xf6 },
	{ 0xf7, 0xf7 },
	{ 0xf8, 0xf8 },
	{ 0xfa, 0xfa },
	{ 0xfb, 0xfb },
	{ 0xfc, 0xfc },
	{ 0x100, 0xc0 },
	{ 0x101, 0xe0 },
	{ 0x104, 0xa1 },
	{ 0x105, 0xb1 },
	{ 0x10c, 0xc8 },
	{ 0x10d, 0xe8 },
	{ 0x110, 0xd0 },
	{ 0x111, 0xf0 },
	{ 0x112, 0xaa },
	{ 0x113, 0xba },
	{ 0x116, 0xcc },
	{ 0x117, 0xec },
	{ 0x118, 0xca },
	{ 0x119, 0xea },
	{ 0x122, 0xab },
	{ 0x123, 0xbb },
	{ 0x128, 0xa5 },
	{ 0x129, 0xb5 },
	{ 0x12a, 0xcf },
	{ 0x12b, 0xef },
	{ 0x12e, 0xc7 },
	{ 0x12f, 0xe7 },
	{ 0x136, 0xd3 },
	{ 0x137, 0xf3 },
	{ 0x138, 0xa2 },
	{ 0x13b, 0xa6 },
	{ 0x13c, 0xb6 },
	{ 0x145, 0xd1 },
	{ 0x146, 0xf1 },
	{ 0x14a, 0xbd },
	{ 0x14b, 0xbf },
	{ 0x14c, 0xd2 },
	{ 0x14d, 0xf2 },
	{ 0x156, 0xa3 },
	{ 0x157, 0xb3 },
	{ 0x160, 0xa9 },
	{ 0x161, 0xb9 },
	{ 0x166, 0xac },
	{ 0x167, 0xbc },
	{ 0x168, 0xdd },
	{ 0x169, 0xfd },
	{ 0x16a, 0xde },
	{ 0x16b, 0xfe },
	{ 0x172, 0xd9 },
	{ 0x173, 0xf9 },
	{ 0x17d, 0xae },
	{ 0x17e, 0xbe },
	{ 0x2c7, 0xb7 },
	{ 0x2d9, 0xff },
	{ 0x2db, 0xb2 },
};

SingleByteCodecState __uniconv_iso8859_4_state = {
	"iso8859_4", iso8859_4_decoding_table, 256, iso8859_4_encoding_map, 256
};

