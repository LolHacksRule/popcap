#ifndef __LINUXINPUTDRIVER_H__
#define __LINUXINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"

#include <pthread.h>

namespace Sexy
{

class SexyAppBase;
class LinuxInputInterface: public InputInterface {
public:
        LinuxInputInterface (InputManager* theManager);
        virtual ~LinuxInputInterface ();

public:
        virtual bool          Init ();
        virtual void          Cleanup ();

        virtual bool          HasEvent ();
        virtual bool          GetEvent (Event & event);

private:
        bool                  OpenDevice ();
        void                  CloseDevice ();

        static void *         Run (void * data);

 private:
        int                   mFd;
        int                   mDone;
        pthread_t*            mThread;
        int                   mX;
        int                   mY;

        int                   mRetry;
};

}

#endif
