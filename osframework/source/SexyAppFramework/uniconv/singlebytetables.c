#include <singlebytecodec.h>

extern SingleByteCodecState __uniconv_ascii_state;
extern SingleByteCodecState __uniconv_cp037_state;
extern SingleByteCodecState __uniconv_cp1006_state;
extern SingleByteCodecState __uniconv_cp1026_state;
extern SingleByteCodecState __uniconv_cp1140_state;
extern SingleByteCodecState __uniconv_cp1250_state;
extern SingleByteCodecState __uniconv_cp1251_state;
extern SingleByteCodecState __uniconv_cp1252_state;
extern SingleByteCodecState __uniconv_cp1253_state;
extern SingleByteCodecState __uniconv_cp1254_state;
extern SingleByteCodecState __uniconv_cp1255_state;
extern SingleByteCodecState __uniconv_cp1256_state;
extern SingleByteCodecState __uniconv_cp1257_state;
extern SingleByteCodecState __uniconv_cp1258_state;
extern SingleByteCodecState __uniconv_cp424_state;
extern SingleByteCodecState __uniconv_cp437_state;
extern SingleByteCodecState __uniconv_cp500_state;
extern SingleByteCodecState __uniconv_cp737_state;
extern SingleByteCodecState __uniconv_cp775_state;
extern SingleByteCodecState __uniconv_cp850_state;
extern SingleByteCodecState __uniconv_cp852_state;
extern SingleByteCodecState __uniconv_cp855_state;
extern SingleByteCodecState __uniconv_cp856_state;
extern SingleByteCodecState __uniconv_cp857_state;
extern SingleByteCodecState __uniconv_cp860_state;
extern SingleByteCodecState __uniconv_cp861_state;
extern SingleByteCodecState __uniconv_cp862_state;
extern SingleByteCodecState __uniconv_cp863_state;
extern SingleByteCodecState __uniconv_cp864_state;
extern SingleByteCodecState __uniconv_cp865_state;
extern SingleByteCodecState __uniconv_cp866_state;
extern SingleByteCodecState __uniconv_cp869_state;
extern SingleByteCodecState __uniconv_cp874_state;
extern SingleByteCodecState __uniconv_cp875_state;
extern SingleByteCodecState __uniconv_iso8859_1_state;
extern SingleByteCodecState __uniconv_iso8859_10_state;
extern SingleByteCodecState __uniconv_iso8859_11_state;
extern SingleByteCodecState __uniconv_iso8859_13_state;
extern SingleByteCodecState __uniconv_iso8859_14_state;
extern SingleByteCodecState __uniconv_iso8859_15_state;
extern SingleByteCodecState __uniconv_iso8859_16_state;
extern SingleByteCodecState __uniconv_iso8859_2_state;
extern SingleByteCodecState __uniconv_iso8859_3_state;
extern SingleByteCodecState __uniconv_iso8859_4_state;
extern SingleByteCodecState __uniconv_iso8859_5_state;
extern SingleByteCodecState __uniconv_iso8859_6_state;
extern SingleByteCodecState __uniconv_iso8859_7_state;
extern SingleByteCodecState __uniconv_iso8859_8_state;
extern SingleByteCodecState __uniconv_iso8859_9_state;

static SingleByteCodecState *singlebytecodecs[] = {
	&__uniconv_ascii_state,
	&__uniconv_cp037_state,
	&__uniconv_cp1006_state,
	&__uniconv_cp1026_state,
	&__uniconv_cp1140_state,
	&__uniconv_cp1250_state,
	&__uniconv_cp1251_state,
	&__uniconv_cp1252_state,
	&__uniconv_cp1253_state,
	&__uniconv_cp1254_state,
	&__uniconv_cp1255_state,
	&__uniconv_cp1256_state,
	&__uniconv_cp1257_state,
	&__uniconv_cp1258_state,
	&__uniconv_cp424_state,
	&__uniconv_cp437_state,
	&__uniconv_cp500_state,
	&__uniconv_cp737_state,
	&__uniconv_cp775_state,
	&__uniconv_cp850_state,
	&__uniconv_cp852_state,
	&__uniconv_cp855_state,
	&__uniconv_cp856_state,
	&__uniconv_cp857_state,
	&__uniconv_cp860_state,
	&__uniconv_cp861_state,
	&__uniconv_cp862_state,
	&__uniconv_cp863_state,
	&__uniconv_cp864_state,
	&__uniconv_cp865_state,
	&__uniconv_cp866_state,
	&__uniconv_cp869_state,
	&__uniconv_cp874_state,
	&__uniconv_cp875_state,
	&__uniconv_iso8859_1_state,
	&__uniconv_iso8859_10_state,
	&__uniconv_iso8859_11_state,
	&__uniconv_iso8859_13_state,
	&__uniconv_iso8859_14_state,
	&__uniconv_iso8859_15_state,
	&__uniconv_iso8859_16_state,
	&__uniconv_iso8859_2_state,
	&__uniconv_iso8859_3_state,
	&__uniconv_iso8859_4_state,
	&__uniconv_iso8859_5_state,
	&__uniconv_iso8859_6_state,
	&__uniconv_iso8859_7_state,
	&__uniconv_iso8859_8_state,
	&__uniconv_iso8859_9_state,
	NULL,
};

SingleByteCodecState** __uniconv_get_single_byte_codecs()
{
	return singlebytecodecs;
}

