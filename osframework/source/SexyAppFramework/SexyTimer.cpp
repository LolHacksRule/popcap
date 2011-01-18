#include "Common.h"
#include "SexyTimer.h"

#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

DWORD Sexy::GetTickCount()
{
#if defined(WIN32) || defined(_WIN32)
	return ::GetTickCount();
#elif defined(__linux__) || defined(ANDROID) || defined(__ANDROID__)
	struct timespec now;
	DWORD ticks;

	clock_gettime(CLOCK_MONOTONIC, &now);
	ticks = now.tv_sec * 1000L +
		now.tv_nsec / 1000000L;
	return ticks;
#elif defined(__APPLE__)
	static mach_timebase_info_data_t timebaseInfo;
	uint64_t cur, nano;

	cur = mach_absolute_time();

	if (timebaseInfo.denom == 0)
	  (void) mach_timebase_info(&timebaseInfo);

	nano = cur * timebaseInfo.numer / timebaseInfo.denom;
	return nano / 1000000L;
#else
	struct timeval now;
	DWORD ticks;

	gettimeofday(&now, NULL);
	ticks = now.tv_sec * 1000L + now.tv_usec / 1000;
	return ticks;
#endif
}
