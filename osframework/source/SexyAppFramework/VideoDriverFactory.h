#ifndef __VIDEODriverFACTORY_H__
#define __VIDEODriverFACTORY_H__

#include "Common.h"
#include "DriverFactory.h"
#include "NativeDisplay.h"

namespace Sexy {
class VideoDriver: public Driver
{
 public:
	virtual NativeDisplay* Create (SexyAppBase * theApp) = 0;


 public:
	VideoDriver (const std::string theName,
		     int               thePriority = 0);
	~VideoDriver ();
};


class VideoDriverFactory: public DriverFactory
{
 public:
	static VideoDriverFactory*  GetVideoDriverFactory ();

 private:
	void                 Load();

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
