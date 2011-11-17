#ifndef __LINUXINPUTDRIVER_H__
#define __LINUXINPUTDRIVER_H__

#include "Common.h"
#include "InputInterface.h"

#include <pthread.h>

struct input_event;
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

struct LinuxAxisInfo {
	float        factor;

	float        coef[3];
	float        fuzz;
	float        flat;
	float        minimum;
	float        maximum;
	float        resolution;

	int          devFuzz;
	int          devFlat;
	int          devMinimum;
	int          devMaximum;
	int          devResolution;
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
	virtual bool          GetInfo (InputInfo &theInfo, int subid = 0);
	virtual bool          IsGrabbed ();
	virtual bool          Grab (bool);

	bool                  HandleKeyEvent (struct input_event & linux_event,
					      int &modifiers, Event & event);

	bool	              HandleRelEvent (struct input_event & linux_event,
					      Event& event);

	bool                  HandleAbsEvent (struct input_event & linux_event,
					      Event & event);

	bool                  HandleEvent (struct input_event& linux_event,
					   int &modifiers, Event &event);

	void                  HandleEvents(struct input_event* linux_event, int nevents);

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
	typedef std::map<int, LinuxAxisInfo> AxisInfoMap;
	AxisInfoMap           mAxisInfoMap;
	typedef std::map<int, int> KeyCodeMap;
	KeyCodeMap            mButtonMap; // for joystick

	bool                  mHasJoystick;
	bool                  mHasPointer;
	bool                  mHasKey;
	bool                  mRouton;
};

}

#endif
