#ifndef __SEXY_DEBUG_H__
#define __SEXY_DEBUG_H__

namespace Sexy
{
	const int FAULT_HANDLER   = 1 << 0;
	const int CORE_DUMP       = 1 << 1;

	void DebugInit(int options);
	void DebugPrintBackTrace();
}

#endif
