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
        virtual bool          GetEvent (Event &event);
	virtual bool          GetInfo(InputInfo &theInfo, int subid = 0);

private:
        bool                  OpenDevice ();
        void                  CloseDevice ();
	void                  UpdateStatus ();
	void                  UpdateStatus (const Event &event);

        static void           Run (void * data);

 private:
        int                   mFd;
        volatile int          mDone;
        Thread                mThread;
        int                   mX;
        int                   mY;
	bool                  mHasKey;
	bool                  mHasPointer;
	DWORD                 mKeyCount;
	DWORD                 mPointerCount;
};

}

#endif
