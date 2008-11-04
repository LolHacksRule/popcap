#ifndef __INPUTDriverFACTORY_H__
#define __INPUTDriverFACTORY_H__

#include "Common.h"
#include "DriverFactory.h"
#include "InputInterface.h"

namespace Sexy {
class InputDriver: public Driver
{
 public:
	virtual InputInterface* Create (SexyAppBase * theApp) = 0;


 public:
	InputDriver (const std::string theName,
		     int	       thePriority = 0);
	~InputDriver ();
};


class InputDriverFactory: public DriverFactory
{
 public:
	static InputDriverFactory*  GetInputDriverFactory ();

 private:
	void		     Load();

 private:
	InputDriverFactory ();
	~InputDriverFactory ();
};

class InputDriverRegistor
{
 public:
	InputDriverRegistor(InputDriver * theDriver)
		: mDriver (theDriver)
	{
		InputDriverFactory* factory;

		factory = InputDriverFactory::GetInputDriverFactory ();
		factory->AddDriver (mDriver);
	}

	~InputDriverRegistor()
	{
		InputDriverFactory* factory;

		factory = InputDriverFactory::GetInputDriverFactory ();
		factory->RemoveDriver (mDriver);
	}

 private:
	InputDriver * mDriver;
};

}

#endif
