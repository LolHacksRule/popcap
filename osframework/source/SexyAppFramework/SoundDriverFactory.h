#ifndef __SOUNDDriverFACTORY_H__
#define __SOUNDDriverFACTORY_H__

#include "Common.h"
#include "DriverFactory.h"
#include "SoundManager.h"
#include "MusicInterface.h"

namespace Sexy {
class SoundDriver: public Driver
{
 public:
	virtual SoundManager* Create (SexyAppBase * theApp) = 0;

	virtual MusicInterface* CreateMusicInterface (SexyAppBase * theApp);

 public:
	SoundDriver (const std::string theName,
		     int	       thePriority = 0);
	~SoundDriver ();
};


class SoundDriverFactory: public DriverFactory
{
 public:
	static SoundDriverFactory*  GetSoundDriverFactory ();

 private:
	void		     Load();

 private:
	friend class StaticSoundDriverFactory;

	SoundDriverFactory ();
	~SoundDriverFactory ();
};

class SoundDriverRegistor
{
 public:
	SoundDriverRegistor(SoundDriver * theDriver)
		: mDriver (theDriver)
	{
		SoundDriverFactory* factory;

		factory = SoundDriverFactory::GetSoundDriverFactory ();
		if (factory)
			factory->AddDriver (mDriver);
	}

	~SoundDriverRegistor()
	{
		SoundDriverFactory* factory;

		factory = SoundDriverFactory::GetSoundDriverFactory ();
		if (factory)
			factory->RemoveDriver (mDriver);
	}

 private:
	SoundDriver * mDriver;
};

}

#endif
