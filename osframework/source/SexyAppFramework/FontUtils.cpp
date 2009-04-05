#include "FontUtils.h"

namespace Sexy
{

	int SexyUtf8ToUcs4Char (const char * str, uint32 * ucs4, int len)
	{
		const unsigned char * utf8 = (const unsigned char *)str;
		unsigned char ch;
		int extra;
		uint32 unicode;

		if (!len)
			return 0;

		ch = *utf8++;

		if (!(ch & 0x80)) {
			unicode = ch;
			extra = 0;
		} else if (!(ch & 0x40)) {
			return -1;
		} else if (!(ch & 0x20)) {
			unicode = ch & 0x1f;
			extra = 1;
		} else if (!(ch & 0x10)) {
			unicode = ch & 0xf;
			extra = 2;
		} else if (!(ch & 0x08)) {
			unicode = ch & 0x07;
			extra = 3;
		} else if (!(ch & 0x04)) {
			unicode = ch & 0x03;
			extra = 4;
		} else if (!(ch & 0x02)) {
			unicode = ch & 0x01;
			extra = 5;
		} else {
			return -1;
		}

		if (extra > len - 1)
			return -1;

		while (extra--) {
			unicode <<= 6;
			ch = *utf8++;

			if ((ch & 0xc0) != 0x80)
				return -1;

			unicode |= ch & 0x3f;
		}

		if (ucs4)
			*ucs4 = unicode;
		return utf8 - (const unsigned char *)str;
	}

	int SexyUtf8Strlen (const char * utf8, int len)
	{
		uint32 ucs4;
		int clen;
		int ucs4len = 0;

		if (len < 0)
			len = strlen (utf8);

		while (len > 0) {
			clen = SexyUtf8ToUcs4Char (utf8, &ucs4, len);
			if (clen < 0)
				return -1;
			if (!ucs4)
				break;
			len -= clen;
			utf8 += clen;
			ucs4len++;
		}

		return ucs4len;
	}

	int SexyUtf8ToUcs4 (const char * utf8, int len, uint32 ** retucs4)
	{

		int i, slen, clen;
		uint32 * ucs4;

		if (len < 0)
			len = strlen (utf8);

		slen = SexyUtf8Strlen (utf8, len);
		if (slen < 0)
			return -1;

		ucs4 = new uint32[slen + 1];
		if (!ucs4)
			return -1;

		for (i = 0; i < slen; i++, len -= clen, utf8 += clen)
			clen = SexyUtf8ToUcs4Char (utf8, ucs4 + i, len);

		ucs4[slen] = 0;
		*retucs4 = ucs4;
		return slen;
	}

	bool
	SexyUtf8Validate (const char * utf8, int len)
	{
		return SexyUtf8Strlen(utf8, len) >= 0;
	}

}
