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

MusicInterface* SoundDriver::CreateMusicInterface (SexyAppBase * theApp)
{
	return 0;
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
extern SoundDriver* GetGstSoundDriver();
extern SoundDriver* GetAudiereSoundDriver();
extern SoundDriver* GetDSoundDriver();
extern SoundDriver* GetOpenALSoundDriver();
typedef SoundDriver* (* SoundDriverGetter)();
SoundDriverGetter SoundDriverGetters []= {
#ifdef SEXY_GST_SOUND_DRIVER
	GetGstSoundDriver,
#endif
#ifdef SEXY_AUDIERE_SOUND_DRIVER
	GetAudiereSoundDriver,
#endif
#ifdef SEXY_DIRECT_SOUND_DRIVER
	GetDSoundDriver,
#endif
#ifdef SEXY_OPENAL_SOUND_DRIVER
	GetOpenALSoundDriver,
#endif
	GetDummySoundDriver,
	NULL
};

void SoundDriverFactory::Load(void)
{
	int i = 0;
	for (i = 0; SoundDriverGetters[i]; i++)
		SoundDriverGetters[i]();
}

