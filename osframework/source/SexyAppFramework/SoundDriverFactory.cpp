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

namespace Sexy {

class StaticSoundDriverFactory
{
public:
	struct StaticData {
		SoundDriverFactory* mFactory;
		bool mDone;
	};

	StaticSoundDriverFactory(StaticData* data)
	{
		mData = data;
	}

	SoundDriverFactory* Get(StaticData* data)
	{
		if (data->mDone)
			return 0;

		if (data->mFactory)
			return data->mFactory;

		data->mFactory = new SoundDriverFactory;
		return data->mFactory;
	}

	~StaticSoundDriverFactory()
	{
		if (!mData)
			return;

		mData->mDone = true;
		if (mData->mFactory)
			delete mData->mFactory;
	}

private:
	StaticData* mData;
};

static StaticSoundDriverFactory::StaticData aData;
static StaticSoundDriverFactory soundDriverFactory(&aData);

}

SoundDriverFactory* SoundDriverFactory::GetSoundDriverFactory ()
{
	return soundDriverFactory.Get(&aData);
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

