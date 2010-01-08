#include "uniconv.h"
#include "unicode.h"

int
unicode_valide(uc_char_t unichar)
{
    return ((unichar) < 0x110000 &&
	    (((unichar) & 0xFFFFF800) != 0xD800) &&
	    ((unichar) < 0xFDD0 || (unichar) > 0xFDEF) &&
	    ((unichar) & 0xFFFE) != 0xFFFE);

}

int
ucs4toutf16(uc_char_t unichar, uc_uint16_t *utf16)
{
    unsigned len;

    if (unichar < 0x10000)
    {
	len = 1;
    }
    else
    {
	len = 2;
    }

    if (utf16)
    {
	if (len == 1) {
	    utf16[0] = unichar;
	} else {
	    utf16[0] = (unichar - 0x10000) / 0x400 + 0xd800;
	    utf16[1] = (unichar - 0x10000) % 0x400 + 0xdc00;
	}
    }

    return len;
}

int
ucs4fromutf16(const uc_uint16_t *utf16, uc_char_t *ret, int inlen)
{
    static const uc_char_t UNI_SUR_HIGH_START = 0xd800;
    static const uc_char_t UNI_SUR_HIGH_END   = 0xdbff;
    static const uc_char_t UNI_SUR_LOW_START  = 0xdc00;
    static const uc_char_t UNI_SUR_LOW_END    = 0xdfff;
    uc_char_t ch;
    int len = 1;

    ch = utf16[0];
    if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
	uc_char_t ch2;

	if (inlen < 2)
	    return -2;

	ch2 = utf16[1];
	if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
	    len++;
	    ch = (ch - UNI_SUR_HIGH_START) << 10;
	    ch +=  ch2 - UNI_SUR_LOW_START + 0x0010000;
	} else {
	    return -1;
	}
    }

    if (ret)
	*ret = ch;
    return len;
}

int
ucs4toutf8(uc_char_t unichar, char *utf8)
{
    int prefix;
    int i;
    unsigned len;

    if (unichar < 0x80)
    {
	prefix = 0;
	len = 1;
    }
    else if (unichar < 0x800)
    {
	prefix = 0xc0;
	len = 2;
    }
    else if (unichar < 0x10000)
    {
	prefix = 0xe0;
	len = 3;
    }
    else if (unichar < 0x200000)
    {
	prefix = 0xf0;
	len = 4;
    }
    else if (unichar < 0x4000000)
    {
	prefix = 0xf8;
	len = 5;
    }
    else
    {
	prefix = 0xfc;
	len = 6;
    }

    if (utf8)
    {
	for (i = len - 1; i > 0; --i)
	{
	    utf8[i] = (unichar & 0x3f) | 0x80;
	    unichar >>= 6;
	}
	utf8[0] = unichar | prefix;
    }

    return len;

}

int ucs4fromutf8 (const char * str, uc_char_t * ucs4, int len)
{
    const unsigned char * utf8 = (const unsigned char *)str;
    unsigned char c;
    unsigned char mask;
    int i;
    int seqlen;
    uc_char_t result;

    c = *utf8;
    if (c < 0x80)
    {
	result = c;
	mask = 0x7f;
	seqlen = 1;
    }
    else if ((c & 0xe0) == 0xc0)
    {
	mask = 0x1f;
	seqlen = 2;
    }
    else if ((c & 0xf0) == 0xe0)
    {
	mask = 0x0f;
	seqlen = 3;
    }
    else if ((c & 0xf8) == 0xf0)
    {
	mask = 0x07;
	seqlen = 4;
    }
    else if ((c & 0xfc) == 0xf8)
    {
	mask = 0x03;
	seqlen = 5;
    }
    else if ((c & 0xfe) == 0xfc)
    {
	mask = 0x01;
	seqlen = 6;
    }
    else
    {
	return -1;
    }

    if (len < seqlen)
	return -2;

    result = c & mask;
    for (i = 1; i < seqlen; i++)
    {
	c = (unsigned char)utf8[i];
	if ((c & 0xc0) != 0x80)
	{
	    return -1;
	}
	result <<= 6;
	result |= c & 0x3f;
    }

    if (!unicode_valide(result))
	return -1;
    if (ucs4toutf8(result, NULL) != seqlen)
	return -1;
    if (ucs4)
	*ucs4 = result;
    return seqlen;
}
