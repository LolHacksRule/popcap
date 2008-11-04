#include "AutoCrit.h"
#include "InputManager.h"
#include "SexyAppBase.h"
#include "InputDriverFactory.h"

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

	InputDriverFactory* factory;

	factory = InputDriverFactory::GetInputDriverFactory ();

	const DriverFactory::Drivers* Creators = factory->GetDrivers ();
	DriverFactory::Drivers::iterator it;
	for (it = Creators->begin (); it != Creators->end (); ++it)
	{
		InputInterface * aInput;
		aInput = dynamic_cast<InputInterface*>
			(((InputDriver*)(*it))->Create (mApp));
		if (aInput->Init ())
			mDrivers.push_back (aInput);
		else
			delete aInput;
	}
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

	if (event.type == EVENT_MOUSE_BUTTON_PRESS ||
	    event.type == EVENT_MOUSE_BUTTON_RELEASE ||
	    event.type == EVENT_MOUSE_MOTION)
	{
		if (event.x < 0)
			event.x = 0;
		if (event.y < 0)
			event.y = 0;
		if (event.x > mApp->mWidth)
			event.x = mApp->mWidth;
		if (event.y > mApp->mHeight)
			event.y = mApp->mHeight;
	}
	return true;
}

void InputManager::Update (void)
{
	Event event;
	while (mApp->mDDInterface && mApp->mDDInterface->GetEvent (event))
		PushEvent (event);

	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		(*it)->Update ();
}
