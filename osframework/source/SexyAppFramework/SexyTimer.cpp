#include "Common.h"
#include "SexyTimer.h"

DWORD Sexy::GetTickCount()
{
#ifdef WIN32
	return ::GetTickCount();
#elif defined(__linux__)
	struct timespec now;
	DWORD ticks;

	clock_gettime(CLOCK_MONOTONIC, &now);
	ticks = now.tv_sec * 1000L +
		now.tv_nsec / 1000000L;
	return ticks;
#else
	struct timeval now;
	DWORD ticks;

	gettimeofday(&now, NULL);
	ticks = now.tv_sec * 1000L + now.tv_usec / 1000;
	return ticks;
#endif
}
