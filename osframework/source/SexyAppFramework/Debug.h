#ifndef __DEBUG_INCLUDED__
#define __DEBUG_INCLUDED__

#include "Common.h"
#include <cassert>

extern SEXY_EXPORT bool gInAssert;

#ifdef SEXY_TRACING_ENABLED
SEXY_EXPORT void SexyTrace(const char *theStr);
#define SEXY_TRACE(theStr) SexyTrace(theStr)
#else
#define SEXY_TRACE(theStr)
#endif

extern SEXY_EXPORT void SexyTraceFmt(const SexyChar* fmt ...);
extern SEXY_EXPORT void OutputDebug(const SexyChar* fmt ...);

#ifdef NDEBUG

#define DBG_ASSERTE(exp)	((void)0)
#define DBG_ASSERT(exp)		((void)0)

#else

#define DBG_ASSERTE(exp)	{ gInAssert = true; assert(exp); gInAssert = false; }
#define DBG_ASSERT(exp)		{ gInAssert = true; assert(exp); gInAssert = false; }

#endif // NDEBUG

#endif //__DEBUG_INCLUDED__
