#ifndef _H_PAK_LIB_CritSect
#define _H_PAK_LIB_CritSect

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace PakLib
{

class CritSect 
{
private:
#ifdef WIN32
	CRITICAL_SECTION mCriticalSection;
#else
        pthread_mutex_t mMutex;
#endif
	friend class AutoCrit;

public:
	CritSect(void);
	~CritSect(void);

public:
	void          Enter();
	void          Leave();
};

}

#endif // _H_CritSect
