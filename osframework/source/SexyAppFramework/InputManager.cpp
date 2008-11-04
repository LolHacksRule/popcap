#include "AutoCrit.h"
#include "InputManager.h"
#include "SexyAppBase.h"
#include "InputDriverFactory.h"

using namespace Sexy;

InputManager::InputManager (SexyAppBase * theApp,
			    unsigned theMaxEvents)
	: mApp (theApp), mDrivers (), mEventQueue (),
	  mMaxEvents (theMaxEvents), mCritSect (),
	  mX (0), mY (0)
{
	mWidth = 1;
	mHeight = 1;
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

	mWidth = mApp->mWidth;
	mHeight = mApp->mHeight;
}

void InputManager::Cleanup (void)
{
	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		delete (*it);

	AutoCrit anAutoCrit (mCritSect);
	mDrivers.clear ();
	mEventQueue.clear ();

	mX = 0;
	mY = 0;
	mWidth = 1;
	mHeight = 1;
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
		int x, y;

		x = event.x;
		y = event.y;
		if (event.flags & EVENT_FLAGS_REL_AXIS)
		{
			mX += x;
			mY += y;
		}
		else if (event.flags & EVENT_FLAGS_AXIS)
		{
			mX = x;
			mY = y;
		}

		if (mX < 0)
			mX = 0;
		if (mY < 0)
			mY = 0;
		if (mX >= mWidth)
			mX = mWidth - 1;
		if (mY > mHeight)
			mY = mHeight - 1;
		event.x = mX;
		event.y = mY;
		event.flags |= EVENT_FLAGS_AXIS;
		event.flags &= ~EVENT_FLAGS_REL_AXIS;
	}

	if (0 && event.type != EVENT_NONE)
	{
		printf ("event.type: %d\n", event.type);
		if (event.flags & EVENT_FLAGS_AXIS)
		{
			printf ("event.x: %d\n", event.x);
			printf ("event.y: %d\n", event.y);
		}
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
