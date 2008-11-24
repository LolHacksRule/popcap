#include "Common.h"
#include "InputInterface.h"
#include "InputManager.h"

using namespace Sexy;


InputInterface::InputInterface(InputManager* theManager)
	: mManager (theManager)
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

void InputInterface::Update()
{
	Event event;

	while (GetEvent (event))
		mManager->PushEvent (event);
}

void InputInterface::PostEvent(Event &event)
{
	mManager->PushEvent (event);
}
