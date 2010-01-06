#include "FontUtils.h"
#include "AutoCrit.h"

#ifndef WIN32
#include <iconv.h>
#include <errno.h>
#include <langinfo.h>
#endif

#include <assert.h>

#include "cjkcodecs/multibytecodec.h"

namespace Sexy
{
	static inline bool
	SexyUnicodeValide(uint32 unichar)
	{
		return ((unichar) < 0x110000 &&
			(((unichar) & 0xFFFFF800) != 0xD800) &&
			((unichar) < 0xFDD0 || (unichar) > 0xFDEF) &&
			((unichar) & 0xFFFE) != 0xFFFE);

	}

	int
	SexyUsc4ToUtf8 (uint32 unichar, char * utf8)
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

	int
	SexyUsc4ToUtf16 (uint32 unichar, short * utf16)
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

	int SexyUtf8ToUcs4Char (const char * str, uint32 * ucs4, int len)
	{
		const unsigned char * utf8 = (const unsigned char *)str;
		unsigned char c;
		unsigned char mask;
		int i;
		int seqlen;
		uint32 result;

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
			return -1;

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

		if (!SexyUnicodeValide(result))
			return -1;
		if (SexyUsc4ToUtf8(result, 0) != seqlen)
			return -1;
		if (ucs4)
			*ucs4 = result;
		return seqlen;
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

	int SexyUtf8ToUtf16 (const char * utf8, int len, short ** retutf16)
	{

		int i, slen, clen;
		short * utf16;

		if (len < 0)
			len = strlen (utf8);

		slen = SexyUtf8Strlen (utf8, len);
		if (slen < 0)
			return -1;

		utf16 = new short[slen * 2 + 1];
		if (!utf16)
			return -1;

		for (i = 0; len > 0; len -= clen, utf8 += clen)
		{
			uint32 unichar;
			clen = SexyUtf8ToUcs4Char (utf8, &unichar, len);
			i += SexyUsc4ToUtf16(unichar, utf16 + i);
		}

		utf16[i] = 0;
		*retutf16 = utf16;
		return i;
	}

	bool SexyUtf8Validate (const char * utf8, int len)
	{
		return SexyUtf8Strlen(utf8, len) >= 0;
	}

	static int CJKCodecsConvert(const char * modname, const char* charset,
				    const char* inbuf, size_t inlen,
				    char** retoutbuf, size_t* retoutlen)
	{
		MultibyteCodecState state;
		const char* inp;
		char * utf8;
		char * utf8p;
		size_t utf8len;
		ucs4_t *ucs4;
		ucs4_t *outp;
		int ret;

		ret = mbcs_init(&state, modname, charset);
		if (ret < 0)
			return -1;

		ucs4 = new ucs4_t[inlen];
		outp = ucs4;
		inp = inbuf;
		ret = mbcs_decode(&state, &inp, inlen, &outp, inlen);
		if (ret != 0)
		{
			delete [] ucs4;
			return -1;
		}

		utf8len = 0;
		for (int i = 0; i < outp - ucs4; i++)
		{
			int chlen = SexyUsc4ToUtf8(ucs4[i], 0);
			assert(chlen > 0);
			utf8len += chlen;
		}

		utf8p = utf8 = new char[utf8len + 1];
		for (int i = 0; i < outp - ucs4; i++)
		{
			int chlen = SexyUsc4ToUtf8(ucs4[i], utf8p);
			assert(chlen > 0);
			utf8p += chlen;
		}
		utf8p[0] = '\0';
		*retoutbuf = utf8;
		*retoutlen = utf8len;
		return inp - inbuf;
	}

	static int CJKCodecsUtf8FallbackConvert (const char * str, int len,
						 char** result)
	{
		static struct
		{
			const char* module;
			const char* charset;
		} charsets[] = {
			{ "cn", "gb18030" },
			{ "cn", "gbk" },
			{ "tw", "big5" },
			{ "tw", "cp950" },
			{ 0, 0 }
		};

		for (unsigned i = 0; charsets[i].module; i++)
		{
			char* outbuf;
			size_t outlen;
			int ret = CJKCodecsConvert(charsets[i].module,
						   charsets[i].charset,
						   str, len, &outbuf, &outlen);
			if (ret < 0)
				return -1;
			ret = SexyUtf8Strlen(outbuf, outlen);
			*result = outbuf;
			return ret;
		}

		return -1;
	}

#ifndef WIN32
	static int CharsetConvert(iconv_t cd, const char* inbuf, size_t inlen,
				  char** retoutbuf, size_t* retoutlen)
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
#if defined(__FreeBSD__)
			ret = iconv(cd, (const char**)&in, &left, &out, &outleft);
#else
			ret = iconv(cd, &in, &left, &out, &outleft);
#endif
			if (ret == (size_t)-1 && errno != E2BIG)
			{
				delete [] outbuf;
				return -1;
			}
			else if (ret >= 0)
			{
				break;
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
			if (!charset || !stricmp (charset, "UTF-8") ||
			    !stricmp (charset, "utf8"))
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
	int    ConvertUtf16toUtf8 (unsigned short* inbuf, size_t inlen,
				   char* outbuf)
	{
		const uint32 UNI_SUR_HIGH_START = 0xd800;
		const uint32 UNI_SUR_HIGH_END   = 0xdbff;
		const uint32 UNI_SUR_LOW_START  = 0xdc00;
		const uint32 UNI_SUR_LOW_END    = 0xdfff;
		unsigned short* inbufend = inbuf + inlen;
		int ret = 0;

		while (inbuf < inbufend) {
			uint32 ch;

			ch = *inbuf++;
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END &&
			    inbuf < inbufend)
			{
				uint32 ch2 = *inbuf;
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
				{
					ch = (ch - UNI_SUR_HIGH_START) << 10;
					ch +=  ch2 - UNI_SUR_LOW_START + 0x0010000;
					inbuf++;
				}
			}

			ret += SexyUsc4ToUtf8(ch, outbuf ? outbuf + ret : 0);
		}
		return ret;
	}

	static int Utf8FromLocale (const char * str, int len, char** result)
	{
		int utf16len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
		if (utf16len <= 0)
			return -1;

		unsigned short * utf16 = new unsigned short[utf16len];
		if (MultiByteToWideChar(CP_ACP, 0, str, -1, (WCHAR*)utf16, utf16len) <= 0)
		{
			delete [] utf16;
			return -1;
		}
		int utf8len = ConvertUtf16toUtf8(utf16, utf16len - 1, 0);
		if (utf8len < 0)
		{
			delete [] utf16;
			return -1;
		}

		char* utf8 = new char[utf8len + 1];
		ConvertUtf16toUtf8(utf16, utf16len, utf8);
		utf8[utf8len] = '\0';
		delete [] utf16;
		*result = utf8;
		return SexyUtf8Strlen(utf8, utf8len);;
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

		ret = CJKCodecsUtf8FallbackConvert(str, len, result);
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

	bool SexyUtf8ToWString(const std::string& utf8,
			       std::wstring& str)
	{
		// asume the encoding of wstring is utf-16
		if (sizeof(wchar_t) == 2)
		{
			short *utf16;
			int len;

			len = SexyUtf8ToUtf16(utf8.c_str(), utf8.length(), &utf16);

			if (len < 0)
				return false;
			for (int i = 0; i < len; i++)
				str[i] = utf16[i];

			delete [] utf16;
		}
		else
		{
			// asume the encoding of wstring is ucs4 otherwise
			uint32 *ucs4;
			int len;

			len = SexyUtf8ToUcs4(utf8.c_str(), utf8.length(), &ucs4);

			if (len < 0)
				return false;
			for (int i = 0; i < len; i++)
				str[i] = ucs4[i];

			delete ucs4;
		}

		return true;
	}
}
