#include "converter.h"
#include "utfconverter.h"
#include "uniconv.h"
#include "unicode.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct utfconverter {
    struct converter base;
};

static int
utf8_encode(struct converter *conv,
	    const uc_char_t **inbuf,
	    size_t inleft,
	    char **outbuf,
	    size_t outleft)
{
    size_t i;

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
	    return UNICONV_E2BIG;
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
    size_t i;
    short **soutbuf = (short **)outbuf;

    for (i = 0; i < inleft; i++) {
	int seqlen = ucs4toutf16(**inbuf, NULL);
	if (seqlen < 0)
	    return UNICONV_EILSEQ;
	if (seqlen * sizeof(short) > outleft)
	    return UNICONV_E2BIG;

	ucs4toutf16(**inbuf, *soutbuf);
	(*inbuf) += 1;
	(*soutbuf) += seqlen;
	outleft -= seqlen;
    }

    return UNICONV_SUCCESS;
}

static int
utf16_decode(struct converter *conv,
	    const char **inbuf,
	    size_t inleft,
	    uc_char_t **outbuf,
	    size_t outleft)
{
    const short **sinbuf = (const short**)inbuf;

    while (inleft) {
	uc_char_t unichar;
	int seqlen = ucs4fromutf16(*sinbuf, &unichar, inleft / sizeof(short));
	if (seqlen == -2)
	    return UNICONV_E2BIG;
	else if (seqlen < 0)
	    return UNICONV_EILSEQ;
	if (!outleft)
	    return UNICONV_E2BIG;

	**outbuf = unichar;
	(*outbuf) += 1;
	outleft -= 1;

	(*sinbuf) += seqlen;
	inleft -= seqlen * sizeof(short);
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
    if (inleft * sizeof(uc_char_t) > outleft)
	return UNICONV_E2BIG;

    memcpy(*outbuf, *inbuf, inleft * sizeof(uc_char_t));
    (*inbuf) += inleft;
    (*outbuf) += inleft * sizeof(uc_char_t);
    return UNICONV_SUCCESS;
}

static int
utf32_decode(struct converter *conv,
	    const char **inbuf,
	    size_t inleft,
	    uc_char_t **outbuf,
	    size_t outleft)
{
    if (inleft & 3)
	return UNICONV_EILSEQ;
    if (inleft > outleft)
	return UNICONV_E2BIG;

    memcpy(*outbuf, *inbuf, inleft);
    (*inbuf) += inleft;
    (*outbuf) += inleft / sizeof(uc_char_t);
    return UNICONV_SUCCESS;
}

static void
utfconverter_close(struct converter *conv)
{
    free(conv);
}

struct converter *
utfconverter_open(const char *charset)
{
    struct utfconverter *conv;

    if (strcmp(charset, "utf_8") && strcmp(charset, "utf_16") &&
	strcmp(charset, "utf_32"))
	return NULL;

    conv = malloc(sizeof(struct utfconverter));
    if (!conv)
	return NULL;
    if (!strcmp(charset, "utf_8")) {
	conv->base.encode = utf8_encode;
	conv->base.decode = utf8_decode;
    } else if (!strcmp(charset, "utf_16")) {
	conv->base.encode = utf16_encode;
	conv->base.decode = utf16_decode;
    } else if (!strcmp(charset, "utf_32")) {
	conv->base.encode = utf32_encode;
	conv->base.decode = utf32_decode;
    }
    conv->base.close = utfconverter_close;
    return &conv->base;
}
