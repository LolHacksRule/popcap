#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "Common.h"
#include "CritSect.h"
#include "InputDriverFactory.h"

#include <list>

namespace Sexy
{

class SexyAppBase;
class InputManager
{
 public:
	InputManager(SexyAppBase * theApp,
		     unsigned theMaxEvents = 1024);
	~InputManager();

 public:
	void	      Init();
	void	      Cleanup(void);

 public:
	bool	      HasEvent();
	void	      PushEvent(Event &event);
	bool	      PopEvent(Event &event);
	void	      Update(void);
	void	      ConnectAll(void);
	void	      ReconnectAll(void);
	void	      Connect(int id);
	void	      Reconnect(int id);

	InputInterface* Find(int id);
	InputInterface* Find(const std::string& name);

 private:
	SexyAppBase * mApp;

	typedef std::list<InputInterface*> Drivers;
	typedef std::list<Event> EventQueue;

	Drivers	      mDrivers;
	EventQueue    mEventQueue;
	unsigned      mMaxEvents;

	CritSect      mCritSect;

	int	      mX;
	int	      mY;
	int	      mWidth;
	int	      mHeight;

	int	      mId;
};

}

#endif
