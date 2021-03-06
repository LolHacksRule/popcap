#ifndef SEXYTHREAD_H
#define SEXYTHREAD_H

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#include "Common.h"

namespace Sexy {

class Thread
{
 public:
    Thread();
    Thread(const Thread &other);
    ~Thread();

    Thread&                   operator = (const Thread &other);
    bool                      operator == (const Thread &other);
    bool                      operator != (const Thread &other);

    static Thread             Create(void (*start_routine) (void *),
				     void * arg);
    static Thread             Self(void);

    void                      Join(void);

    bool                      IsValid(void);

 private:
#ifdef WIN32
    HANDLE mThread;
    DWORD mThreadId;
#else
    pthread_t mThread;
#endif
    bool mValid;
};

}

#endif
