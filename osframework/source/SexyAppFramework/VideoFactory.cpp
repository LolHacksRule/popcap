#include "VideoFactory.h"

using namespace Sexy;

VideoDriver::VideoDriver (const std::string theName,
			  int               thePriority)
    : mName (theName), mPriority (thePriority)
{
}

VideoDriver::~VideoDriver ()
{
}

VideoDriverFactory::VideoDriverFactory ()
    : mDrivers ()
{
}

VideoDriverFactory::~VideoDriverFactory ()
{
}

VideoDriverFactory*  VideoDriverFactory::GetVideoDriverFactory ()
{
	static VideoDriverFactory  * theVideoDriverFactory;

	if (!theVideoDriverFactory)
		theVideoDriverFactory = new VideoDriverFactory ();
	return theVideoDriverFactory;
}

void VideoDriverFactory::AddDriver (VideoDriver * theDriver)
{
	mDrivers.insert (theDriver);
}

void VideoDriverFactory::RemoveDriver (VideoDriver * theDriver)
{
	VideoDrivers::iterator anItr = mDrivers.find (theDriver);
	if (anItr != mDrivers.end ())
		mDrivers.erase (anItr);
}

VideoDriver* VideoDriverFactory::Find (const std::string name)
{
	if (name == "auto") {
		VideoDrivers::reverse_iterator rit;

		if (!mDrivers.size ())
			return 0;
		return *mDrivers.rbegin();
	}

	VideoDrivers::iterator it;
	for (it = mDrivers.begin (); it != mDrivers.end (); ++it)
		if ((*it)->mName == name)
			return *it;
	return 0;

}
