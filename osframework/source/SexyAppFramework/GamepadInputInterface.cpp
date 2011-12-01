#include "GamepadInputInterface.h"

#include <math.h>

using namespace Sexy;

GamepadInputInterface::GamepadInputInterface(InputManager* theManager)
	: InputInterface(theManager)
{
}

GamepadInputInterface::~GamepadInputInterface()
{
}

GamepadInputInterface::DevState& GamepadInputInterface::GetState(int subid)
{
	DevStateMap::iterator it = mDevStateMap.find(subid);
	if (it != mDevStateMap.end())
		return it->second;

	DevState state;

	state.mX = 0;
	state.mY = 0;
	state.mHatX = 0;
	state.mHatY = 0;
	mDevStateMap[subid] = state;
	return mDevStateMap[subid];
}

void GamepadInputInterface::PostResetEvents()
{
	DevStateMap::iterator it = mDevStateMap.begin();

	Event kevt;
	kevt.type = EVENT_KEY_UP;
	kevt.id = mId;
	kevt.flags |= EVENT_FLAGS_KEY_CODE;

	for (; it != mDevStateMap.end(); ++it)
	{
		DevState& state = it->second;

		ButtonStateMap::iterator bit = state.mButtons.begin();
		for (; bit != state.mButtons.end(); ++bit)
		{
			if (bit->second)
			{
				kevt.u.key.keyCode = bit->first;
				PostEvent(kevt, it->first);
			}
		}
	}
}

void GamepadInputInterface::ResetStates()
{
	PostResetEvents();
	mDevStateMap.clear();
}

bool GamepadInputInterface::HandleAxisChanged(float lx, float x, float flat,
				     bool vertical, int subid,
				     const Event& ref)
{
	int ix = 0, ilx = 0;

	if (fabs(x) <= flat)
		ix = 0;
	else if (x > flat)
		ix = 1;
	else
		ix = -1;

	if (fabs(lx) <= flat)
		ilx = 0;
	else if (lx > flat)
		ilx = 1;
	else
		ilx = -1;

	if (ix == ilx)
		return false;

	Event evt = ref;
	evt.flags |= EVENT_FLAGS_KEY_CODE;

	KeyCode keyCodes[2];
	if (!vertical)
	{
		keyCodes[0] = KEYCODE_GAMEPAD_LEFT;
		keyCodes[1] = KEYCODE_GAMEPAD_RIGHT;
	}
	else
	{
		keyCodes[0] = KEYCODE_GAMEPAD_UP;
		keyCodes[1] = KEYCODE_GAMEPAD_DOWN;
	}

	KeyCode keyCode = KEYCODE_UNKNOWN;
	if (ilx + ix == 1)
	{
		keyCode = keyCodes[1];
		evt.u.key.keyCode = keyCode;
		if (ix == 1)
		{
			evt.type = EVENT_KEY_DOWN;
			UpdateState(evt, subid);
			PostEvent(evt, subid);
		}
		else
		{
			evt.type = EVENT_KEY_UP;
			UpdateState(evt, subid);
			PostEvent(evt, subid);
		}
	}
	else if (ilx + ix == -1)
	{
		keyCode = keyCodes[0];
		evt.u.key.keyCode = keyCode;
		if (ix == -1)
		{
			evt.type = EVENT_KEY_DOWN;
			UpdateState(evt, subid);
			PostEvent(evt, subid);
		}
		else
		{
			evt.type = EVENT_KEY_UP;
			UpdateState(evt, subid);
			PostEvent(evt, subid);
		}
	}
	else if (ix == 1 && ilx == -1)
	{
		keyCode = keyCodes[1];
		evt.u.key.keyCode = keyCode;
		evt.type = EVENT_KEY_UP;
		UpdateState(evt, subid);
		PostEvent(evt, subid);

		keyCode = keyCodes[0];
		evt.u.key.keyCode = keyCode;
		evt.type = EVENT_KEY_DOWN;
		UpdateState(evt, subid);
		PostEvent(evt, subid);
	}
	else if (ix == -1 && ilx == 1)
	{
		keyCode = keyCodes[0];
		evt.u.key.keyCode = keyCode;
		evt.type = EVENT_KEY_UP;
		UpdateState(evt, subid);
		PostEvent(evt, subid);

		keyCode = keyCodes[1];
		evt.u.key.keyCode = keyCode;
		evt.type = EVENT_KEY_DOWN;
		UpdateState(evt, subid);
		PostEvent(evt, subid);
	}

	return true;
}

void GamepadInputInterface::UpdateState(Event& event, int subid)
{
	if (event.type != EVENT_KEY_DOWN && event.type != EVENT_KEY_UP &&
	    event.type != EVENT_AXIS_MOVED)
		return;

	DevState& state = GetState(subid);
	if (event.type == EVENT_KEY_DOWN)
	{
		state.mButtons[KeyCode(event.u.key.keyCode)] = true;
		return;
	}
	else if (event.type == EVENT_KEY_UP)
	{
		state.mButtons[KeyCode(event.u.key.keyCode)] = false;
		return;
	}

	Axis axis = Axis(event.u.axis.axis);
	AxisValueMap::iterator it = state.mAxes.find(axis);

	if (it == state.mAxes.end())
	{
		state.mAxes[axis] = 0.0f;
		it = state.mAxes.find(axis);
	}

	float curValue = event.u.axis.value;
	state.mAxes[axis] = curValue;

	if (axis != AXIS_X && axis != AXIS_Y &&
	    axis != AXIS_HAT0X && axis != AXIS_HAT0Y)
		return;

	float flat = event.u.axis.flat;
	float x = axis == AXIS_X ? state.mX : state.mHatX;
	float y = axis == AXIS_Y ? state.mY : state.mHatY;
	float lx = x;
	float ly = y;

	if (axis == AXIS_X)
	{
		x = event.u.axis.value;
		if (HandleAxisChanged(lx, x, flat, false, subid, event))
			state.mX = x;
	}
	if (axis == AXIS_HAT0X)
	{
		x = event.u.axis.value;
		if (HandleAxisChanged(lx, x, flat, false, subid, event))
			state.mHatX = x;
	}
	else if (axis == AXIS_Y)
	{
		y = event.u.axis.value;
		if (HandleAxisChanged(ly, y, flat, true, subid, event))
			state.mY = y;
	}
	else if (axis == AXIS_HAT0Y)
	{
		y = event.u.axis.value;
		if (HandleAxisChanged(ly, y, flat, true, subid, event))
			state.mHatY = y;
	}
}

void GamepadInputInterface::Update()
{
	InputInterface::Update();

	// todo: handle repeat here.
}
