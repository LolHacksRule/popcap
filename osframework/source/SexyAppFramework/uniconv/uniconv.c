#include "uniconv.h"
#include "uniconvint.h"
#include "multibytecodec.h"
#include "singlebytecodec.h"
#include "charsetalias.h"
#include "converter.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define UNICONV_MODE_IGNORE   (1 << 0)
#define UNICONV_MODE_TRANSLIT (1 << 1)

struct _uniconv {
    struct converter *from;
    struct converter *to;
    char from_charset[64];
    char to_charset[64];
    int mode;
};

static char
uniconv_lower(char c)
{
    if (c >= 'A' && c <= 'Z')
	return c - 'A' + 'a';
    return c;
}

static char *
uniconv_strrstr(const char *str, const char *token)
{
    int len = strlen(token);
    const char *p = str + strlen(str);

    while (str <= --p)
        if (p[0] == token[0] && strncmp(p, token, len) == 0)
            return (char *)p;
    return NULL;
}

static const char*
uniconv_canonical_charset(const char *charset,
			  char *buf, size_t bufsz,
			  int *retmode)
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

    if (retmode) {
	char *p;

	for (p = uniconv_strrstr(buf, "//"); p;
	     p = uniconv_strrstr(buf, "//")) {
	    if (!strcmp(p + 2, "ignore"))
		*retmode |= UNICONV_MODE_IGNORE;
	    else if (!strcmp(p + 2, "translit"))
		*retmode |= UNICONV_MODE_TRANSLIT;
	    *p = '\0';
	}
    }

    return get_canonical_charset(buf);
}

uniconv_t*
uniconv_open(const char *to, const char *from)
{
    char frombuf[64];
    char tobuf[64 * 2];
    int mode;
    uniconv_t *uc;
    struct converter *fc, *tc;

    if (!from || !to)
	return NULL;

    mode = 0;
    from = uniconv_canonical_charset(from, frombuf, sizeof(frombuf), NULL);
    to = uniconv_canonical_charset(to, tobuf, sizeof(tobuf), &mode);
    if (!from || !to)
	return NULL;

    if (strlen(from) > 63 || strlen(to) > 63)
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
    uc->mode = mode;
    strcpy(uc->from_charset, from);
    strcpy(uc->to_charset, to);
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
	     size_t *inleft,
	     char **outbuf,
	     size_t *outleft)
{
    int ret;
    size_t ucs4len;
    const char *saved_inbuf;
    const char *saved_outbuf;

    if (!uc)
	return UNICONV_EBADF;

    /* reset converter */
    if ((!inbuf || !(*inbuf)) && (!outbuf || !(*outbuf))) {
	converter_reset(uc->from);
	converter_reset(uc->to);
	return UNICONV_SUCCESS;
    }

    /* converting/pushing input data */
    if (inbuf && *inbuf) {
	uc_char_t ucs4;
	uc_char_t *ucsbuf;

	saved_inbuf = *inbuf;
	saved_outbuf = *outbuf;
	ret = UNICONV_SUCCESS;
	while (*inleft) {
	    size_t left = 0;
	    const char *abuf;
	    const char *aoutbuf;

	    ucsbuf = &ucs4;
	    abuf = *inbuf;
	    left = *inleft;
	    ret = uc->from->decode(uc->from, &abuf, left, &ucsbuf, 1);
	    if (ret < 0 && ret != UNICONV_E2BIG)
		return ret;

	    assert (ucsbuf - &ucs4 > 0 && ucsbuf - &ucs4 <= 1);
	    ucs4len = ucsbuf - &ucs4;
	    ucsbuf = &ucs4;
	    aoutbuf = *outbuf;

	    ret = uc->to->encode(uc->to, (const ucs4_t **)&ucsbuf, ucs4len,
				 outbuf, *outleft);
	    if (ret == UNICONV_EILSEQ && uc->mode & UNICONV_MODE_IGNORE)
		ret = UNICONV_SUCCESS;
	    *outleft -= *outbuf - aoutbuf;
	    if (ret < 0)
		break;
	    *inleft -= abuf - *inbuf;
	    (*inbuf) = abuf;
	}
    } else {
	/* converting pending data in buffer */
	assert (outbuf && *outbuf);
	saved_inbuf = NULL;
	saved_outbuf = *outbuf;
	ret = uc->to->encode(uc->to, NULL, 0, outbuf, *outleft);
	*outleft -= *outbuf - saved_outbuf;
    }

    return ret;
}

