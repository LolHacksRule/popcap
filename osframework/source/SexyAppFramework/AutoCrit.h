#ifndef __AUTOCRIT_INCLUDED__
#define __AUTOCRIT_INCLUDED__

#include "Common.h"
#include "CritSect.h"

namespace Sexy
{

class AutoCrit
{
#ifdef WIN32
	LPCRITICAL_SECTION		mCritSec;
#else
        pthread_mutex_t                * mMutex;
#endif
public:
#ifdef WIN32
	AutoCrit(LPCRITICAL_SECTION theCritSec) : 
		mCritSec(theCritSec)
	{ 
		EnterCriticalSection(mCritSec); 
	}

	AutoCrit(const CritSect& theCritSect) : 
		mCritSec((LPCRITICAL_SECTION) &theCritSect.mCriticalSection)
	{ 
		EnterCriticalSection(mCritSec); 
	}
#else
	AutoCrit(pthread_mutex_t * theMutex) : 
		mMutex(theMutex)
	{ 
		pthread_mutex_lock(mMutex);
	}

	AutoCrit(const CritSect& theCritSect) : 
		mMutex((pthread_mutex_t *)&theCritSect.mMutex)
	{ 
		pthread_mutex_lock(mMutex);
	}
#endif

	~AutoCrit()
	{
#ifdef WIN32
		LeaveCriticalSection(mCritSec);
#else
                pthread_mutex_lock(mMutex);
#endif
	}
};

}

#endif //__AUTOCRIT_INCLUDED__
