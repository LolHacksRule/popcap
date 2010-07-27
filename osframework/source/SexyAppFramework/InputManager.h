#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "Common.h"
#include "CritSect.h"
#include "InputDriverFactory.h"

#include <list>

namespace Sexy
{

class SexyAppBase;

struct InputStatusInfo {
	int mNumPointer;
	int mNumKeyboard;
	int mNum3DInput;
	int mNumInput;
	int mNumDriver;
};

class InputManager
{
 public:
        InputManager(SexyAppBase * theApp,
                     unsigned theMaxEvents = 1024);
        ~InputManager();

 public:
        void          Init();
        void          Cleanup(void);

 public:
        bool          HasEvent();
        void          PushEvent(Event &event);
	void          PushEvents(std::list<Event> &events);
        bool          PopEvent(Event &event);
        void          Update(void);
        void          ConnectAll(void);
        void          ReconnectAll(void);
        void          Connect(int id);
        void          Reconnect(int id);

	InputInterface* Find(int id);

	bool          Add(InputInterface * theInput,
			  InputDriver * theDriver,
			  bool connect = false);

	bool          Remove(InputInterface * theInput);

	unsigned int  GetCookie();
	void          GetStatus(InputStatusInfo &theInfo);
	void          Changed();

 private:
        SexyAppBase * mApp;

        typedef std::list<InputInterface*> Drivers;
	typedef std::list<Event> EventQueue;

        Drivers       mDrivers;
        EventQueue    mEventQueue;
        unsigned      mMaxEvents;

	CritSect      mCritSect;

	int           mX;
	int           mY;
	int           mWidth;
	int           mHeight;

	int           mId;
	unsigned int  mCookie;
};

}

#endif
