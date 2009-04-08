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
	LinuxInputInterface (InputManager* theManager, const char* theName);
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
	bool                  mGrabed;
        volatile int          mDone;
        pthread_t             mThread;
	bool                  mInitialized;
        int                   mX;
        int                   mY;

        int                   mRetry;

	std::string         * mDeviceName;
};

}

#endif
