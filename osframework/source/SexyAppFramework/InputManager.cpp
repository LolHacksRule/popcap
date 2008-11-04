#include "AutoCrit.h"
#include "InputManager.h"
#include "SexyAppBase.h"

using namespace Sexy;

InputManager::InputManager (SexyAppBase * theApp,
			    unsigned theMaxEvents)
	: mApp (theApp), mDrivers (), mEventQueue (),
	  mMaxEvents (theMaxEvents), mCritSect ()
{
}

InputManager::~InputManager ()
{
	Cleanup ();
}

void InputManager::Init ()
{
	Cleanup ();

	AutoCrit anAutoCrit (mCritSect);
}

void InputManager::Cleanup (void)
{
	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		delete (*it);

	AutoCrit anAutoCrit (mCritSect);
	mDrivers.clear ();
	mEventQueue.clear ();
}

bool InputManager::HasEvent ()
{
	AutoCrit anAutoCrit (mCritSect);

	if (mEventQueue.size () > 0)
		return true;
	return false;
}

void InputManager::PushEvent (Event &event)
{
	AutoCrit anAutoCrit (mCritSect);

	if (event.type != EVENT_NONE &&
	    mEventQueue.size () <= mMaxEvents)
		mEventQueue.push_back (event);
}

bool InputManager::PopEvent (Event &event)
{
	AutoCrit anAutoCrit (mCritSect);

	event.type = EVENT_NONE;
	if (mEventQueue.size () == 0)
		return false;

	event = mEventQueue.front ();
	mEventQueue.pop_front ();
	return true;
}
