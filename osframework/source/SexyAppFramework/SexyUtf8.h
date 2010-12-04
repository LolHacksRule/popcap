#ifndef FONTUTILS_H
#define FONTUTILS_H

#include "Common.h"

namespace Sexy
{

	std::string SexyGetLocaleEncoding();

	void SexySetLocaleEncoding(const std::string encoding);

	int SexyUtf8ToUcs4Char (const char * str, uint32 * ucs4, int len);

	int SexyUtf8Strlen (const char * utf8, int len);

	int SexyUtf8ToUcs4 (const char * utf8, int len, uint32 ** retucs4);

	int SexyUtf8ToUtf16 (const char * utf8, int len, short ** retutf16);

	bool SexyUtf8Validate (const char * utf8, int len);

	int SexyUtf8FromLocale (const char * str, int len, char** result);

	int SexyUtf8FromLocaleString(const std::string& string, std::string& utf8);

	bool SexyUtf8ToWString(const std::string& utf8, std::wstring& str);
	bool SexyUtf8ToWString(const std::string& utf8, Sexy::WString& str);
	std::string SexyUtf8FromWString(const Sexy::WString& str);
}

#endif
