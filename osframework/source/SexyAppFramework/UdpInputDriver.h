#ifndef __UDPINPUTDRIVER_H__
#define __UDPINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"
#include "SexyThread.h"

namespace Sexy
{

class SexyAppBase;
class UdpInputInterface: public InputInterface {
public:
        UdpInputInterface (InputManager* theManager);
        virtual ~UdpInputInterface ();

public:
        virtual bool          Init ();
        virtual void          Cleanup ();

        virtual bool          HasEvent ();
        virtual bool          GetEvent (Event & event);
	virtual bool          GetInfo(InputInfo &theInfo);

private:
        bool                  OpenDevice ();
        void                  CloseDevice ();

        static void           Run (void * data);

 private:
        int                   mFd;
        volatile int          mDone;
        Thread                mThread;
        int                   mX;
        int                   mY;
};

}

#endif
