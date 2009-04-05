#pragma warning( disable : 4786 )

#include "CritSect.h"
#ifdef WIN32
#include <windows.h>
#endif

using namespace Sexy;

////////////////////////////////////////////////////////////////////////////////

CritSect::CritSect(void)
{
#ifdef WIN32
	InitializeCriticalSection(&mCriticalSection);
#else
	pthread_mutex_init(&mMutex, NULL);
#endif
}

////////////////////////////////////////////////////////////////////////////////

CritSect::~CritSect(void)
{
#ifdef WIN32
	DeleteCriticalSection(&mCriticalSection);
#else
	pthread_mutex_destroy(&mMutex);
#endif
}

void CritSect::Enter(void)
{
#ifdef WIN32
	EnterCriticalSection(&mCriticalSection); 
#else
	pthread_mutex_lock(&mMutex);
#endif
}

void CritSect::Leave(void)
{
#ifdef WIN32
	LeaveCriticalSection(&mCriticalSection);
#else
	pthread_mutex_unlock(&mMutex);
#endif
}
