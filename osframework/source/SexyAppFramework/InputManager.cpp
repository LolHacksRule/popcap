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
	mId = 0;
	mCookie = 0;
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
	DriverFactory::Drivers::const_iterator it;
	for (it = Creators->begin (); it != Creators->end (); ++it)
	{
		InputInterface * aInput;

		aInput = dynamic_cast<InputInterface*>
			(((InputDriver*)(*it))->Create (mApp));

		if (aInput && !Add(aInput, (InputDriver*)(*it)))
			delete aInput;
	}

	mWidth = mApp->mDDInterface->mDisplayWidth;
	mHeight = mApp->mDDInterface->mDisplayHeight;

	ConnectAll();

	/* notify creators */
	for (it = Creators->begin (); it != Creators->end (); ++it)
	{
		InputDriver* driver =(InputDriver*)(*it);
		driver->OnStart (mApp, this);
	}

}

void InputManager::Cleanup (void)
{
	InputDriverFactory* factory;

	factory = InputDriverFactory::GetInputDriverFactory ();

	const DriverFactory::Drivers* Creators = factory->GetDrivers ();
	DriverFactory::Drivers::const_iterator dit;

	/* notify creators */
	for (dit = Creators->begin (); dit != Creators->end (); ++dit)
	{
		InputDriver* driver =(InputDriver*)(*dit);
		driver->OnStop ();
	}

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
	{
		if (!(event.flags & EVENT_FLAGS_TIMESTAMP))
		{
			event.timestamp = Sexy::GetTickCount();
			event.flags |= EVENT_FLAGS_TIMESTAMP;
		}
		mEventQueue.push_back (event);
	}
}

void InputManager::PushEvents (std::list<Event> &events)
{
	AutoCrit anAutoCrit (mCritSect);

	if (mEventQueue.size () + events.size() >= mMaxEvents)
		return;

	for (std::list<Event>::iterator it = events.begin();
	     it != events.end(); ++it)
	{
		Event &event = *it;

		if (event.type != EVENT_NONE)
		{
			if (!(event.flags & EVENT_FLAGS_TIMESTAMP))
			{
				event.timestamp = Sexy::GetTickCount();
				event.flags |= EVENT_FLAGS_TIMESTAMP;
			}
			mEventQueue.push_back (event);
		}
	}
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
		if (event.flags & (EVENT_FLAGS_REL_AXIS | EVENT_FLAGS_AXIS))
		{
			int x, y;

			x = event.u.mouse.x;
			y = event.u.mouse.y;
			if (event.flags & EVENT_FLAGS_REL_AXIS)
			{
				mX += x;
				mY += y;
			}
			else if (event.flags & EVENT_FLAGS_AXIS)
			{
				mX = x;
				mY = y;
				if (event.flags & EVENT_FLAGS_AXIS_RANGE)
				{
					mX = x * mWidth / event.u.mouse.maxx;
					mY = x * mHeight / event.u.mouse.maxy;
				}
			}
		}
		if (mX < 0)
			mX = 0;
		if (mY < 0)
			mY = 0;
		if (mX >= mWidth)
			mX = mWidth - 1;
		if (mY > mHeight)
			mY = mHeight - 1;
		event.u.mouse.x = mX;
		event.u.mouse.y = mY;
		event.flags |= EVENT_FLAGS_AXIS;
		event.flags &= ~EVENT_FLAGS_REL_AXIS;
	}

	if (false && event.type != EVENT_NONE)
	{
		printf ("event.type: %d\n", event.type);
		printf ("event.button: %d\n", event.u.mouse.button);
		if (event.flags & EVENT_FLAGS_AXIS)
		{
			printf ("event.x: %d\n", event.u.mouse.x);
			printf ("event.y: %d\n", event.u.mouse.y);
		}
	}
	return true;
}

void InputManager::Update (void)
{
	Event event;
	while (mApp->mDDInterface && mApp->mDDInterface->GetEvent (event))
	{
		// the id field of events from NativeDisplay is always 0
		event.id = 0;
		PushEvent (event);
	}

	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		(*it)->Update ();
}

void InputManager::ConnectAll (void)
{
	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		(*it)->Connect ();
}

void InputManager::ReconnectAll (void)
{
	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		(*it)->Reconnect ();
}

void InputManager::Connect (int id)
{
	InputInterface* anInput;

	anInput = Find(id);
	if (!anInput)
		return;
	anInput->Connect();
}

void InputManager::Reconnect (int id)
{
	InputInterface* anInput;

	anInput = Find(id);
	if (!anInput)
		return;
	anInput->Reconnect();
}

InputInterface* InputManager::Find(int id)
{
	Drivers::iterator it;

	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		if ((*it)->mId == id)
			return *it;

	return 0;
}

bool InputManager::Add(InputInterface * theInput,
		       InputDriver * theDriver,
		       bool connect)
{
	AutoCrit anAutoCrit (mCritSect);

	theInput->mId = mId + 1;
	theInput->mInputDriver = theDriver;

	if (theInput->Init ())
	{
		mId++;
		mDrivers.push_back (theInput);

		if (connect)
			theInput->Connect ();

		mCookie++;
		return true;
	}

	return false;
}

bool InputManager::Remove(InputInterface * theInput)
{
	if (!theInput)
		return false;

	{
		AutoCrit anAutoCrit (mCritSect);

		Drivers::iterator it = std::find (mDrivers.begin (), mDrivers.end (), theInput);
		if (it == mDrivers.end ())
			return false;

		mDrivers.erase (it);
	}

	delete theInput;
	mCookie++;
	return true;
}

unsigned int InputManager::GetCookie()
{
	return mCookie;
}

static void UpdateStatusInfo(InputStatusInfo &theInfo,
			     InputInfo &anInfo)
{
	if (anInfo.mHasPointer)
		theInfo.mNumPointer++;
	else if (anInfo.mHasKey)
		theInfo.mNumKeyboard++;
	else if (anInfo.mHasAcc || anInfo.mHasGyro)
		theInfo.mNum3DInput++;
}

void InputManager::GetStatus(InputStatusInfo &theInfo)
{
	AutoCrit anAutoCrit (mCritSect);

	theInfo.mNumDriver = mDrivers.size();
	theInfo.mNumInput = 0;
	theInfo.mNumPointer = 0;
	theInfo.mNumKeyboard = 0;
	theInfo.mNum3DInput = 0;

	InputInfo anInfo;
	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
	{
		int numDevices = (*it)->GetNumDevices();
		theInfo.mNumInput += numDevices;
		for (int i = 0; i < numDevices; i++)
		{
			if (!(*it)->GetInfo(anInfo))
				continue;

			UpdateStatusInfo(theInfo, anInfo);
			anInfo.Reset();
		}
	}
	if (mApp->mDDInterface && mApp->mDDInterface->GetInputInfo(anInfo))
		UpdateStatusInfo(theInfo, anInfo);
}

void InputManager::Changed()
{
	AutoCrit anAutoCrit (mCritSect);
	mCookie++;
}
