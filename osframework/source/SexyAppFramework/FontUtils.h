#ifndef FONTUTILS_H
#define FONTUTILS_H

#include "Common.h"

namespace Sexy
{

    int SexyUtf8ToUcs4Char (const char * str, uint32 * ucs4, int len);

    int SexyUtf8Strlen (const char * utf8, int len);

    int SexyUtf8ToUcs4 (const char * utf8, int len, uint32 ** retucs4);

    bool SexyUtf8Validate (const char * utf8, int len);

    int SexyUtf8FromLocale (const char * str, int len, char** result);

    int SexyUtf8FromString(const std::string& string, std::string& utf8);
}

#endif
