#include "DriverFactory.h"

using namespace Sexy;

Driver::Driver (const std::string theName,
			  int		    thePriority)
    : mName (theName), mPriority (thePriority)
{
}

Driver::~Driver ()
{
}

DriverFactory::DriverFactory ()
    : mDrivers ()
{
}

DriverFactory::~DriverFactory ()
{
}

void DriverFactory::AddDriver (Driver * theDriver)
{
	mDrivers.insert (theDriver);
}

void DriverFactory::RemoveDriver (Driver * theDriver)
{
	Drivers::iterator anItr = mDrivers.find (theDriver);
	if (anItr != mDrivers.end ())
		mDrivers.erase (anItr);
}

Driver* DriverFactory::Find (const std::string name)
{
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
