#ifndef FONTUTILS_H
#define FONTUTILS_H

#include "Common.h"

namespace Sexy
{

    SEXY_EXPORT int SexyUtf8ToUcs4Char (const char * str, uint32 * ucs4, int len);

    SEXY_EXPORT int SexyUtf8Strlen (const char * utf8, int len);

    SEXY_EXPORT int SexyUtf8ToUcs4 (const char * utf8, int len, uint32 ** retucs4);

    SEXY_EXPORT int SexyUtf8ToUtf16 (const char * utf8, int len, short ** retutf16);

    SEXY_EXPORT bool SexyUtf8Validate (const char * utf8, int len);

    SEXY_EXPORT int SexyUtf8FromLocale (const char * str, int len, char** result);

    SEXY_EXPORT int SexyUtf8FromString(const std::string& string, std::string& utf8);

    SEXY_EXPORT bool SexyUtf8ToWString(const std::string& utf8, std::wstring& str);
}

#endif
