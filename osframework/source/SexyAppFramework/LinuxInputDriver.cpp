#include "LinuxInputDriver.h"
#include "InputDriverFactory.h"
#include "SexyAppBase.h"

using namespace Sexy;

LinuxInputInterface::LinuxInputInterface ()
	: InputInterface()
{
}

LinuxInputInterface::~LinuxInputInterface ()
{
}

bool LinuxInputInterface::HasEvent ()
{
	return false;
}

bool LinuxInputInterface::GetEvent (Event & event)
{
	return false;
}

class LinuxInputDriver: public InputDriver {
public:
	LinuxInputDriver ()
	 : InputDriver("LinuxInputInterface", 10)
	{
	}

	InputInterface* Create (SexyAppBase * theApp)
	{
		return new LinuxInputInterface ();
        }
};

static LinuxInputDriver aLinuxInputDriver;
InputDriverRegistor aLinuxInputDriverRegistor (&aLinuxInputDriver);
InputDriver* GetLinuxInputDriver()
{
	return &aLinuxInputDriver;
}
