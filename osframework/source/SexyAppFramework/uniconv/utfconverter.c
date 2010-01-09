#include "converter.h"
#include "utfconverter.h"
#include "uniconv.h"
#include "unicode.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define FLAG_USE_BOM_ENDIAN  (1 << 0)
#define FLAG_DONE_BOM_ENDIAN (1 << 1)
struct utfconverter {
    struct converter base;

    unsigned int flags;
    int little_endian;
};

static int
is_big_endian()
{
    static const union {
	int iv;
	char cv[4];
    } u = { 0x12345678 };

    return u.cv[0] == 0x12;
}

static int
utf8_encode(struct converter *conv,
	    const uc_char_t **inbuf,
	    size_t inleft,
	    char **outbuf,
	    size_t outleft)
{
    size_t i;

    if (!inbuf)
	return UNICONV_SUCCESS;

    for (i = 0; i < inleft; i++) {
	int seqlen = ucs4toutf8(**inbuf, NULL);
	if (seqlen < 0)
	    return UNICONV_EILSEQ;
	if (seqlen > outleft)
	    return UNICONV_E2BIG;

	ucs4toutf8(**inbuf, *outbuf);
	(*inbuf) += 1;
	(*outbuf) += seqlen;
	outleft -= seqlen;
    }

    return UNICONV_SUCCESS;
}

static int
utf8_decode(struct converter *conv,
	    const char **inbuf,
	    size_t inleft,
	    uc_char_t **outbuf,
	    size_t outleft)
{
    while (inleft) {
	uc_char_t unichar;
	int seqlen = ucs4fromutf8(*inbuf, &unichar, inleft);
	if (seqlen == -2)
	    return UNICONV_EINVAL;
	else if (seqlen < 0)
	    return UNICONV_EILSEQ;
	if (!outleft)
	    return UNICONV_E2BIG;

	**outbuf = unichar;
	(*outbuf) += 1;
	outleft -= 1;

	(*inbuf) += seqlen;
	inleft -= seqlen;
    }

    return UNICONV_SUCCESS;
}

static int
utf16_encode(struct converter *conv,
	     const uc_char_t **inbuf,
	     size_t inleft,
	     char **outbuf,
	     size_t outleft)
{
    struct utfconverter *uc = (struct utfconverter*)conv;
    size_t i;

    if (!inbuf)
	return UNICONV_SUCCESS;

    for (i = 0; i < inleft; i++) {
	int seqlen = ucs4toutf16(**inbuf, NULL);
	uc_uint16_t utf16[2];
	if (seqlen < 0)
	    return UNICONV_EILSEQ;
	if (seqlen * 2 > outleft)
	    return UNICONV_E2BIG;

	ucs4toutf16(**inbuf, utf16);
	if (uc->little_endian) {
	    (*outbuf)[0] = utf16[0] & 0xff;
	    (*outbuf)[1] = utf16[0] >> 8;
	    if (seqlen == 2) {
		(*outbuf)[2] = utf16[1] & 0xff;
		(*outbuf)[3] = utf16[1] >> 8;
	    }
	} else {
	    (*outbuf)[0] = utf16[0] >> 8;
	    (*outbuf)[1] = utf16[0] & 0xff;
	    if (seqlen == 2) {
		(*outbuf)[2] = utf16[1] >> 8;
		(*outbuf)[3] = utf16[1] & 0xff;
	    }
	}
	(*inbuf) += 1;
	(*outbuf) += seqlen * 2;
	outleft -= seqlen * 2;
    }

    return UNICONV_SUCCESS;
}

static int
utf16_decode(struct converter *conv,
	     const char **sinbuf,
	     size_t inleft,
	     uc_char_t **outbuf,
	     size_t outleft)
{
    struct utfconverter *uc = (struct utfconverter*)conv;
    const uc_uint8_t **inbuf = (const uc_uint8_t **)sinbuf;

    while (inleft) {
	uc_char_t unichar;
	uc_uint16_t utf16[2];
	int seqlen;

	if (inleft < 2)
	    return UNICONV_EINVAL;

	if (uc->little_endian)
	    utf16[0] = (*inbuf)[1] << 8 | (*inbuf)[0];
	else
	    utf16[0] = (*inbuf)[0] << 8 | (*inbuf)[1];
	if (utf16[0] >= 0xd800 && utf16[0] <= 0xbeff)
	    seqlen = 2;
	else
	    seqlen = 1;
	if (inleft < seqlen * 2)
	    return UNICONV_EINVAL;
	if (seqlen == 2) {
	    if (uc->little_endian)
		utf16[1] = (*inbuf)[3] << 8 | (*inbuf)[2];
	    else
		utf16[1] = (*inbuf)[2] << 8 | (*inbuf)[3];
	}
	seqlen = ucs4fromutf16(utf16, &unichar, seqlen);
	if (seqlen == -2)
	    return UNICONV_EINVAL;
	else if (seqlen < 0)
	    return UNICONV_EILSEQ;
	if (!outleft)
	    return UNICONV_E2BIG;

	/* BOM */
	if (unichar == 0xfffe && uc->flags & FLAG_USE_BOM_ENDIAN &&
	    !(uc->flags & FLAG_DONE_BOM_ENDIAN)) {
	    uc->flags &= ~FLAG_DONE_BOM_ENDIAN;
	    uc->little_endian ^= 1;
	    unichar = 0xfeff;
	}
	**outbuf = unichar;
	(*outbuf) += 1;
	outleft -= 1;

	(*inbuf) += seqlen * 2;
	inleft -= seqlen * 2;
    }

    return UNICONV_SUCCESS;
}

