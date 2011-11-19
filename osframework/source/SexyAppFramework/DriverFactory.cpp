#include "DriverFactory.h"

using namespace Sexy;

Driver::Driver (const std::string theName,
		int		    thePriority)
    : mName (theName), mPriority (thePriority),
      mDisabled (false)
{
}

Driver::~Driver ()
{
}

void Driver::Disable (bool disable)
{
	mDisabled = disable;
}

bool Driver::IsDisabled ()
{
	return mDisabled;
}

DriverFactory::DriverFactory ()
	: mDrivers (), mValid(true)
{
}

DriverFactory::~DriverFactory ()
{
	mValid = false;
}

void DriverFactory::AddDriver (Driver * theDriver)
{
	if (!mValid)
		return;

	mDrivers.insert (theDriver);
}

void DriverFactory::RemoveDriver (Driver * theDriver)
{
	if (!mValid)
		return;

	mDrivers.erase (theDriver);
}

Driver* DriverFactory::Find (const std::string name)
{
	if (!mValid)
		return 0;

	if (name == "auto") {
		if (!mDrivers.size ())
			return 0;

		return *mDrivers.rbegin();
	}

	Drivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		if ((*it)->mName == name)
			return *it;
	return 0;

}

Driver* DriverFactory::FindNext (Driver * theDriver)
{
       if (!theDriver)
               return Find();

       Drivers::iterator it = mDrivers.find(theDriver);
       if (it == mDrivers.end () || ++it == mDrivers.end ())
               return 0;
       return *it;
}

const DriverFactory::Drivers* DriverFactory::GetDrivers()
{
	return &mDrivers;
}
