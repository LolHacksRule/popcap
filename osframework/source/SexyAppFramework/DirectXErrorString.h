#ifndef __SEXY_DIRECTXERRORSTRING_H__
#define __SEXY_DIRECTXERRORSTRING_H__
#ifdef WIN32
#include <string>
#include <ddraw.h>

namespace Sexy
{
	std::string GetDirectXErrorString(HRESULT theResult);
} // namespace 
#endif
#endif
