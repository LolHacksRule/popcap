#ifndef __INPUTDriverFACTORY_H__
#define __INPUTDriverFACTORY_H__

#include "Common.h"
#include "DriverFactory.h"
#include "InputInterface.h"

namespace Sexy {
class InputManager;

class InputDriver: public Driver
{
 public:
	virtual InputInterface* Create (SexyAppBase * theApp) = 0;
	virtual void            OnStart (SexyAppBase * theApp,
					 InputManager * theManager) {};
	virtual void            OnStop () {};

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
	void		            Load();

 private:
	friend class StaticInputDriverFactory;

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
		if (factory)
			factory->AddDriver (mDriver);
	}

	~InputDriverRegistor()
	{
		InputDriverFactory* factory;

		factory = InputDriverFactory::GetInputDriverFactory ();
		if (factory)
			factory->RemoveDriver (mDriver);
	}

 private:
	InputDriver * mDriver;
};

}

#endif
