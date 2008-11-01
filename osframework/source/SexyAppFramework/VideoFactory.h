#ifndef __VIDEOFACTORY_H__
#define __VIDEOFACTORY_H__

#include "Common.h"
#include "NativeDisplay.h"

namespace Sexy {
class VideoDriver
{
 public:
	virtual NativeDisplay* Create (SexyAppBase * theApp) = 0;

 public:
	std::string mName;
	int         mPriority;

	VideoDriver (const std::string theName,
		     int               thePriority = 0);
	~VideoDriver ();

	bool operator< (const VideoDriver& other) const
	{
		return mPriority < other.mPriority;
	}
};

struct VideoDriverCompare {
	bool operator() (VideoDriver* const & lhs, VideoDriver* const & rhs) const
	{
		return *lhs < *rhs;
	}
};



class VideoDriverFactory
{
 public:
	static VideoDriverFactory*  GetVideoDriverFactory ();

	void                 AddDriver (VideoDriver * theDriver);
	void                 RemoveDriver (VideoDriver * theDriver);

	VideoDriver*         Find (const std::string name = "auto");

 private:
	void                 Load();

 private:
	typedef std::set<VideoDriver*, VideoDriverCompare> VideoDrivers;
	VideoDrivers         mDrivers;

 private:
	VideoDriverFactory ();
	~VideoDriverFactory ();
};

class VideoDriverRegistor
{
 public:
	VideoDriverRegistor(VideoDriver * theDriver)
		: mDriver (theDriver)
	{
		VideoDriverFactory* factory;

		factory = VideoDriverFactory::GetVideoDriverFactory ();
		factory->AddDriver (mDriver);
	}

	~VideoDriverRegistor()
	{
		VideoDriverFactory* factory;

		factory = VideoDriverFactory::GetVideoDriverFactory ();
		factory->RemoveDriver (mDriver);
	}

 private:
	VideoDriver * mDriver;
};

}

#endif
