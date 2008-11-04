#include "InputDriverFactory.h"

using namespace Sexy;

InputDriver::InputDriver (const std::string theName,
			  int		    thePriority)
	: Driver(theName, thePriority)
{
}

InputDriver::~InputDriver ()
{
}

InputDriverFactory::InputDriverFactory ()
	: DriverFactory ()
{
}

InputDriverFactory::~InputDriverFactory ()
{
}

InputDriverFactory*  InputDriverFactory::GetInputDriverFactory ()
{
	static InputDriverFactory  * theInputDriverFactory;

	if (!theInputDriverFactory)
		theInputDriverFactory = new InputDriverFactory ();
	return theInputDriverFactory;
}

/* This is a hack that preventing gcc from striping drivers out of
 * binary.
 */
extern InputDriver* GetLinuxInputDriver();
typedef InputDriver* (* InputDriverGetter)();
InputDriverGetter InputDriverGetters []= {
#ifdef SEXY_LINUX_DRIVER
	//GetLinuxInputDriver,
#endif
	NULL
};

void InputDriverFactory::Load(void)
{
	int i = 0;
	for (i = 0; InputDriverGetters[i]; i++)
		InputDriverGetters[i]();
}

