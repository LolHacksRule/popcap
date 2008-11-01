#include "SoundDriverFactory.h"

using namespace Sexy;

SoundDriver::SoundDriver (const std::string theName,
			  int		    thePriority)
	: Driver(theName, thePriority)
{
}

SoundDriver::~SoundDriver ()
{
}

SoundDriverFactory::SoundDriverFactory ()
	: DriverFactory ()
{
}

SoundDriverFactory::~SoundDriverFactory ()
{
}

SoundDriverFactory*  SoundDriverFactory::GetSoundDriverFactory ()
{
	static SoundDriverFactory  * theSoundDriverFactory;

	if (!theSoundDriverFactory)
		theSoundDriverFactory = new SoundDriverFactory ();
	return theSoundDriverFactory;
}

/* This is a hack that preventing gcc from striping drivers out of
 * binary.
 */
extern SoundDriver* GetDummySoundDriver();
typedef SoundDriver* (* SoundDriverGetter)();
SoundDriverGetter SoundDriverGetters []= {
	GetDummySoundDriver,
	NULL
};

void SoundDriverFactory::Load(void)
{
	int i = 0;
	for (i = 0; SoundDriverGetters[i]; i++)
		SoundDriverGetters[i]();
}

