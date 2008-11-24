#ifndef _H_CritSect
#define _H_CritSect

#include "Common.h"
#ifndef WIN32
#include <pthread.h>
#endif
class CritSync;

namespace Sexy
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
};

}

#endif // _H_CritSect