static int
utf32_encode(struct converter *conv,
	     const uc_char_t **inbuf,
	     size_t inleft,
	     char **outbuf,
	     size_t outleft)
{
    struct utfconverter *uc = (struct utfconverter*)conv;

    if (!inbuf)
	return UNICONV_SUCCESS;

    while (inleft) {
	if (outleft < 4)
	    return UNICONV_E2BIG;

	if (uc->little_endian) {
	    (*outbuf)[0] = ((**inbuf) & 0x000000ff) >>  0;
	    (*outbuf)[1] = ((**inbuf) & 0x0000ff00) >>  8;
	    (*outbuf)[2] = ((**inbuf) & 0x00ff0000) >> 16;
	    (*outbuf)[3] = ((**inbuf) & 0xff000000) >> 24;
	} else {
	    (*outbuf)[3] = ((**inbuf) & 0x000000ff) >>  0;
	    (*outbuf)[2] = ((**inbuf) & 0x0000ff00) >>  8;
	    (*outbuf)[1] = ((**inbuf) & 0x00ff0000) >> 16;
	    (*outbuf)[0] = ((**inbuf) & 0xff000000) >> 24;
	}
	(*inbuf) += 1;
	(*outbuf) += 4;
	inleft -= 1;
	outleft -= 4;
    }

    return UNICONV_SUCCESS;
}

static int
utf32_decode(struct converter *conv,
	     const char **sinbuf,
	     size_t inleft,
	     uc_char_t **outbuf,
	     size_t outleft)
{
    struct utfconverter *uc = (struct utfconverter*)conv;
    const uc_uint8_t **inbuf = (const uc_uint8_t **)sinbuf;

    if (inleft & 3)
	return UNICONV_EINVAL;

    while (inleft) {
	if (!outleft)
	    return UNICONV_E2BIG;

	if (uc->little_endian)
	    **outbuf =
		((*inbuf)[0] <<  0) |
		((*inbuf)[1] <<  8) |
		((*inbuf)[2] << 16) |
		((*inbuf)[3] << 24);
	else
	    **outbuf =
		((*inbuf)[3] <<  0) |
		((*inbuf)[2] <<  8) |
		((*inbuf)[1] << 16) |
		((*inbuf)[0] << 24);

	/* BOM */
	if (**outbuf == 0xfffe && uc->flags & FLAG_USE_BOM_ENDIAN &&
	    !(uc->flags & FLAG_DONE_BOM_ENDIAN)) {
	    uc->flags &= ~FLAG_DONE_BOM_ENDIAN;
	    uc->little_endian ^= 1;
	    **outbuf = 0xfeff;
	}

	(*inbuf) += 4;
	(*outbuf) += 1;
	inleft -= 4;
	outleft -= 1;
    }

    return UNICONV_SUCCESS;
}

static void
utfconverter_close(struct converter *conv)
{
    free(conv);
}

static void
utfconverter_reset(struct converter *suc)
{
    struct utfconverter *uc = (struct utfconverter*)suc;

    /* default to host endian, should be big endian? */
    if (uc->flags & FLAG_USE_BOM_ENDIAN)
	uc->little_endian = !is_big_endian();
    uc->flags &= ~FLAG_DONE_BOM_ENDIAN;
}

struct converter *
utfconverter_open(const char *charset)
{
    struct utfconverter *conv;

    if (strcmp(charset, "utf_8") &&
	strcmp(charset, "utf_16") &&
	strcmp(charset, "utf_16_le") &&
	strcmp(charset, "utf_16_be") &&
	strcmp(charset, "utf_32") &&
	strcmp(charset, "utf_32_le") &&
	strcmp(charset, "utf_32_be"))
	return NULL;

    conv = malloc(sizeof(struct utfconverter));
    if (!conv)
	return NULL;
    conv->flags = 0;
    conv->little_endian = !is_big_endian();
    if (!strcmp(charset, "utf_8")) {
	conv->base.encode = utf8_encode;
	conv->base.decode = utf8_decode;
    } else if (!strcmp(charset, "utf_16")) {
	conv->base.encode = utf16_encode;
	conv->base.decode = utf16_decode;
	conv->flags |= FLAG_USE_BOM_ENDIAN;
    } else if (!strcmp(charset, "utf_16_le")) {
	conv->base.encode = utf16_encode;
	conv->base.decode = utf16_decode;
	conv->little_endian = 1;
    } else if (!strcmp(charset, "utf_16_be")) {
	conv->base.encode = utf16_encode;
	conv->base.decode = utf16_decode;
	conv->little_endian = 0;
    } else if (!strcmp(charset, "utf_32")) {
	conv->base.encode = utf32_encode;
	conv->base.decode = utf32_decode;
	conv->flags |= FLAG_USE_BOM_ENDIAN;
    } else if (!strcmp(charset, "utf_32_le")) {
	conv->base.encode = utf32_encode;
	conv->base.decode = utf32_decode;
	conv->little_endian = 1;
    } else if (!strcmp(charset, "utf_32_be")) {
	conv->base.encode = utf32_encode;
	conv->base.decode = utf32_decode;
	conv->little_endian = 0;
    }
    conv->base.close = utfconverter_close;
    conv->base.reset = utfconverter_reset;

    return &conv->base;
}
