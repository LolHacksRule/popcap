#include "Common.h"
#include "InputInterface.h"
#include "InputManager.h"

using namespace Sexy;


InputInterface::InputInterface(InputManager* theManager)
    : mManager (theManager), mId(-1), mInputDriver(0)
{
}

InputInterface::~InputInterface()
{
}

bool InputInterface::Init()
{
	return true;
}

void InputInterface::Cleanup()
{
}

void InputInterface::Connect()
{
}

void InputInterface::Reconnect()
{
}

void InputInterface::Update()
{
	Event event;

	while (GetEvent (event))
	{
		event.id = mId;
		mManager->PushEvent (event);
	}
}

bool InputInterface::HasEvent ()
{
	return false;
}

bool InputInterface::GetEvent (Event &event)
{
	return false;
}

void InputInterface::PostEvent(Event &event, int subid)
{
	event.id = mId;
	event.id = subid;
	mManager->PushEvent (event);
}

bool InputInterface::GetProperty(const std::string& name,
				 void* retval)
{
	if (name == "id")
	{
		int* ret = (int*)retval;
		*ret = mId;
		return true;
	}
	return false;
}

bool InputInterface::GetInfo(InputInfo &theInfo)
{
	theInfo.mName = "";
	theInfo.mHasPointer = true;
	theInfo.mHasKey = false;
	theInfo.mHasAcc = false;
	theInfo.mHasGyro = false;
	theInfo.mId = mId;

	return true;
}
