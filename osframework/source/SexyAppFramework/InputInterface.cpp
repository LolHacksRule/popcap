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
		mManager->PushEvent (event);
}

void InputInterface::PostEvent(Event &event)
{
	event.id = mId;
	mManager->PushEvent (event);
}
