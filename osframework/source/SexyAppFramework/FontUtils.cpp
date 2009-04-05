#include "FontUtils.h"
#include "AutoCrit.h"

#ifndef WIN32
#include <iconv.h>
#include <errno.h>
#include <langinfo.h>
#endif

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

	bool SexyUtf8Validate (const char * utf8, int len)
	{
		return SexyUtf8Strlen(utf8, len) >= 0;
	}

#ifndef WIN32
	static int CharsetConvert(iconv_t cd, const char* inbuf, size_t inlen, char** retoutbuf, size_t* retoutlen)
	{
		size_t ret;
		char* outbuf;
		size_t left;
		size_t outleft;
		size_t outlen;
		char* out;

		outlen = inlen * 3 / 2 + 1;
		out = outbuf = new char[outlen];
		do
		{
			char* in = (char *)inbuf;
			left = inlen;
			outleft = outlen - 1;
			ret = iconv(cd, &in, &left, &out, &outleft);
			if (ret >= 0)
			{
				break;
			}
			else if (ret == (size_t)-1 && errno != E2BIG)
			{
				delete [] outbuf;
				return -1;
			}
			delete [] outbuf;
			outlen = outlen * 3 / 2 + 1;
			out = outbuf = new char[outlen];
		} while (true);

		*retoutbuf = outbuf;
		*retoutlen = outlen - 1 - outleft;
		outbuf[*retoutlen] = '\0';
		return inlen - left;
	}

	static int Utf8FromLocale (const char * str, int len, char** result)
	{
		static CritSect aCritSect;
		AutoCrit aAutoCrit(aCritSect);
		static iconv_t cd = (iconv_t)-1;

		if (cd == (iconv_t)-1)
		{
			const char* charset = nl_langinfo(CODESET);
			if (!charset || stricmp (charset, "UTF-8") ||
			    stricmp (charset, "utf8"))
				return -1;

			cd = iconv_open("UTF-8", charset);
			if (cd == (iconv_t)-1)
				return -1;
		}

		char* outbuf;
		size_t outlen;
		int ret = CharsetConvert(cd, str, len, &outbuf, &outlen);
		if (ret < 0)
			return -1;
		ret = SexyUtf8Strlen(outbuf, outlen);
		*result = outbuf;
		return ret;
	}

	static int Utf8FallbackConvert (const char * str, int len, char** result)
	{
		static CritSect aCritSect;
		AutoCrit aAutoCrit(aCritSect);
		const unsigned int MAX_CHARSETS = 4;
		static const char* charsets[MAX_CHARSETS] =
			{
				"GB18030",
				"GBK",
				"GB2312",
				"BIG5"
			};
		static iconv_t cds[MAX_CHARSETS] =
			{
				(iconv_t)-1,
				(iconv_t)-1,
				(iconv_t)-1,
				(iconv_t)-1
			};

		for (unsigned i = 0; i < MAX_CHARSETS; i++)
		{
			if (cds[i] == (iconv_t)-1)
			{
				cds[i] = iconv_open("UTF-8", charsets[i]);
				if (cds[i] == (iconv_t)-1)
					continue;
			}

			char* outbuf;
			size_t outlen;
			int ret = CharsetConvert(cds[i], str, len, &outbuf, &outlen);
			if (ret < 0)
				return -1;
			ret = SexyUtf8Strlen(outbuf, outlen);
			if (ret < 0)
				delete [] outbuf;
			else
				*result = outbuf;
			return ret;
		}

		return -1;
	}
#else
	static int Utf8FromLocale (const char * str, int len, char** result)
	{
		return -1;
	}

	static int Utf8FallbackConvert (const char * str, int len, char** result)
	{
		return -1;
	}
#endif
	int SexyUtf8FromLocale (const char * str, int len, char** result)
	{
		int ret;

		if (len < 0)
			len = strlen(str);

		ret = Utf8FromLocale(str, len, result);
		if (ret >= 0)
			return ret;

		ret = Utf8FallbackConvert(str, len, result);
		if (ret >= 0)
			return ret;
		return -1;
	}

	int SexyUtf8FromString(const std::string& string,
			       std::string& utf8)
	{
		int len = SexyUtf8Strlen(string.c_str(), -1);
		if (len >= 0)
		{
			utf8 = string;
			return len;
		}

		char* result;
		len = SexyUtf8FromLocale(string.c_str(), -1, &result);
		if (len >= 0)
		{
			utf8 = std::string(result);
			delete [] result;
			return len;
		}

		return -1;
	}

}
