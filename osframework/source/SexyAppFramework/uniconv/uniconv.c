#include "uniconv.h"
#include "uniconvint.h"
#include "multibytecodec.h"
#include "singlebytecodec.h"
#include "charsetalias.h"
#include "converter.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct _uniconv {
    struct converter *from;
    struct converter *to;
};

static char
uniconv_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
	return c - 'A' + 'a';
    return c;
}

static const char*
uniconv_canonical_charset(const char *charset,
			  char *buf, size_t bufsz)
{
    size_t i;
    size_t len = strlen(charset);

    if (len + 1  > bufsz)
	return NULL;
    for (i = 0; i < len + 1; i++) {
	if (charset[i] == '-')
	    buf[i] = '_';
	else
	    buf[i] = uniconv_lower(charset[i]);
    }

    return get_canonical_charset(buf);
}

uniconv_t*
uniconv_open(const char *from, const char *to)
{
    char frombuf[64];
    char tobuf[64];
    uniconv_t *uc;
    struct converter *fc, *tc;

    if (!from || !to)
	return NULL;

    from = uniconv_canonical_charset(from, frombuf, sizeof(frombuf));
    to = uniconv_canonical_charset(to, tobuf, sizeof(tobuf));
    if (!from || !to)
	return NULL;

    fc = converter_open(from);
    tc = converter_open(to);
    if (!fc || !tc)
	goto close_conv;

    uc = malloc(sizeof(uniconv_t));
    if (!uc)
	goto close_conv;
    uc->from = fc;
    uc->to = tc;
    return uc;
 close_conv:
    converter_close(fc);
    converter_close(tc);
    return NULL;
}

void
uniconv_close(uniconv_t *uc)
{
    if (!uc)
	return;

    converter_close(uc->from);
    converter_close(uc->to);
    free(uc);
}

#define UNICONV_MAX_LOCAL 2048
int
uniconv_conv(uniconv_t *uc,
	     const char **inbuf,
	     size_t inleft,
	     char **outbuf,
	     size_t outleft)
{
    int ret;
    uc_char_t local_ucs4[UNICONV_MAX_LOCAL];
    uc_char_t *ucs4, *inucs4;
    size_t ucs4len;

    if (!uc)
	return UNICONV_EBADF;

    if (!inbuf || !outbuf)
	return UNICONV_EINVAL;

    if (inleft < UNICONV_MAX_LOCAL)
	ucs4 = local_ucs4;
    else
	ucs4 = malloc(sizeof(uc_char_t) * inleft);
    if (!ucs4)
	return UNICONV_EINVAL;

    inucs4 = ucs4;
    ucs4len = inleft;
    ret = uc->from->decode(uc->from, inbuf, inleft, &inucs4, ucs4len);
    if (ret)
	goto error_decode;

    ucs4len = inucs4 - ucs4;
    inucs4 = ucs4;
    ret = uc->to->encode(uc->to, (const ucs4_t **)&inucs4, ucs4len, outbuf, outleft);

 error_decode:
    if (ucs4 != local_ucs4)
	free (ucs4);
    return ret;
}

