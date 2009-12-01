#ifndef __LINUXINPUTDRIVER_H__
#define __LINUXINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"

#include <pthread.h>

namespace Sexy
{

class SexyAppBase;
class LinuxInputDriver;

struct LinuxDeviceInfo {
	int          vid;
	int          pid;
	int          version;
	int          bustype;

	bool operator == (const LinuxDeviceInfo &other) const
	{
		return
		vid == other.vid && pid == other.pid &&
		version == other.version && bustype == other.bustype;
        }
};

class LinuxInputInterface: public InputInterface {
public:
	LinuxInputInterface (InputManager* theManager,
			     LinuxInputDriver *driver,
			     const char* theName,
			     bool hotpluged = false);
        virtual ~LinuxInputInterface ();

public:
        virtual bool          Init ();
        virtual void          Cleanup ();

        virtual bool          HasEvent ();
        virtual bool          GetEvent (Event & event);
	virtual bool          GetInfo (InputInfo &theInfo);

	const std::string &   GetDeviceName ()
	{
		return mDeviceName;
	}

private:
        bool                  OpenDevice ();
        void                  CloseDevice ();
	bool                  ReopenDevice ();

        static void *         Run (void * data);

 private:
        int                   mFd;
	bool                  mGrabed;
        volatile int          mDone;
        pthread_t             mThread;

	bool                  mHotpluged;
	bool                  mInitialized;
        int                   mX;
        int                   mY;

	int                   mMinX;
	int                   mMaxX;
	int                   mMinY;
	int                   mMaxY;

        int                   mRetry;
	int                   mModifiers;

	std::string           mDeviceName;

	LinuxInputDriver     *mDriver;
	LinuxDeviceInfo       mInfo;
	bool                  mHasPointer;
	bool                  mHasKey;
};

}

#endif
