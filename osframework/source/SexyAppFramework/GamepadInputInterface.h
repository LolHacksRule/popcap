#ifndef __GAMEPAD_INPUT_INTERFACE_H__
#define __GAMEPAD_INPUT_INTERFACE_H__

#include "InputInterface.h"
#include "KeyCodes.h"

namespace Sexy {

	// This is not neccessary a real gamepad
	class GamepadInputInterface : public InputInterface {
	public:
		GamepadInputInterface(InputManager* theManager);
		~GamepadInputInterface();

		virtual void PostResetEvents();
		virtual void ResetStates();
		virtual void UpdateState(Event& event, int subid = 0);
		virtual void Update();

	protected:
		struct DevState;
		bool HandleAxisChanged(float lx, float x, float flat,
				       bool vertical, int subid,
				       const Event& ref);
		DevState& GetState(int subid);

	protected:
		typedef std::map<Axis, float>   AxisValueMap;
		typedef std::map<KeyCode, bool> ButtonStateMap;
		struct DevState {
			AxisValueMap   mAxes;
			ButtonStateMap mButtons;

			float mX, mY;
			float mHatX, mHatY;
		};
		typedef std::map<int, DevState> DevStateMap;
		DevStateMap mDevStateMap;
	};
};

#endif
