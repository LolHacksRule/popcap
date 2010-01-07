#ifndef __SEXYLANG_H__
#define __SEXYLANG_H__

#include "Common.h"

namespace Sexy {
	std::string SexyGetCharset();
	std::string SexyGetLocaleName(const char* category = "LC_CTYPE");
}

#endif
